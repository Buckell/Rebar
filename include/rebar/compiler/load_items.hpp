//
// Created by maxng on 8/21/2022.
//

#ifndef REBAR_LOAD_ITEMS_HPP
#define REBAR_LOAD_ITEMS_HPP

#include "ext.hpp"
#include "macro.hpp"

#include "../compiler.hpp"

namespace rebar {
    void compiler::load_token(function_context& ctx, const token& a_token, const output_side a_side) {
        function_context::pass_control pass(ctx);

        auto [out_type, out_data] = ctx.expression_registers(a_side);
        auto& cc = ctx.assembler;

        object& constant_assignable = ctx.constant_side(a_side);
        bool evaluate_constant = pass.flags_set(pass_flag::evaluate_constant_expression);

        switch (a_token.m_type) {
            case token::type::string_literal: {
                REBAR_CC_DEBUG("Loading string literal. (\"%s\")", a_token.get_string_literal().data());

                string literal = ctx.source.emplace_string_dependency(a_token.get_string_literal());

                cc.mov(out_type, type::string);
                cc.movabs(out_data, literal.data());

                if (evaluate_constant) {
                    constant_assignable = literal;
                }

                break;
            }
            case token::type::identifier:
                load_identifier(ctx, a_token.get_identifier(), a_side);
                break;
            case token::type::integer_literal:
                REBAR_CC_DEBUG("Loading integer literal. (%d)", a_token.get_integer_literal());

                cc.mov(out_type, type::integer);
                ctx.efficient_load_integer(out_data, a_token.get_integer_literal());

                if (evaluate_constant) {
                    constant_assignable = a_token.get_integer_literal();
                }

                break;
            case token::type::number_literal:
                REBAR_CC_DEBUG("Loading number literal. (%f)", a_token.get_number_literal());

                cc.mov(out_type, type::number);
                cc.movabs(out_data, a_token.get_number_literal());

                if (evaluate_constant) {
                    constant_assignable = a_token.get_number_literal();
                }

                break;
            case token::type::keyword:
                switch (a_token.get_keyword()) {
                    case keyword::literal_true:
                        REBAR_CC_DEBUG("Loading boolean literal. (true)");

                        cc.mov(out_type, type::boolean);
                        cc.mov(out_data, true);

                        if (evaluate_constant) {
                            constant_assignable = boolean_true;
                        }

                        break;
                    case keyword::literal_false:
                        REBAR_CC_DEBUG("Loading boolean literal. (false)");

                        cc.mov(out_type, type::boolean);
                        cc.xor_(out_data, out_data);

                        if (evaluate_constant) {
                            constant_assignable = boolean_false;
                        }

                        break;
                    case keyword::literal_null:
                        REBAR_CC_DEBUG("Loading null literal.");

                        cc.xor_(out_type, out_type);
                        cc.xor_(out_data, out_data);

                        if (evaluate_constant) {
                            constant_assignable = null;
                        }

                        break;
                    default:
                        break;
                }
                break;

            default: // Theoretically, this should never happen.
                break;
        }
    }

    void compiler::load_identifier(function_context& ctx, std::string_view a_identifier, output_side a_side) {
        function_context::pass_control pass(ctx);

        auto [out_type, out_data] = ctx.expression_registers(a_side);
        auto& cc = ctx.assembler;

        object& constant_assignable = ctx.constant_side(a_side);
        bool evaluate_constant = pass.flags_set(pass_flag::evaluate_constant_expression);

        auto& local_tables = ctx.local_variable_list;
        auto& constant_tables = ctx.constant_tables;

        // Find identifier in local tables.
        auto constant_table_it = constant_tables.rbegin();
        for (auto local_table_it = local_tables.rbegin(); local_table_it != local_tables.crend(); ++local_table_it, ++constant_table_it) {
            for (auto& entry : *constant_table_it) {
                if (entry.first == a_identifier) {
                    ctx.load_constant(a_side, entry.second);

                    if (evaluate_constant) {
                        constant_assignable = entry.second;
                    }

                    pass.set_flags(side_clobber_flag(a_side));
                    return;
                }
            }

            for (auto& entry : *local_table_it) {
                // TODO: Apply additional constraint: choose highest stack offset.
                // (Choose local--with same identifier--defined last.)
                if (entry.first == a_identifier) {
                    REBAR_CC_DEBUG("Loading local identifier. (Stack Pos. %d - \"%s\")", entry.second, a_identifier.data());

                    asmjit::x86::Mem locals_stack(ctx.locals_stack);
                    locals_stack.addOffset(static_cast<uint64_t>(entry.second * sizeof(object)));
                    locals_stack.setSize(sizeof(void*));

                    cc.mov(out_type, locals_stack);

                    locals_stack.addOffset(sizeof(void*));
                    cc.mov(out_data, locals_stack);

                    pass.set_flags(pass_flag::dynamic_expression | side_clobber_flag(a_side));
                    return;
                }
            }
        }

        for (size_t i = 0; i < ctx.source.parameters.size(); ++i) {
            const auto& parameter = ctx.source.parameters[i];

            if (parameter.identifier == a_identifier) {
                auto parameter_space(ctx.function_argument_stack);
                parameter_space.addOffset(i * sizeof(object));

                cc.mov(out_type, parameter_space);

                parameter_space.addOffset(object_data_offset);
                cc.mov(out_data, parameter_space);

                pass.set_flags(pass_flag::dynamic_expression | side_clobber_flag(a_side));
                return;
            }
        }

        // Pull global.

        string o = ctx.source.emplace_string_dependency(a_identifier);

        cc.mov(out_type, type::string);
        cc.mov(out_data, o.data());

        REBAR_CODE_GENERATION_GUARD({
            asmjit::InvokeNode* invoke_node;
            cc.invoke(&invoke_node, _ext_index_global, asmjit::FuncSignatureT<object*, environment*, uint64_t, uint64_t>(asmjit::CallConvId::kX64Windows));
            invoke_node->setArg(0, ctx.environment);
            invoke_node->setArg(1, out_type);
            invoke_node->setArg(2, out_data);
            invoke_node->setRet(0, out_data);
        })

        cc.mov(out_type, asmjit::x86::qword_ptr(out_data));
        cc.mov(out_data, asmjit::x86::qword_ptr(out_data, sizeof(void*)));

        pass.set_flags(pass_flag::dynamic_expression | side_clobber_flag(a_side));
        return;
    }

    void compiler::load_identifier_pointer(function_context& ctx, const std::string_view a_identifier) {
        function_context::pass_control pass(ctx);

        auto& cc = ctx.assembler;

        auto& local_tables = ctx.local_variable_list;
        auto& constant_tables = ctx.constant_tables;

        // Find identifier in local tables.
        auto constant_table_it = constant_tables.rbegin();
        for (auto local_table_it = local_tables.rbegin(); local_table_it != local_tables.crend(); ++local_table_it, ++constant_table_it) {
            for (auto& entry : *constant_table_it) {
                if (entry.first == a_identifier) {
                    ctx.constant_identifier = &entry.second;
                    pass.set_flags(pass_flag::constant_assignable);
                    return;
                }
            }

            for (auto& entry : *local_table_it) {
                if (entry.first == a_identifier) {
                    REBAR_CC_DEBUG("Loading local identifier pointer. (Stack Pos. %d - \"%s\")", entry.second, a_identifier.data());

                    asmjit::x86::Mem locals_stack(ctx.locals_stack);
                    locals_stack.addOffset(static_cast<uint64_t>(entry.second * sizeof(object)));

                    cc.lea(ctx.identifier, locals_stack);

                    pass.set_flags(pass_flag::clobber_identifier | pass_flag::dynamic_expression);
                    return;
                }
            }
        }

        for (size_t i = 0; i < ctx.source.parameters.size(); ++i) {
            const auto& parameter = ctx.source.parameters[i];

            if (parameter.identifier == a_identifier) {
                auto parameter_space(ctx.function_argument_stack);
                parameter_space.addOffset(i * sizeof(object));

                cc.lea(ctx.identifier, parameter_space);

                pass.set_flags(pass_flag::clobber_identifier | pass_flag::dynamic_expression);
            }
        }

        // Pull global.

        string o = ctx.source.emplace_string_dependency(a_identifier);

        cc.mov(ctx.lhs_type, type::string);
        cc.mov(ctx.lhs_data, o.data());

        REBAR_CODE_GENERATION_GUARD({
            asmjit::InvokeNode* invoke_node;
            cc.invoke(&invoke_node, _ext_index_global, asmjit::FuncSignatureT<object*, environment*, uint64_t, uint64_t>(platform_call_convention));
            invoke_node->setArg(0, ctx.environment);
            invoke_node->setArg(1, ctx.lhs_type);
            invoke_node->setArg(2, ctx.lhs_data);
            invoke_node->setRet(0, ctx.identifier);
        })

        pass.set_flags(pass_flag::clobber_identifier | pass_flag::dynamic_expression | pass_flag::clobber_left);
    }
}

#endif //REBAR_LOAD_ITEMS_HPP
