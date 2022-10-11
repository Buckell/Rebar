//
// Created by maxng on 8/21/2022.
//

#ifndef REBAR_FUNCTION_COMPILE_HPP
#define REBAR_FUNCTION_COMPILE_HPP

#include "../environment.hpp"

#include "../compiler.hpp"

namespace rebar {
    class asmjit_error_handler : public asmjit::ErrorHandler {
        compiler& m_compiler;

    public:
        asmjit_error_handler(compiler& a_compiler) : asmjit::ErrorHandler(), m_compiler(a_compiler) {}

        void handleError(asmjit::Error err, const char* message, asmjit::BaseEmitter* origin) override {
            if (m_compiler.is_assembly_debug_output_enabled()) {
                std::cout << "AsmJit Error (" << static_cast<size_t>(err) << "): " << message << std::endl;
            }
        }
    };

    function compiler::compile(parse_unit& a_unit, const origin& a_origin) {
        return compile_function(a_unit, node::parameter_list(), a_unit.m_block, a_origin);
    }

    function compiler::compile_function(parse_unit& a_unit, const node::parameter_list& a_parameters, const node::block& a_block, const origin& a_origin) {
        std::unique_ptr<compiler_function_source> function_source{ std::make_unique<compiler_function_source>(m_environment, a_unit, a_parameters, m_runtime, m_logger) };

        asmjit::x86::Compiler cc(&function_source->code);
        cc.addDiagnosticOptions(asmjit::DiagnosticOptions::kRADebugAll);

        asmjit_error_handler error_handler(*this);

        if constexpr (debug_mode) {
            function_source->code.setErrorHandler(&error_handler);
        }

        function_context ctx(cc, *function_source, a_origin);

        auto* func_node = cc.addFunc(asmjit::FuncSignatureT<void, object*, environment*>(platform_call_convention));

        ctx.return_object = cc.newGpq("ret");
        ctx.environment = cc.newGpq("env");

        ctx.identifier = cc.newGpq("id");

        ctx.lhs_type = cc.newGpq("ltype");
        ctx.lhs_data = cc.newGpq("ldata");
        ctx.rhs_type = cc.newGpq("rtype");
        ctx.rhs_data = cc.newGpq("rdata");

        ctx.transfer = cc.newXmm("transfer");

        func_node->setArg(0, ctx.return_object);
        func_node->setArg(1, ctx.environment);

        perform_preliminary_function_scan(ctx, a_block);

        const size_t locals_allocation_size = ctx.max_locals_count * sizeof(object);
        const size_t temps_allocation_size = default_temporary_stack_allocation * sizeof(object);

        ctx.locals_stack = cc.newStack(locals_allocation_size, alignof(object));
        //ctx.locals_stack = ctx.stack;

        //ctx.temporary_store_stack = ctx.stack;
        ctx.temporary_store_stack = cc.newStack(temps_allocation_size, alignof(object));

        ctx.function_call_information = cc.newStack(sizeof(compiler_implementation::stack_entry_information), alignof(compiler_implementation::stack_entry_information));

        if (a_parameters.size() > 0) {
            ctx.function_argument_stack = cc.newStack(a_parameters.size() * sizeof(object), alignof(object));

            const auto& label_short_copy = cc.newLabel();
            const auto& label_short_copy_loop = cc.newLabel();
            const auto& label_short_copy_zero_loop = cc.newLabel();
            const auto& label_short_copy_zero = cc.newLabel();
            const auto& label_full_copy_loop = cc.newLabel();
            const auto& label_end = cc.newLabel();

            // Load argument pointer into identifier.
            cc.mov(ctx.identifier, asmjit::x86::qword_ptr(reinterpret_cast<size_t>(m_environment.get_arguments_pointer_ref())));
            cc.lea(ctx.lhs_data, ctx.function_argument_stack);

            // Load argument count into lhs_type.
            cc.mov(ctx.lhs_type, asmjit::x86::qword_ptr(reinterpret_cast<size_t>(m_environment.get_arguments_size_pointer())));

            // Check if the number of passed arguments is lower than the number of parameters.
            cc.cmp(ctx.lhs_type, a_parameters.size());
            cc.jl(label_short_copy);

            // Full Copy - Only copy number of parameters.
            cc.xor_(ctx.lhs_type, ctx.lhs_type);  // Counter

            cc.bind(label_full_copy_loop);

            // TODO: Investigate shift not working with dqword_ptr.
            // asmjit::x86::dqword_ptr(ctx.identifier, ctx.lhs_type, 4)

            cc.movdqa(ctx.transfer, asmjit::x86::dqword_ptr(ctx.identifier));
            cc.movdqa(asmjit::x86::dqword_ptr(ctx.lhs_data), ctx.transfer);

            cc.cmp(ctx.lhs_type, a_parameters.size() - 1);
            cc.je(label_end);

            cc.inc(ctx.lhs_type);
            cc.add(ctx.lhs_data, 16);
            cc.add(ctx.identifier, 16);

            cc.jmp(label_full_copy_loop);

            // End Full Copy

            cc.bind(label_short_copy);

            // Short Copy - Copy number of passed arguments and zero (null) remaining.

            cc.xor_(ctx.rhs_type, ctx.rhs_type);  // Counter

            cc.cmp(ctx.lhs_type, 0);
            cc.je(label_short_copy_zero);

            cc.bind(label_short_copy_loop);

            cc.movdqa(ctx.transfer, asmjit::x86::dqword_ptr(ctx.identifier));
            cc.movdqa(asmjit::x86::dqword_ptr(ctx.lhs_data), ctx.transfer);

            cc.inc(ctx.rhs_type);
            cc.add(ctx.lhs_data, 16);
            cc.add(ctx.identifier, 16);

            cc.cmp(ctx.rhs_type, ctx.lhs_type);
            cc.jne(label_short_copy_loop);

            cc.bind(label_short_copy_zero);

            cc.pxor(ctx.transfer, ctx.transfer);

            cc.bind(label_short_copy_zero_loop);

            cc.movdqa(asmjit::x86::dqword_ptr(ctx.lhs_data), ctx.transfer);

            cc.cmp(ctx.rhs_type, a_parameters.size() - 1);
            cc.je(label_end);

            cc.inc(ctx.rhs_type);
            cc.add(ctx.lhs_data, 16);
            cc.add(ctx.identifier, 16);

            cc.jmp(label_short_copy_zero_loop);

            cc.bind(label_end);

            for (size_t i = 0; i < a_parameters.size(); ++i) {
                const auto& parameter = a_parameters[i];

                if (parameter.default_value) {
                    auto parameter_space(ctx.function_argument_stack);
                    parameter_space.addOffset(i * sizeof(object));

                    const auto& label_default_parameter_end = cc.newLabel();

                    cc.mov(ctx.lhs_type, parameter_space);

                    cc.cmp(ctx.lhs_type, type::null);
                    cc.jne(label_default_parameter_end);

                    perform_node_pass(ctx, *parameter.default_value, output_side::lefthand);

                    cc.mov(parameter_space, ctx.lhs_type);

                    parameter_space.addOffset(object_data_offset);
                    cc.mov(parameter_space, ctx.lhs_data);

                    cc.bind(label_default_parameter_end);
                }
            }
        }

        perform_block_pass(ctx, a_block);

        cc.mov(asmjit::x86::qword_ptr(ctx.return_object), type::null);
        cc.mov(asmjit::x86::qword_ptr(ctx.return_object, object_data_offset), 0);

        cc.ret();

        cc.endFunc();
        cc.finalize();

        callable func;
        asmjit::Error err = m_runtime.add(&func, &function_source->code);

        m_functions.emplace(func, std::move(function_source));

        return { m_environment, reinterpret_cast<void*>(func) };
    }
}

#endif //REBAR_FUNCTION_COMPILE_HPP
