//
// Created by maxng on 8/17/2022.
//

#ifndef REBAR_COMPILER_HPP
#define REBAR_COMPILER_HPP

#include <set>

#include "asmjit/asmjit.h"

#include "provider.hpp"
#include "object.hpp"
#include "table.hpp"

#include "compiler/ext.hpp"

namespace rebar {
    class environment;

    struct compiler_function_source {
        environment& env;
        parse_unit& puint;
        asmjit::CodeHolder code;
        std::vector<string> string_dependencies;
        node::parameter_list parameters;

        explicit compiler_function_source(environment& a_env, parse_unit& a_puint, const node::parameter_list& a_parameters, asmjit::JitRuntime& a_rt, asmjit::Logger& a_logger) : env(a_env), puint(a_puint), parameters(a_parameters) {
            code.init(a_rt.environment());
            code.setLogger(&a_logger);
        }

        string emplace_string_dependency(const std::string_view a_string);
    };

    class compiler : public provider {
        environment& m_environment;

        std::unordered_map<callable, std::unique_ptr<compiler_function_source>> m_functions;

        asmjit::JitRuntime m_runtime;
        asmjit::FileLogger m_logger;

    public:
        constexpr static asmjit::CallConvId platform_call_convention = platform::current & platform::windows ? asmjit::CallConvId::kX64Windows : asmjit::CallConvId::kHost;

        // TODO: Modify preliminary scan to determine needed allocation size;
        constexpr static size_t default_temporary_stack_allocation = 6;

        explicit compiler(environment& a_environment) noexcept : m_environment(a_environment), m_logger(stdout) {}

        [[nodiscard]] function compile(parse_unit& a_unit) override;

        [[nodiscard]] function bind(callable a_function) override {
            return { m_environment, reinterpret_cast<void*>(a_function) };
        }

        [[nodiscard]] object call(const void* a_data) override {
            auto function = reinterpret_cast<callable>(const_cast<void*>(a_data));

            object return_value;
            function(&return_value, &m_environment);

            return return_value;
        }

        void enable_assembly_debug_output(bool a_enable) {
            m_logger.setFile(a_enable ? stdout : nullptr);
        }

        enum class output_side {
            lefthand,
            righthand
        };

        struct output_registers {
            asmjit::x86::Gp& type;
            asmjit::x86::Gp& data;
        };

        struct function_context {
            asmjit::x86::Compiler& assembler;
            compiler_function_source& source;
            std::vector<std::string_view> parameter_list;

            // Local variable stack offsets.
            // Local definitions state. (Whether local is defined or not.)
            std::vector<std::set<std::pair<std::string_view, size_t>>> local_variable_list;
            asmjit::x86::Mem locals_stack;
            size_t local_stack_position = 0;
            std::vector<size_t> block_local_offsets;
            size_t max_locals_count = 0;

            asmjit::x86::Gp return_object;
            asmjit::x86::Gp environment;

            asmjit::x86::Gp identifier;

            asmjit::x86::Gp lhs_type;
            asmjit::x86::Gp lhs_data;

            asmjit::x86::Gp rhs_type;
            asmjit::x86::Gp rhs_data;

            asmjit::x86::Mem temporary_store_stack;

            asmjit::x86::Mem function_argument_stack;

            std::vector<asmjit::x86::Mem> argument_stack;
            size_t current_argument_ref = 0;

            output_registers expression_registers(output_side a_side) {
                return a_side == output_side::righthand ? output_registers{ rhs_type, rhs_data } : output_registers{ lhs_type, lhs_data };
            }

            void push_side(output_side a_side) {
                auto [out_type, out_data] = expression_registers(a_side);
                auto& cc = assembler;

                cc.mov(temporary_store_stack, out_type);
                temporary_store_stack.addOffset(object_data_offset);
                cc.mov(temporary_store_stack, out_data);
                temporary_store_stack.addOffset(object_data_offset);
            }

            void pop_side(output_side a_side) {
                auto [out_type, out_data] = expression_registers(a_side);
                auto& cc = assembler;

                temporary_store_stack.addOffset(-static_cast<int64_t>(object_data_offset));
                cc.mov(out_data, temporary_store_stack);
                temporary_store_stack.addOffset(-static_cast<int64_t>(object_data_offset));
                cc.mov(out_type, temporary_store_stack);
            }

            void push_identifier() {
                auto& cc = assembler;

                cc.mov(temporary_store_stack, identifier);
                temporary_store_stack.addOffset(sizeof(void*));
            }

            void pop_identifier() {
                auto& cc = assembler;

                temporary_store_stack.addOffset(-sizeof(void*));
                cc.mov(identifier, temporary_store_stack);
            }

            void emplace_raw_side_output(output_side a_side, size_t a_annotation = 0) {
                auto [out_type, out_data] = expression_registers(a_side);
                auto& cc = assembler;

                cc.commentf("EMPLACE_RAW_SIDE_OUTPUT (%d)", a_annotation);

                static auto t = cc.newGpq();

                cc.mov(t, a_annotation);

                asmjit::InvokeNode* invoke_node;
                cc.invoke(&invoke_node, _ext_output_object_data, asmjit::FuncSignatureT<void, size_t, size_t, size_t>(platform_call_convention));
                invoke_node->setArg(0, out_type);
                invoke_node->setArg(1, out_data);
                invoke_node->setArg(2, t);
            }

            [[nodiscard]] asmjit::x86::Mem mark_argument_allocation() noexcept {
                if (argument_stack.size() == current_argument_ref) {
                    argument_stack.push_back(assembler.newStack(default_argument_allocation * sizeof(object), alignof(object)));
                }

                return argument_stack[current_argument_ref++];
            }

            void unmark_argument_allocation() noexcept {
                --current_argument_ref;
            }
        };

        struct pass_flag {
            constexpr static size_t none                 = 0x0;
            constexpr static size_t identifier_as_string = 0x1;
            constexpr static size_t local_identifier     = 0x1 << 1;
        };

        using pass_flags = size_t;

    private:
        function compile_function(parse_unit& a_unit, const node::parameter_list& a_parameters, const node::block& a_block);

        void load_token(function_context& a_ctx, const token& a_token, output_side a_side, pass_flags a_flags = pass_flag::none);
        void load_identifier(function_context& a_ctx, std::string_view a_identifier, output_side a_side, pass_flags a_flags = pass_flag::none);
        void load_identifier_pointer(function_context& a_ctx, std::string_view a_identifier, pass_flags a_flags = pass_flag::none);

        void perform_preliminary_function_scan(function_context& a_ctx, const node::block& a_block);

        void perform_block_pass(function_context& a_ctx, const node::block& a_block, pass_flags a_flags = pass_flag::none);
        void perform_node_pass(function_context& a_ctx, const node& a_node, output_side a_side = output_side::lefthand, pass_flags a_flags = pass_flag::none);
        void perform_expression_pass(function_context& a_ctx, const node::expression& a_expression, output_side a_side = output_side::lefthand, pass_flags a_flags = pass_flag::none);
        void perform_assignable_node_pass(function_context& a_ctx, const node& a_node, pass_flags a_flags = pass_flag::none);
        void perform_assignable_expression_pass(function_context& a_ctx, const node::expression& a_expression, pass_flags a_flags = pass_flag::none);
    };

    static compiler::output_side operator ! (compiler::output_side a_pos) noexcept {
        return a_pos == compiler::output_side::lefthand ? compiler::output_side::righthand : compiler::output_side::lefthand;
    }
}

#endif //REBAR_COMPILER_HPP
