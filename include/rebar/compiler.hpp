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

    struct compiler_flag {
        DEFINE_REBAR_FLAG(none, 0);
        DEFINE_REBAR_FLAG(optimize_bypass_register_save, 1);
        DEFINE_REBAR_FLAG(optimize_constant_expressions, 2);

        constexpr static flags default_flags = optimize_bypass_register_save
                                             | optimize_constant_expressions;
    };

    class compiler : public provider {
        environment& m_environment;

        std::unordered_map<callable, std::unique_ptr<compiler_function_source>> m_functions;

        asmjit::JitRuntime m_runtime;
        asmjit::FileLogger m_logger;

        flags m_flags = compiler_flag::default_flags;

    public:
        constexpr static asmjit::CallConvId platform_call_convention = platform::current & platform::windows ? asmjit::CallConvId::kX64Windows : asmjit::CallConvId::kHost;

        // TODO: Modify preliminary scan to determine needed allocation size;
        constexpr static size_t default_temporary_stack_allocation = 6;

        explicit compiler(environment& a_environment) noexcept : m_environment(a_environment), m_logger(stdout) {}

        [[nodiscard]] flags& get_flags() noexcept {
            return m_flags;
        }

        [[nodiscard]] bool flags_set(flags a_flags) noexcept {
            return m_flags & a_flags;
        }

        void set_flags(flags a_flags) noexcept {
            m_flags |= a_flags;
        }

        void unset_flags(flags a_flags) noexcept {
            m_flags &= (~a_flags);
        }

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

        struct pass_flag {
            DEFINE_REBAR_FLAG(none, 0);

            // Pass Flags

            // Converts identifiers into strings. Useful for indexing operations.
            DEFINE_REBAR_FLAG(identifier_as_string,         1);
            // Treats an identifier as a local definition. Useful for local function definitions.
            DEFINE_REBAR_FLAG(local_identifier,             2);
            // Voids machine code generation. Useful for test/scan passes.
            DEFINE_REBAR_FLAG(void_code_generation,         3);
            // Evaluates expression as constant.
            DEFINE_REBAR_FLAG(evaluate_constant_expression, 4);

            // Return Flags

            DEFINE_REBAR_FLAG(clobber_left,         1);
            DEFINE_REBAR_FLAG(clobber_right,        2);
            DEFINE_REBAR_FLAG(clobber_identifier,   3);
            DEFINE_REBAR_FLAG(clobber_return,       4);
            DEFINE_REBAR_FLAG(clobber_transfer,     5);

            DEFINE_REBAR_FLAG(dynamic_expression,  10);
            DEFINE_REBAR_FLAG(constant_assignable, 11);
        };

        struct function_context {
            asmjit::x86::Compiler& assembler;
            compiler_function_source& source;

            std::vector<std::unordered_map<std::string_view, object>> constant_tables;

            // Local variable stack offsets.
            // Local definitions state. (Whether local is defined or not.)
            std::vector<std::unordered_map<std::string_view, size_t>> local_variable_list;
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

            asmjit::x86::Xmm transfer;

            asmjit::x86::Mem temporary_store_stack;

            asmjit::x86::Mem function_argument_stack;

            std::vector<asmjit::x86::Mem> argument_stack;
            size_t current_argument_ref = 0;

            object* constant_identifier;
            object constant_left;
            object constant_right;

            size_t pass_index = 0;

            // Top value for next recursive pass call.
            std::vector<flags> input_flag_stack;

            // Top value for previous call.
            std::vector<flags> output_flag_stack;

            flags last_output;

            function_context(asmjit::x86::Compiler& a_cc, compiler_function_source& a_source) : assembler(a_cc), source(a_source) {
                input_flag_stack.push_back(pass_flag::none); // Next pass.
                output_flag_stack.push_back(pass_flag::none); // Current pass.
            }

            object& constant_side(output_side a_side) {
                return a_side == output_side::righthand ? constant_right : constant_left;
            }

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

            [[maybe_unused]] void emplace_raw_side_output(output_side a_side, size_t a_annotation = 0) {
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

            void efficient_load_integer(asmjit::x86::Gp& a_reg, size_t a_integer) {
                auto& cc = assembler;

                if (a_integer == 0) {
                    cc.xor_(a_reg, a_reg);
                } else if (a_integer < std::numeric_limits<uint32_t>::max()) {
                    cc.mov(a_reg, a_integer);
                } else {
                    cc.movabs(a_reg, a_integer);
                }
            }

            void load_constant(output_side a_side, const object& a_object) {
                auto& cc = assembler;
                auto [out_type, out_data] = expression_registers(a_side);

                efficient_load_integer(out_type, static_cast<size_t>(a_object.object_type()));
                efficient_load_integer(out_data, a_object.data());
            }

            void enable_code_generation(bool a_enable) {
                assembler._code = a_enable ? &source.code : nullptr;
            }

            [[nodiscard]] bool is_code_generation_enabled() const noexcept {
                return assembler._code != nullptr;
            }

            size_t begin_pass() {
                input_flag_stack.push_back(input_flag_stack.back()); // Next pass.
                output_flag_stack.push_back(pass_flag::none); // Current pass.

                ++pass_index;

                if (get_input_flags() & pass_flag::void_code_generation) {
                    enable_code_generation(false);
                }

                return pass_index;
            }

            void end_pass() {
                flags out_flags = get_output_flags();
                output_flag_stack.pop_back();
                output_flag_stack.back() |= out_flags;
                last_output = out_flags;

                input_flag_stack.pop_back();
                input_flag_stack.back() = input_flag_stack.size() >= 2 ? *(input_flag_stack.end() - 2) : pass_flag::none;

                if (!--pass_index) {
                    return;
                }

                if (!(get_input_flags() & pass_flag::void_code_generation)) {
                    enable_code_generation(true);
                }
            }

            flags& get_output_flags() {
                return output_flag_stack[pass_index];
            }

            flags& get_input_flags() {
                return input_flag_stack[pass_index - 1];
            }

            flags& next_pass_flags() {
                return input_flag_stack[pass_index];
            }

            flags& get_output_flags(size_t a_pass_index) {
                return output_flag_stack[a_pass_index];
            }

            flags& get_input_flags(size_t a_pass_index) {
                return input_flag_stack[a_pass_index - 1];
            }

            flags& next_pass_flags(size_t a_pass_index) {
                return input_flag_stack[a_pass_index];
            }

            // Set output flag(s).
            void set_flags(flags a_flags) noexcept {
                get_output_flags() |= a_flags;
            }

            // Set input flag(s) of next pass.
            void target_flags(flags a_flags) noexcept {
                next_pass_flags() |= a_flags;
            }

            // Set input flag(s) of next pass.
            void unset_target_flags(flags a_flags) noexcept {
                next_pass_flags() &= ~a_flags;
            }

            [[nodiscard]] flags output_flags() const noexcept {
                return last_output;
            }

            [[nodiscard]] bool out_flags_set(flags a_flags) noexcept {
                return get_input_flags() & a_flags;
            }

            // RAII controller for passes.
            struct pass_control {
                function_context& ctx;
                size_t pass_index;

                // Assigning references to flags at construction would be efficient, but
                // risks references being invalidated upon creating additional passes.
                pass_control(function_context& a_ctx) : ctx(a_ctx), pass_index(a_ctx.begin_pass()) {}

                ~pass_control() {
                    ctx.end_pass();
                }

                // Set output flag(s).
                void set_flags(flags a_flags) noexcept {
                    ctx.get_output_flags(pass_index) |= a_flags;
                }

                // Check current input flags.
                [[nodiscard]] bool flags_set(flags a_flags) noexcept {
                    return ctx.get_input_flags(pass_index) & a_flags;
                }
            };
        };

        void combine_return_flags(flags& a_parent, flags a_child) {
            a_parent |= a_child;
        }

        flags side_clobber_flag(output_side a_side) {
            return a_side == output_side::righthand ? pass_flag::clobber_right : pass_flag::clobber_left;
        }

    private:
        function compile_function(parse_unit& a_unit, const node::parameter_list& a_parameters, const node::block& a_block);

        void load_token(function_context& ctx, const token& a_token, output_side a_side);
        void load_identifier(function_context& ctx, std::string_view a_identifier, output_side a_side);
        void load_identifier_pointer(function_context& ctx, std::string_view a_identifier);

        void perform_preliminary_function_scan(function_context& ctx, const node::block& a_block);

        void perform_block_pass(function_context& ctx, const node::block& a_block);
        void perform_node_pass(function_context& ctx, const node& a_node, output_side a_side = output_side::lefthand);
        void perform_expression_pass(function_context& ctx, const node::expression& a_expression, output_side a_side = output_side::lefthand);
        void perform_assignable_node_pass(function_context& ctx, const node& a_node);
        void perform_assignable_expression_pass(function_context& ctx, const node::expression& a_expression);
    };

    static compiler::output_side operator ! (compiler::output_side a_pos) noexcept {
        return a_pos == compiler::output_side::lefthand ? compiler::output_side::righthand : compiler::output_side::lefthand;
    }
}

#endif //REBAR_COMPILER_HPP
