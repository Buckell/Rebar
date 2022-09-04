//
// Created by maxng on 8/22/2022.
//

#ifndef REBAR_NODE_PASS_HPP
#define REBAR_NODE_PASS_HPP

#include "macro.hpp"

#include "../compiler.hpp"

namespace rebar {
    void compiler::perform_node_pass(function_context& a_ctx, const node& a_node, output_side a_side) {
        auto [out_type, out_data] = a_ctx.expression_registers(a_side);
        auto& cc = a_ctx.assembler;

        switch (a_node.m_type) {
            case node::type::token:
                load_token(a_ctx, a_node.get_token(), a_side);
                break;
            case node::type::expression:
            case node::type::group:
                perform_expression_pass(a_ctx, a_node.get_expression(), a_side);
                break;
            case node::type::block:
                perform_block_pass(a_ctx, a_node.get_block());
                break;
            case node::type::selector:
                break;
            case node::type::ranged_selector:
                break;
            case node::type::argument_list:
                break;
            case node::type::if_declaration: {
                 const auto& decl = a_node.get_if_declaration();
                 const auto& conditional = decl.m_conditional;
                 const auto& body = decl.m_body;

                REBAR_CC_DEBUG("If declaration start.");

                const auto& label_end = cc.newLabel();

                perform_expression_pass(a_ctx, conditional, output_side::lefthand);

                REBAR_CC_DEBUG("Compare conditional value.");

                cc.cmp(a_ctx.lhs_data, 0);
                cc.je(label_end);

                perform_block_pass(a_ctx, body);

                cc.bind(label_end);

                break;
            }
            case node::type::else_if_declaration:
                break;
            case node::type::else_declaration:
                break;
            case node::type::for_declaration:
                break;
            case node::type::function_declaration: {
                const auto& decl = a_node.get_function_declaration();

                function func = compile_function(a_ctx.source.puint, decl.m_parameters, decl.m_body);

                std::string_view function_identifier_plaintext = decl.m_identifier.get_string_representation(a_ctx.source.puint.m_plaintext, 1);

                m_environment.emplace_function_info(func, {
                    std::string(function_identifier_plaintext),
                    "REBAR INTERNAL;COMPILER;"s + std::to_string(m_environment.get_current_function_id_stack()),
                    0,
                    std::make_unique<function_info_source::rebar>(a_ctx.source.puint.m_plaintext, a_node)
                });

                std::string_view function_name = m_environment.get_function_info(func).get_name();

                REBAR_CC_DEBUG("Performing function declaration. (%s)", function_name);

                perform_assignable_expression_pass(a_ctx, decl.m_identifier);

                cc.mov(out_type, type::function);
                cc.movabs(out_data, func.data());

                cc.mov(asmjit::x86::qword_ptr(a_ctx.identifier), out_type);
                cc.mov(asmjit::x86::qword_ptr(a_ctx.identifier, object_data_offset), out_data);

                break;
            }
            case node::type::while_declaration:
                break;
            case node::type::do_declaration:
                break;
            case node::type::switch_declaration:
                break;
            case node::type::class_declaration:
                break;
            case node::type::return_statement: {
                auto& expression = a_node.get_return_statement();

                perform_expression_pass(a_ctx, expression, a_side);

                cc.mov(asmjit::x86::qword_ptr(a_ctx.return_object), out_type);
                cc.mov(asmjit::x86::qword_ptr(a_ctx.return_object, sizeof(void*)), out_data);

                if (a_ctx.local_stack_position > 0) {
                    // Dereference and garbage collect locals.
                    asmjit::x86::Mem locals_stack(a_ctx.locals_stack);

                    REBAR_CC_DEBUG("Perform garbage collection. (%d Objects)", a_ctx.local_stack_position);

                    cc.lea(a_ctx.identifier, locals_stack);
                    cc.mov(a_ctx.lhs_type, a_ctx.local_stack_position);

                    asmjit::InvokeNode* invoke_node;
                    cc.invoke(&invoke_node, object::block_dereference, asmjit::FuncSignatureT<void, object*, size_t>(platform_call_convention));
                    invoke_node->setArg(0, a_ctx.identifier);
                    invoke_node->setArg(1, a_ctx.lhs_type);
                }

                cc.ret();

                break;
            }
            case node::type::immediate_table: {
                const auto& tbl = a_node.get_immediate_table();

                REBAR_CC_DEBUG("Begin intermediate table.");

                asmjit::InvokeNode* table_invoke;
                cc.invoke(&table_invoke, _ext_allocate_table, asmjit::FuncSignatureT<table*>(platform_call_convention));
                table_invoke->setRet(0, a_ctx.identifier);

                for (const auto& entry : tbl.m_entries) {
                    REBAR_CC_DEBUG("Immediate table entry.");

                    a_ctx.push_identifier();

                    perform_detail_node_pass(a_ctx, entry.first, node_detail::identifier_as_string, output_side::lefthand);

                    a_ctx.push_side(output_side::lefthand);

                    perform_expression_pass(a_ctx, entry.second, output_side::righthand);

                    a_ctx.pop_side(output_side::lefthand);

                    a_ctx.pop_identifier();
                    cc.mov(asmjit::x86::qword_ptr(a_ctx.return_object), type::table);
                    cc.mov(asmjit::x86::qword_ptr(a_ctx.return_object, object_data_offset), a_ctx.identifier);

                    asmjit::InvokeNode* index_invoke;
                    cc.invoke(&index_invoke, _ext_object_index, asmjit::FuncSignatureT<object*, environment*, object*, type, size_t>(platform_call_convention));
                    index_invoke->setArg(0, a_ctx.environment);
                    index_invoke->setArg(1, a_ctx.return_object);
                    index_invoke->setArg(2, a_ctx.lhs_type);
                    index_invoke->setArg(3, a_ctx.lhs_data);
                    index_invoke->setRet(0, a_ctx.lhs_data);

                    cc.mov(asmjit::x86::qword_ptr(a_ctx.lhs_data), a_ctx.rhs_type);
                    cc.mov(asmjit::x86::qword_ptr(a_ctx.lhs_data, object_data_offset), a_ctx.rhs_data);
                }

                cc.mov(out_type, type::table);
                cc.mov(out_data, a_ctx.identifier);

                break;
            }
            case node::type::break_statement:
                break;
            case node::type::continue_statement:
                break;
            case node::type::immediate_array:
                break;
            default:
                break;
        }
    }

    void compiler::perform_assignable_node_pass(function_context& a_ctx, const node& a_node) {
        if (a_node.is_token()) {
            const token& tok = a_node.get_token();

            if (tok.is_identifier()) {
                return load_identifier_pointer(a_ctx, tok.get_identifier());
            }
        } else if (a_node.is_group() || a_node.is_selector() || a_node.is_expression()) {
            const auto& expression = a_node.get_expression();
            perform_assignable_expression_pass(a_ctx, expression);
        }
    }

    void compiler::perform_detail_node_pass(function_context& a_ctx, const node& a_node, size_t a_flags, output_side a_side) {
        auto [out_type, out_data] = a_ctx.expression_registers(a_side);
        auto& cc = a_ctx.assembler;

        if (a_flags & node_detail::identifier_as_string) {
            if (a_node.is_token()) {
                const token &tok = a_node.get_token();

                if (tok.is_identifier()) {
                    cc.mov(out_type, type::string);
                    cc.mov(out_data, a_ctx.source.emplace_string_dependency(tok.get_identifier()).data());
                    return;
                }
            }
        }

        return perform_node_pass(a_ctx, a_node, a_side);
    }
}

#endif //REBAR_NODE_PASS_HPP
