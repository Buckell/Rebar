//
// Created by maxng on 8/22/2022.
//

#ifndef REBAR_NODE_PASS_HPP
#define REBAR_NODE_PASS_HPP

#include "macro.hpp"

#include "../compiler.hpp"

namespace rebar {
    void compiler::perform_node_pass(function_context& ctx, const node& a_node, output_side a_side, std::optional<const node*> a_next) {
        function_context::pass_control pass(ctx);

        auto [out_type, out_data] = ctx.expression_registers(a_side);
        auto& cc = ctx.assembler;

        switch (a_node.m_type) {
            case node::type::token:
                if (pass.flags_set(pass_flag::identifier_as_string) && a_node.is_token()) {
                    const token &tok = a_node.get_token();

                    if (tok.is_identifier()) {
                        cc.mov(out_type, type::string);
                        cc.mov(out_data, ctx.source.emplace_string_dependency(tok.get_identifier()).data());
                        break;
                    }
                }

                load_token(ctx, a_node.get_token(), a_side);

                break;
            case node::type::expression:
            case node::type::group:
                perform_expression_pass(ctx, a_node.get_expression(), a_side);
                break;
            case node::type::block:
                perform_block_pass(ctx, a_node.get_block());
                break;
            case node::type::if_declaration: {
                 const auto& decl = a_node.get_if_declaration();
                 const auto& conditional = decl.m_conditional;
                 const auto& body = decl.m_body;

                REBAR_CC_DEBUG("If declaration start.");

                ctx.if_stack.back() = cc.newLabel();

                auto label_end = cc.newLabel();

                perform_expression_pass(ctx, conditional, output_side::lefthand);

                REBAR_CC_DEBUG("Compare conditional value.");

                cc.test(ctx.lhs_data, ctx.lhs_data);
                cc.jz(label_end);

                perform_block_pass(ctx, body);

                cc.jmp(*ctx.if_stack.back());

                cc.bind(label_end);

                if (!a_next.has_value() || !((*a_next)->is_else_if_declaration() || (*a_next)->is_else_declaration())) {
                    cc.bind(*ctx.if_stack.back());
                }

                break;
            }
            case node::type::else_if_declaration: {
                const auto& decl = a_node.get_else_if_declaration();
                const auto& conditional = decl.m_conditional;
                const auto& body = decl.m_body;

                REBAR_CC_DEBUG("Else-if declaration start.");

                auto label_end = cc.newLabel();

                perform_expression_pass(ctx, conditional, output_side::lefthand);

                REBAR_CC_DEBUG("Compare conditional value.");

                cc.test(ctx.lhs_data, ctx.lhs_data);
                cc.jz(label_end);

                perform_block_pass(ctx, body);

                cc.jmp(*ctx.if_stack.back());

                cc.bind(label_end);

                if (!a_next.has_value() || !((*a_next)->is_else_if_declaration() || (*a_next)->is_else_declaration())) {
                    cc.bind(*ctx.if_stack.back());
                }

                break;
            }
            case node::type::else_declaration: {
                const auto& decl = a_node.get_else_declaration();

                perform_block_pass(ctx, decl);

                if (!a_next.has_value() || !((*a_next)->is_else_if_declaration() || (*a_next)->is_else_declaration())) {
                    cc.bind(*ctx.if_stack.back());
                }

                break;
            }
            case node::type::for_declaration: {
                const auto& decl = a_node.get_for_declaration();

                // SYNCHRONIZE CHANGES WITH PERFORM_BLOCK_PASS

                function_context::pass_control for_pass(ctx);

                auto& cc = ctx.assembler;
                ctx.local_variable_list.emplace_back();
                ctx.constant_tables.emplace_back();

                ctx.block_local_offsets.emplace_back(0);

                ctx.if_stack.push_back(std::nullopt);

                const auto& label_begin = cc.newLabel();
                const auto& label_end = cc.newLabel();
                const auto& label_body = cc.newLabel();

                ctx.loop_stack.emplace_back(std::in_place, label_begin, label_end);

                const auto& body = decl.m_body;

                perform_expression_pass(ctx, decl.m_initialization);

                cc.jmp(label_body);

                cc.bind(label_begin);

                perform_expression_pass(ctx, decl.m_iteration);

                cc.bind(label_body);

                perform_expression_pass(ctx, decl.m_conditional, a_side);

                cc.test(out_data, out_data);
                cc.jz(label_end);

                for (size_t i = 0; i < body.size(); ++i) {
                    perform_node_pass(ctx, body[i], output_side::lefthand, i + 1 < body.size() ? std::optional<const node*>(&body[i + 1]) : std::nullopt);
                }

                cc.jmp(label_begin);

                cc.bind(label_end);

                ctx.loop_stack.pop_back();

                auto& locals_table = ctx.local_variable_list.back();

                if (!locals_table.empty()) {
                    auto block_local_stack_offset = ctx.block_local_offsets.back();

                    ctx.local_stack_position -= block_local_stack_offset;

                    REBAR_CC_DEBUG("Perform garbage collection. (Offset: %d - %d Objects)", ctx.local_stack_position, locals_table.size());

                    // Dereference and garbage collect locals.
                    asmjit::x86::Mem locals_stack(ctx.locals_stack);
                    locals_stack.addOffset(ctx.local_stack_position * sizeof(object));

                    cc.lea(ctx.identifier, locals_stack);
                    cc.mov(ctx.lhs_type, locals_table.size());

                    REBAR_CODE_GENERATION_GUARD({
                        asmjit::InvokeNode* invoke_node;
                        cc.invoke(&invoke_node, object::block_dereference, asmjit::FuncSignatureT<void, object*, size_t>(platform_call_convention));
                        invoke_node->setArg(0, ctx.identifier);
                        invoke_node->setArg(1, ctx.lhs_type);
                    })

                    pass.set_flags(pass_flag::clobber_identifier | pass_flag::clobber_left);
                }

                ctx.if_stack.pop_back();
                ctx.block_local_offsets.pop_back();
                ctx.local_variable_list.pop_back();
                ctx.constant_tables.pop_back();

                break;
            }
            case node::type::function_declaration: {
                const auto& decl = a_node.get_function_declaration();

                function func = compile_function(ctx.source.puint, decl.m_parameters, decl.m_body);

                std::string_view function_identifier_plaintext = decl.m_identifier.get_string_representation(ctx.source.puint.m_plaintext);

                m_environment.emplace_function_info(func, {
                    std::string(function_identifier_plaintext),
                    "REBAR INTERNAL;COMPILER;"s + std::to_string(m_environment.get_current_function_id_stack()),
                    0,
                    std::make_unique<function_info_source::rebar>(ctx.source.puint.m_plaintext, a_node)
                });

                std::string_view function_name = m_environment.get_function_info(func).get_name();

                REBAR_CC_DEBUG("Performing function declaration. (%s)", function_name);

                ctx.target_flags((decl.m_tags == function_tags::basic) ? pass_flag::local_identifier : pass_flag::none);
                perform_assignable_expression_pass(ctx, decl.m_identifier);

                cc.mov(out_type, type::function);
                cc.movabs(out_data, func.data());

                cc.mov(asmjit::x86::qword_ptr(ctx.identifier), out_type);
                cc.mov(asmjit::x86::qword_ptr(ctx.identifier, object_data_offset), out_data);

                pass.set_flags(side_clobber_flag(a_side) | pass_flag::clobber_identifier);

                break;
            }
            case node::type::while_declaration: {
                const auto& decl = a_node.get_while_declaration();

                auto label_begin = cc.newLabel();
                auto label_end = cc.newLabel();
                auto label_body = cc.newLabel();

                ctx.loop_stack.emplace_back(std::in_place, label_begin, label_end);

                cc.jmp(label_begin);

                cc.bind(label_body);

                perform_block_pass(ctx, decl.m_body);

                cc.bind(label_begin);

                perform_expression_pass(ctx, decl.m_conditional, a_side);

                cc.test(out_data, out_data);
                cc.jnz(label_body);

                cc.bind(label_end);

                ctx.loop_stack.pop_back();

                break;
            }
            case node::type::do_declaration: {
                const auto decl = a_node.get_do_declaration();

                auto label_begin = cc.newLabel();
                auto label_end = cc.newLabel();
                auto label_body = cc.newLabel();

                ctx.loop_stack.emplace_back(std::in_place, label_begin, label_end);

                cc.bind(label_body);

                perform_block_pass(ctx, decl.m_body);

                cc.bind(label_begin);

                perform_expression_pass(ctx, decl.m_conditional, a_side);

                cc.test(out_data, out_data);
                cc.jne(label_body);

                cc.bind(label_end);

                ctx.loop_stack.pop_back();

                break;
            }
            case node::type::switch_declaration:
                break;
            case node::type::class_declaration:
                break;
            case node::type::return_statement: {
                auto& expression = a_node.get_return_statement();

                perform_expression_pass(ctx, expression, a_side);

                cc.mov(asmjit::x86::qword_ptr(ctx.return_object), out_type);
                cc.mov(asmjit::x86::qword_ptr(ctx.return_object, sizeof(void*)), out_data);

                if (ctx.local_stack_position > 0) {
                    // Dereference and garbage collect locals.
                    asmjit::x86::Mem locals_stack(ctx.locals_stack);

                    REBAR_CC_DEBUG("Perform garbage collection. (%d Objects)", ctx.local_stack_position);

                    cc.lea(ctx.identifier, locals_stack);
                    cc.mov(ctx.lhs_type, ctx.local_stack_position);

                    pass.set_flags(pass_flag::clobber_left | pass_flag::clobber_identifier);

                    REBAR_CODE_GENERATION_GUARD({
                        asmjit::InvokeNode* invoke_node;
                        cc.invoke(&invoke_node, object::block_dereference, asmjit::FuncSignatureT<void, object*, size_t>(platform_call_convention));
                        invoke_node->setArg(0, ctx.identifier);
                        invoke_node->setArg(1, ctx.lhs_type);
                    })
                }

                cc.ret();

                pass.set_flags(pass_flag::clobber_return);

                break;
            }
            case node::type::immediate_table: {
                const auto& tbl = a_node.get_immediate_table();

                REBAR_CC_DEBUG("Begin intermediate table.");

                REBAR_CODE_GENERATION_GUARD({
                    asmjit::InvokeNode* table_invoke;
                    cc.invoke(&table_invoke, _ext_allocate_table, asmjit::FuncSignatureT<table*>(platform_call_convention));
                    table_invoke->setRet(0, ctx.identifier);
                })

                for (const auto& entry : tbl.m_entries) {
                    REBAR_CC_DEBUG("Immediate table entry.");

                    // TODO: Optimize: Eliminate unnecessary push/pops.

                    ctx.push_identifier();

                    ctx.target_flags(pass_flag::identifier_as_string);
                    perform_node_pass(ctx, entry.first, output_side::lefthand);

                    ctx.push_side(output_side::lefthand);

                    perform_expression_pass(ctx, entry.second, output_side::righthand);

                    ctx.pop_side(output_side::lefthand);

                    ctx.pop_identifier();

                    cc.mov(asmjit::x86::qword_ptr(ctx.return_object), type::table);
                    cc.mov(asmjit::x86::qword_ptr(ctx.return_object, object_data_offset), ctx.identifier);

                    REBAR_CODE_GENERATION_GUARD({
                        asmjit::InvokeNode* index_invoke;
                        cc.invoke(&index_invoke, _ext_object_index, asmjit::FuncSignatureT<object*, environment*, object*, type, size_t>(platform_call_convention));
                        index_invoke->setArg(0, ctx.environment);
                        index_invoke->setArg(1, ctx.return_object);
                        index_invoke->setArg(2, ctx.lhs_type);
                        index_invoke->setArg(3, ctx.lhs_data);
                        index_invoke->setRet(0, ctx.lhs_data);
                    })

                    cc.mov(asmjit::x86::qword_ptr(ctx.lhs_data), ctx.rhs_type);
                    cc.mov(asmjit::x86::qword_ptr(ctx.lhs_data, object_data_offset), ctx.rhs_data);

                    pass.set_flags(pass_flag::clobber_return | pass_flag::clobber_left);
                }

                cc.mov(out_type, type::table);
                cc.mov(out_data, ctx.identifier);

                pass.set_flags(side_clobber_flag(a_side) | pass_flag::dynamic_expression | pass_flag::clobber_identifier);

                break;
            }
            case node::type::break_statement: {
                size_t index = a_node.get_break_statement();

                cc.jmp((*(ctx.loop_stack.rbegin() + index))->second);

                break;
            }
            case node::type::continue_statement: {
                size_t index = a_node.get_break_statement();

                cc.jmp((*(ctx.loop_stack.rbegin() + index))->first);

                break;
            }
            case node::type::immediate_array: {
                const auto& arr = a_node.get_immediate_array();

                REBAR_CC_DEBUG("Begin intermediate array.");

                cc.mov(ctx.identifier, arr.size());

                REBAR_CODE_GENERATION_GUARD({
                    asmjit::InvokeNode* array_invoke;
                    cc.invoke(&array_invoke, _ext_allocate_array, asmjit::FuncSignatureT<void, object*, size_t>(platform_call_convention));
                    array_invoke->setArg(0, ctx.return_object);
                    array_invoke->setArg(1, ctx.identifier);
                })

                cc.mov(ctx.identifier, asmjit::x86::qword_ptr(ctx.return_object, object_data_offset));

                for (size_t i = 0; i < arr.size(); ++i) {
                    REBAR_CC_DEBUG("Immediate array entry.");

                    // TODO: Optimize: Eliminate unnecessary push/pops.

                    ctx.push_identifier();

                    perform_node_pass(ctx, arr[i], output_side::righthand);

                    ctx.pop_identifier();

                    cc.mov(asmjit::x86::qword_ptr(ctx.return_object), type::array);
                    cc.mov(asmjit::x86::qword_ptr(ctx.return_object, object_data_offset), ctx.identifier);

                    cc.mov(ctx.lhs_type, type::integer);
                    cc.mov(ctx.lhs_data, i);

                    REBAR_CODE_GENERATION_GUARD({
                        asmjit::InvokeNode* index_invoke;
                        cc.invoke(&index_invoke, _ext_object_index, asmjit::FuncSignatureT<object*, environment*, object*, type, size_t>(platform_call_convention));
                        index_invoke->setArg(0, ctx.environment);
                        index_invoke->setArg(1, ctx.return_object);
                        index_invoke->setArg(2, ctx.lhs_type);
                        index_invoke->setArg(3, ctx.lhs_data);
                        index_invoke->setRet(0, ctx.lhs_data);
                    })

                    cc.mov(asmjit::x86::qword_ptr(ctx.lhs_data), ctx.rhs_type);
                    cc.mov(asmjit::x86::qword_ptr(ctx.lhs_data, object_data_offset), ctx.rhs_data);

                    pass.set_flags(pass_flag::clobber_return | pass_flag::clobber_left);
                }

                cc.mov(out_type, type::array);
                cc.mov(out_data, ctx.identifier);

                pass.set_flags(side_clobber_flag(a_side) | pass_flag::dynamic_expression | pass_flag::clobber_identifier);

                break;
            }
            case node::type::selector: {
                const auto &arr = a_node.get_selector();

                if (arr.count() > 1) {
                    break;
                }

                REBAR_CC_DEBUG("Begin intermediate array.");

                cc.mov(ctx.identifier, arr.count() ? 1 : 0);

                REBAR_CODE_GENERATION_GUARD({
                    asmjit::InvokeNode *array_invoke;
                    cc.invoke(&array_invoke, _ext_allocate_array, asmjit::FuncSignatureT<void, object *, size_t>(platform_call_convention));
                    array_invoke->setArg(0, ctx.return_object);
                    array_invoke->setArg(1, ctx.identifier);
                })

                cc.mov(ctx.identifier, asmjit::x86::qword_ptr(ctx.return_object, object_data_offset));

                REBAR_CC_DEBUG("Immediate array entry.");

                // TODO: Optimize: Eliminate unnecessary push/pops.

                if (arr.count() > 0) {
                    ctx.push_identifier();

                    perform_node_pass(ctx, arr.get_operand(0), output_side::righthand);

                    ctx.pop_identifier();

                    cc.mov(asmjit::x86::qword_ptr(ctx.return_object), type::array);
                    cc.mov(asmjit::x86::qword_ptr(ctx.return_object, object_data_offset), ctx.identifier);

                    cc.mov(ctx.lhs_type, type::integer);
                    cc.mov(ctx.lhs_data, 0);

                    REBAR_CODE_GENERATION_GUARD({
                        asmjit::InvokeNode *index_invoke;
                        cc.invoke(&index_invoke, _ext_object_index, asmjit::FuncSignatureT<object *, environment *, object *, type, size_t>(platform_call_convention));
                        index_invoke->setArg(0, ctx.environment);
                        index_invoke->setArg(1, ctx.return_object);
                        index_invoke->setArg(2, ctx.lhs_type);
                        index_invoke->setArg(3, ctx.lhs_data);
                        index_invoke->setRet(0, ctx.lhs_data);
                    })

                    cc.mov(asmjit::x86::qword_ptr(ctx.lhs_data), ctx.rhs_type);
                    cc.mov(asmjit::x86::qword_ptr(ctx.lhs_data, object_data_offset), ctx.rhs_data);

                    pass.set_flags(pass_flag::clobber_return | pass_flag::clobber_left);
                }

                cc.mov(out_type, type::array);
                cc.mov(out_data, ctx.identifier);

                pass.set_flags(side_clobber_flag(a_side) | pass_flag::dynamic_expression | pass_flag::clobber_identifier);

                break;
            }
            default:
                break;
        }
    }

    void compiler::perform_assignable_node_pass(function_context& ctx, const node& a_node) {
        function_context::pass_control pass(ctx);

        if (a_node.is_token()) {
            const token& tok = a_node.get_token();

            if (tok.is_identifier()) {
                 load_identifier_pointer(ctx, tok.get_identifier());
            }
        } else if (a_node.is_group() || a_node.is_selector() || a_node.is_expression()) {
             perform_assignable_expression_pass(ctx, a_node.get_expression());
        }
    }
}

#endif //REBAR_NODE_PASS_HPP
