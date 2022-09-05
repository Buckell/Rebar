//
// Created by maxng on 8/21/2022.
//

#ifndef REBAR_LOAD_ITEMS_HPP
#define REBAR_LOAD_ITEMS_HPP

#include "ext.hpp"
#include "macro.hpp"

#include "../compiler.hpp"

namespace rebar {
    void compiler::load_token(function_context& a_ctx, const token& a_token, const output_side a_side, pass_flags a_flags) {
        auto [out_type, out_data] = a_ctx.expression_registers(a_side);
        auto& cc = a_ctx.assembler;

        switch (a_token.m_type) {
            case token::type::string_literal:
                REBAR_CC_DEBUG("Loading string literal. (\"%s\")", a_token.get_string_literal().data());

                cc.mov(out_type, type::string);
                cc.movabs(out_data, a_ctx.source.emplace_string_dependency(a_token.get_string_literal()).data());
                break;
            case token::type::identifier:
                load_identifier(a_ctx, a_token.get_identifier(), a_side);
                break;
            case token::type::integer_literal:
                REBAR_CC_DEBUG("Loading integer literal. (%d)", a_token.get_integer_literal());

                cc.mov(out_type, type::integer);
                cc.movabs(out_data, a_token.get_integer_literal());
                break;
            case token::type::number_literal:
                REBAR_CC_DEBUG("Loading number literal. (%f)", a_token.get_number_literal());

                cc.mov(out_type, type::number);
                cc.movabs(out_data, a_token.get_number_literal());
                break;
            case token::type::keyword:
                switch (a_token.get_keyword()) {
                    case keyword::literal_true:
                        REBAR_CC_DEBUG("Loading boolean literal. (true)");

                        cc.mov(out_type, type::boolean);
                        cc.mov(out_data, true);

                        break;
                    case keyword::literal_false:
                        REBAR_CC_DEBUG("Loading boolean literal. (false)");

                        cc.mov(out_type, type::boolean);
                        cc.xor_(out_data, out_data);

                        break;
                    case keyword::literal_null:
                        REBAR_CC_DEBUG("Loading null literal.");

                        cc.xor_(out_type, out_type);
                        cc.xor_(out_data, out_data);

                        break;
                    default:
                        break;
                }
                break;

            default: // Theoretically, this should never happen.
                break;
        }
    }

    void compiler::load_identifier(function_context& a_ctx, std::string_view a_identifier, output_side a_side, pass_flags a_flags) {
        auto [out_type, out_data] = a_ctx.expression_registers(a_side);
        auto& cc = a_ctx.assembler;

        auto& local_tables = a_ctx.local_variable_list;

        // Find identifier in local tables.
        if (local_tables.size() > 0) {
            for (auto it = local_tables.end() - 1; it >= local_tables.begin(); --it) {
                for (auto it1 = it->begin(); it1 != it->cend(); ++it1) {
                    // TODO: Apply additional constraint: choose highest stack offset.
                    // (Choose local--with same identifier--defined last.)
                    if (it1->first == a_identifier) {
                        if constexpr (debug_mode) {
                            cc.commentf("Loading local identifier. (Stack Pos. %d - \"%s\")", it1->second, a_identifier.data());
                        }

                        asmjit::x86::Mem locals_stack(a_ctx.locals_stack);
                        locals_stack.addOffset(static_cast<uint64_t>(it1->second * sizeof(object)));
                        locals_stack.setSize(sizeof(void*));

                        cc.mov(out_type, locals_stack);

                        locals_stack.addOffset(sizeof(void*));
                        cc.mov(out_data, locals_stack);

                        return;
                    }
                }

                // MSVC Compatibility: "can't decrement vector iterator before begin"
                if (it == local_tables.cbegin()) {
                    break;
                }
            }

            for (size_t i = 0; i < a_ctx.source.parameters.size(); ++i) {
                const auto& parameter = a_ctx.source.parameters[i];

                if (parameter.identifier == a_identifier) {
                    auto parameter_space(a_ctx.function_argument_stack);
                    parameter_space.addOffset(i * sizeof(object));

                    cc.mov(out_type, parameter_space);

                    parameter_space.addOffset(object_data_offset);
                    cc.mov(out_data, parameter_space);

                    return;
                }
            }
        }

        // Pull global.

        string o = a_ctx.source.emplace_string_dependency(a_identifier);

        cc.mov(out_type, type::string);
        cc.mov(out_data, o.data());

        asmjit::InvokeNode* invoke_node;
        cc.invoke(&invoke_node, _ext_index_global, asmjit::FuncSignatureT<object*, environment*, uint64_t, uint64_t>(asmjit::CallConvId::kX64Windows));

        invoke_node->setArg(0, a_ctx.environment);
        invoke_node->setArg(1, out_type);
        invoke_node->setArg(2, out_data);

        invoke_node->setRet(0, out_data);

        cc.mov(out_type, asmjit::x86::qword_ptr(out_data));
        cc.mov(out_data, asmjit::x86::qword_ptr(out_data, sizeof(void*)));
    }

    void compiler::load_identifier_pointer(function_context& a_ctx, const std::string_view a_identifier, pass_flags a_flags) {
        auto& cc = a_ctx.assembler;

        auto& local_tables = a_ctx.local_variable_list;

        // Find identifier in local tables.
        for (auto it = local_tables.end() - 1; it >= local_tables.begin(); --it) {
            for (auto it1 = it->begin(); it1 != it->cend(); ++it1) {
                // TODO: Apply additional constraint: choose highest stack offset.
                // (Choose local--with same identifier--defined last.)
                if (it1->first == a_identifier) {
                    if constexpr (debug_mode) {
                        cc.commentf("Loading local identifier pointer. (Stack Pos. %d - \"%s\")", it1->second, a_identifier.data());
                    }

                    asmjit::x86::Mem locals_stack(a_ctx.locals_stack);
                    locals_stack.addOffset(static_cast<uint64_t>(it1->second * sizeof(object)));

                    cc.lea(a_ctx.identifier, locals_stack);

                    return;
                }
            }

            // MSVC Compatibility: "can't decrement vector iterator before begin"
            if (it == local_tables.cbegin()) {
                break;
            }
        }

        for (size_t i = 0; i < a_ctx.source.parameters.size(); ++i) {
            const auto& parameter = a_ctx.source.parameters[i];

            if (parameter.identifier == a_identifier) {
                auto parameter_space(a_ctx.function_argument_stack);
                parameter_space.addOffset(i * sizeof(object));

                cc.lea(a_ctx.identifier, parameter_space);

                return;
            }
        }

        // Pull global.

        string o = a_ctx.source.emplace_string_dependency(a_identifier);

        cc.mov(a_ctx.lhs_type, type::string);
        cc.mov(a_ctx.lhs_data, o.data());

        asmjit::InvokeNode* invoke_node;
        cc.invoke(&invoke_node, _ext_index_global, asmjit::FuncSignatureT<object*, environment*, uint64_t, uint64_t>(platform_call_convention));

        invoke_node->setArg(0, a_ctx.environment);
        invoke_node->setArg(1, a_ctx.lhs_type);
        invoke_node->setArg(2, a_ctx.lhs_data);

        invoke_node->setRet(0, a_ctx.identifier);
    }
}

#endif //REBAR_LOAD_ITEMS_HPP
