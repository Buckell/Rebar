//
// Created by maxng on 8/21/2022.
//

#ifndef REBAR_FUNCTION_COMPILE_HPP
#define REBAR_FUNCTION_COMPILE_HPP

#include "../environment.hpp"

#include "../compiler.hpp"

namespace rebar {
    function compiler::compile(parse_unit& a_unit) {
        return compile_function(node::argument_list(), a_unit.m_block);
    }

    function compiler::compile_function(const node::argument_list& a_parameters, const node::block& a_block) {
        std::unique_ptr<compiler_function_source> function_source{ std::make_unique<compiler_function_source>(m_environment, m_runtime, m_logger) };

        asmjit::x86::Compiler cc(&function_source->code);
        cc.addDiagnosticOptions(asmjit::DiagnosticOptions::kRADebugAll);

        function_context ctx{ cc, *function_source };

        auto* func_node = cc.addFunc(asmjit::FuncSignatureT<void, object*, environment*>(platform_call_convention));

        ctx.return_object = cc.newGpq("ret");
        ctx.environment = cc.newGpq("env");

        ctx.identifier = cc.newGpq("id");

        ctx.lhs_type = cc.newGpq("ltype");
        ctx.lhs_data = cc.newGpq("ldata");
        ctx.rhs_type = cc.newGpq("rtype");
        ctx.rhs_data = cc.newGpq("rdata");

        func_node->setArg(0, ctx.return_object);
        func_node->setArg(1, ctx.environment);

        perform_preliminary_function_scan(ctx, a_block);

        const size_t locals_allocation_size = ctx.max_locals_count * sizeof(object);
        const size_t temps_allocation_size = default_temporary_stack_allocation * sizeof(object);

        ctx.locals_stack = cc.newStack(locals_allocation_size, alignof(object));
        //ctx.locals_stack = ctx.stack;

        //ctx.temporary_store_stack = ctx.stack;
        ctx.temporary_store_stack = cc.newStack(temps_allocation_size, alignof(object));

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
