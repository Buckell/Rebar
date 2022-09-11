//
// Created by maxng on 8/22/2022.
//

#ifndef REBAR_EXPRESSION_PASS_HPP
#define REBAR_EXPRESSION_PASS_HPP

#include "ext.hpp"
#include "macro.hpp"

#include "../compiler.hpp"

#include "../environment.hpp"

namespace rebar {
    void compiler::perform_expression_pass(function_context& ctx, const node::expression& a_expression, output_side a_side) {
        function_context::pass_control pass(ctx);

        pass.set_flags(side_clobber_flag(a_side));

        auto [out_type, out_data] = ctx.expression_registers(a_side);
        auto [opp_out_type, opp_out_data] = ctx.expression_registers(!a_side);
        auto& cc = ctx.assembler;

        switch (a_expression.get_operation()) {
            case separator::space:
                perform_node_pass(ctx, a_expression.get_operand(0), a_side);
                break;
            case separator::assignment: {
                pass.set_flags(pass_flag::dynamic_expression);

                REBAR_CC_DEBUG("Perform assignment.");

                pass.target_flags(pass_flag::void_code_generation);
                perform_assignable_node_pass(ctx, a_expression.get_operand(0));
                auto pre_assignable_pass = pass.output_flags();

                /*if (pre_assignable_pass & pass_flag::constant_assignable) {
                    if (pre_value_pass & pass_flag::dynamic_expression) {
                        // TODO: Throw exception, dynamic expression assigned to constant.
                        m_environment.cout() << "CONSTANT EXCEPTION" << std::endl;
                    } else {
                        *ctx.constant_identifier = ctx.constant_side(a_side);
                    }
                } else {
                    perform_node_pass(ctx, a_expression.get_operand(1), a_side);

                    const auto& reference_needed = cc.newLabel();
                    const auto& reference_unneeded = cc.newLabel();

                    REBAR_CC_DEBUG("Test referencing.");

                    // Test if object needs explicit referencing.
                    cc.cmp(out_type, object::simple_type_end_boundary);
                    cc.jg(reference_needed);
                    cc.jmp(reference_unneeded);

                    cc.bind(reference_needed);

                    REBAR_CC_DEBUG("Call reference.");

                    REBAR_CODE_GENERATION_GUARD({
                        asmjit::InvokeNode* reference_func_invoke;
                        cc.invoke(&reference_func_invoke, _ext_reference_object, asmjit::FuncSignatureT<void, size_t, size_t>(platform_call_convention));
                        reference_func_invoke->setArg(0, out_type);
                        reference_func_invoke->setArg(1, out_data);
                    })

                    cc.bind(reference_unneeded);

                    REBAR_CC_DEBUG("Push value.");

                    // TODO: Optimization: See if side stack operations can be avoided.

                    ctx.push_side(a_side);

                    REBAR_CC_DEBUG("Evaluate assignee.");

                    perform_assignable_node_pass(ctx, a_expression.get_operand(0));

                    cc.mov(out_type, asmjit::x86::qword_ptr(ctx.identifier));
                    cc.mov(out_data, asmjit::x86::qword_ptr(ctx.identifier, object_data_offset));

                    REBAR_CC_DEBUG("Dereference origin data if needed.");

                    const auto& dereference_needed = cc.newLabel();
                    const auto& dereference_unneeded = cc.newLabel();

                    // Test if object needs explicit referencing.
                    cc.cmp(out_type, object::simple_type_end_boundary);
                    cc.jg(dereference_needed);
                    cc.jmp(dereference_unneeded);

                    cc.bind(dereference_needed);

                    REBAR_CC_DEBUG("Dereference function.");

                    REBAR_CODE_GENERATION_GUARD({
                        asmjit::InvokeNode* dereference_func_invoke;
                        cc.invoke(&dereference_func_invoke, _ext_dereference_object, asmjit::FuncSignatureT<void, size_t, size_t>(platform_call_convention));
                        dereference_func_invoke->setArg(0, out_type);
                        dereference_func_invoke->setArg(1, out_data);
                    })

                    cc.bind(dereference_unneeded);

                    REBAR_CC_DEBUG("Pop value.");

                    ctx.pop_side(a_side);

                    REBAR_CC_DEBUG("Assign value.");

                    cc.mov(asmjit::x86::qword_ptr(ctx.identifier), out_type);
                    cc.mov(asmjit::x86::qword_ptr(ctx.identifier, object_data_offset), out_data);
                }

                break;
            }
            case separator::addition: {
                perform_node_pass(ctx, a_expression.get_operand(0), a_side);

                bool push_required = true;

                if (flags_set(compiler_flag::optimize_bypass_register_save)) {
                    perform_node_pass(ctx, a_expression.get_operand(1), !a_side);
                    //push_required = test_push_require_flags & side_clobber_flag(a_side);
                }

                if (push_required) {
                    ctx.push_side(a_side);
                }

                perform_node_pass(ctx, a_expression.get_operand(1), !a_side);

                if (push_required) {
                    ctx.pop_side(a_side);
                }

                cc.mov(asmjit::x86::qword_ptr(ctx.return_object), out_type);
                cc.mov(asmjit::x86::qword_ptr(ctx.return_object, object_data_offset), out_data);

                pass.set_flags(pass_flag::clobber_return);

                REBAR_CODE_GENERATION_GUARD({
                    asmjit::InvokeNode* add_invoke;
                    cc.invoke(&add_invoke, _ext_object_add, asmjit::FuncSignatureT<void, environment*, object*, type, size_t>(platform_call_convention));
                    add_invoke->setArg(0, ctx.environment);
                    add_invoke->setArg(1, ctx.return_object);
                    add_invoke->setArg(2, opp_out_type);
                    add_invoke->setArg(3, opp_out_data);
                })

                cc.mov(out_type, asmjit::x86::qword_ptr(ctx.return_object));
                cc.mov(out_data, asmjit::x86::qword_ptr(ctx.return_object, object_data_offset));

                break;
            }
            case separator::addition_assignment: {
                REBAR_CC_DEBUG("Addition.");

                perform_assignable_node_pass(ctx, a_expression.get_operand(0));

                bool push_required = true;

                //if (flags_set(compiler_flag::optimize_bypass_register_save)) {
                //    auto test_push_required_flags = perform_node_pass(a_ctx, a_expression.get_operand(1), !a_side, a_flags | pass_flag::void_code_generation);
                //    push_required = test_push_required_flags & pass_flag::clobber_identifier;
                //}

                if (push_required) {
                    ctx.push_identifier();
                }

                perform_node_pass(ctx, a_expression.get_operand(1), !a_side);

                if (push_required) {
                    ctx.pop_identifier();
                }

                // TODO: OPTIMIZE: Only call for complex types.

                cc.movdqa(ctx.transfer, asmjit::x86::dqword_ptr(ctx.identifier));
                cc.movdqa(asmjit::x86::dqword_ptr(ctx.return_object), ctx.transfer);

                pass.set_flags(pass_flag::clobber_transfer);

                REBAR_CODE_GENERATION_GUARD({
                    asmjit::InvokeNode* add_invoke;
                    cc.invoke(&add_invoke, _ext_object_add, asmjit::FuncSignatureT<void, environment*, object*, type, size_t>(platform_call_convention));
                    add_invoke->setArg(0, ctx.environment);
                    add_invoke->setArg(1, ctx.return_object);
                    add_invoke->setArg(2, opp_out_type);
                    add_invoke->setArg(3, opp_out_data);
                })

                cc.movdqa(ctx.transfer, asmjit::x86::dqword_ptr(ctx.return_object));
                cc.movdqa(asmjit::x86::dqword_ptr(ctx.identifier), ctx.transfer);

                break;
            }
            case separator::multiplication:
                break;
            case separator::multiplication_assignment:
                break;
            case separator::division:
                break;
            case separator::division_assignment:
                break;
            case separator::subtraction:
                break;
            case separator::subtraction_assignment:
                break;
            case separator::increment:
                break;
            case separator::decrement:
                break;
            case separator::group_open:
                break;
            case separator::group_close:
                break;
            case separator::selector_open:
                break;
            case separator::selector_close:
                break;
            case separator::scope_open:
                break;
            case separator::scope_close:
                break;
            case separator::equality: {
                REBAR_CC_DEBUG("Performing operation. (EQUALITY)");

                bool push_required = true;

                //if (flags_set(compiler_flag::optimize_bypass_register_save)) {
                //    auto test_push_required_flags = perform_node_pass(a_ctx, a_expression.get_operand(1), output_side::righthand, a_flags | pass_flag::void_code_generation);
                //    push_required = test_push_required_flags & pass_flag::clobber_left;
                //}

                perform_node_pass(ctx, a_expression.get_operand(0), output_side::lefthand);

                if (push_required) {
                    ctx.push_side(output_side::lefthand);
                }

                perform_node_pass(ctx, a_expression.get_operand(1), output_side::righthand);

                if (push_required) {
                    ctx.pop_side(output_side::lefthand);
                }

                const auto& label_end = cc.newLabel();
                const auto& label_bad_compare = cc.newLabel();
                const auto& label_complex_compare = cc.newLabel();

                REBAR_CC_DEBUG("Comparing types.");

                cc.cmp(ctx.lhs_type, ctx.rhs_type);
                cc.jne(label_bad_compare);

                REBAR_CC_DEBUG("Check simple comparison.");

                cc.cmp(ctx.lhs_type, object::simply_comparable_end_boundary);
                cc.jg(label_complex_compare);

                REBAR_CC_DEBUG("Simple comparison.");

                cc.cmp(ctx.lhs_data, ctx.rhs_data);
                cc.jne(label_bad_compare);

                cc.mov(out_type, type::boolean);
                cc.mov(out_data, true);
                cc.jmp(label_end);

                REBAR_CC_DEBUG("Complex comparison.");

                cc.bind(label_complex_compare);

                // TODO: Implement complex compare.

                REBAR_CC_DEBUG("Bad comparison.");

                cc.bind(label_bad_compare);

                cc.mov(out_type, type::boolean);
                cc.mov(out_data, false);

                cc.bind(label_end);

                REBAR_CC_DEBUG("End of operation. (EQUALITY)");

                break;
            }
            case separator::inverse_equality:
                break;
            case separator::greater:
                break;
            case separator::lesser:
                break;
            case separator::greater_equality:
                break;
            case separator::lesser_equality:
                break;
            case separator::logical_or:
                break;
            case separator::logical_and:
                break;
            case separator::logical_not:
                break;
            case separator::bitwise_or:
                break;
            case separator::bitwise_or_assignment:
                break;
            case separator::bitwise_xor:
                break;
            case separator::bitwise_xor_assignment:
                break;
            case separator::bitwise_and:
                break;
            case separator::bitwise_and_assignment:
                break;
            case separator::bitwise_not:
                break;
            case separator::shift_right:
                break;
            case separator::shift_right_assignment:
                break;
            case separator::shift_left:
                break;
            case separator::shift_left_assignment:
                break;
            case separator::exponent:
                break;
            case separator::exponent_assignment:
                break;
            case separator::modulus:
                break;
            case separator::modulus_assignment:
                break;
            //case separator::seek:
            case separator::ternary:
                break;
            //case separator::list:
            case separator::length:
                break;
            case separator::ellipsis:
                break;
            case separator::end_statement:
                break;
            case separator::new_object: {
                const auto& callable_node = a_expression.get_operand(0);

                auto argument_memory = ctx.mark_argument_allocation();
                auto argument_allocation(argument_memory);
                size_t argument_offset = 0;

                for (auto it = a_expression.get_operands().begin() + 1; it != a_expression.get_operands().cend(); ++it) {
                    REBAR_CC_DEBUG("Assign function argument. (%d)", argument_offset);

                    perform_node_pass(ctx, *it, a_side);

                    cc.mov(argument_allocation, out_type);
                    argument_allocation.addOffset(object_data_offset);
                    cc.mov(argument_allocation, out_data);
                    argument_allocation.addOffset(object_data_offset);

                    const auto& reference_needed = cc.newLabel();
                    const auto& reference_unneeded = cc.newLabel();

                    REBAR_CC_DEBUG("Test argument referencing.");

                    // Test if object needs explicit referencing.
                    cc.cmp(out_type, object::simple_type_end_boundary);
                    cc.jg(reference_needed);
                    cc.jmp(reference_unneeded);

                    cc.bind(reference_needed);

                    REBAR_CC_DEBUG("Call reference.");

                    REBAR_CODE_GENERATION_GUARD({
                        asmjit::InvokeNode* reference_func_invoke;
                        cc.invoke(&reference_func_invoke, _ext_reference_object, asmjit::FuncSignatureT<void, size_t, size_t>(platform_call_convention));
                        reference_func_invoke->setArg(0, out_type);
                        reference_func_invoke->setArg(1, out_data);
                    })

                    cc.bind(reference_unneeded);

                    ++argument_offset;
                }

                REBAR_CC_DEBUG("Assign argument count. (%d)", a_expression.get_operands().size() - 1);

                perform_node_pass(ctx, a_expression.get_operand(0), a_side);

                cc.movabs(ctx.identifier, reinterpret_cast<size_t>(m_environment.get_arguments_size_pointer()));
                cc.mov(asmjit::x86::qword_ptr(ctx.identifier), a_expression.get_operands().size() - 1);

                auto [env_arg_pointer, arg_alloc] = ctx.expression_registers(!a_side);
                cc.mov(env_arg_pointer, reinterpret_cast<size_t>(m_environment.get_arguments_pointer_ref()));
                cc.lea(arg_alloc, argument_memory);
                cc.mov(asmjit::x86::qword_ptr(env_arg_pointer), arg_alloc);

                cc.mov(asmjit::x86::qword_ptr(ctx.return_object), out_type);
                cc.mov(asmjit::x86::qword_ptr(ctx.return_object, object_data_offset), out_data);

                pass.set_flags(pass_flag::clobber_return | pass_flag::clobber_identifier);

                REBAR_CODE_GENERATION_GUARD({
                    asmjit::InvokeNode* invoke_node;
                    cc.invoke(&invoke_node, _ext_object_new, asmjit::FuncSignatureT<void, object*, environment*>(platform_call_convention));
                    invoke_node->setArg(0, ctx.environment);
                    invoke_node->setArg(1, ctx.return_object);
                })

                argument_offset = 0;

                argument_allocation = argument_memory;

                for (auto it = a_expression.get_operands().begin() + 1; it != a_expression.get_operands().cend(); ++it) {
                    REBAR_CC_DEBUG("Test argument dereferencing. (%d)", argument_offset);

                    cc.mov(out_type, argument_allocation);
                    argument_allocation.addOffset(object_data_offset);
                    cc.mov(out_data, argument_allocation);
                    argument_allocation.addOffset(object_data_offset);

                    const auto& dereference_needed = cc.newLabel();
                    const auto& dereference_unneeded = cc.newLabel();

                    // Test if object needs explicit referencing.
                    cc.cmp(out_type, object::simple_type_end_boundary);
                    cc.jg(dereference_needed);
                    cc.jmp(dereference_unneeded);

                    cc.bind(dereference_needed);

                    REBAR_CC_DEBUG("Dereference function.");

                    REBAR_CODE_GENERATION_GUARD({
                        asmjit::InvokeNode* dereference_func_invoke;
                        cc.invoke(&dereference_func_invoke, _ext_dereference_object, asmjit::FuncSignatureT<void, size_t, size_t>(platform_call_convention));
                        dereference_func_invoke->setArg(0, out_type);
                        dereference_func_invoke->setArg(1, out_data);
                    })

                    cc.bind(dereference_unneeded);

                    ++argument_offset;
                }

                cc.mov(out_type, asmjit::x86::qword_ptr(ctx.return_object));
                cc.mov(out_data, asmjit::x86::qword_ptr(ctx.return_object, object_data_offset));

                break;
            }
            case separator::namespace_index:
            case separator::direct:
            case separator::dot:
                if (a_expression.count() == 2) {
                    REBAR_CC_DEBUG("Starting select operation.");

                    perform_node_pass(ctx, a_expression.get_operand(0), a_side);

                    REBAR_CC_DEBUG("Move LHS data into return object.");

                    cc.mov(asmjit::x86::qword_ptr(ctx.return_object), out_type);
                    cc.mov(asmjit::x86::qword_ptr(ctx.return_object, object_data_offset), out_data);

                    pass.target_flags(pass_flag::identifier_as_string);
                    perform_node_pass(ctx, a_expression.get_operand(1), a_side);

                    REBAR_CC_DEBUG("Call select function.");

                    REBAR_CODE_GENERATION_GUARD({
                        asmjit::InvokeNode* select_invoke;
                        cc.invoke(&select_invoke, _ext_object_select, asmjit::FuncSignatureT<void, environment*, object*, type, size_t>(platform_call_convention));
                        select_invoke->setArg(0, ctx.environment);
                        select_invoke->setArg(1, ctx.return_object);
                        select_invoke->setArg(2, out_type);
                        select_invoke->setArg(3, out_data);
                    })

                    REBAR_CC_DEBUG("Move return data into correct registers.");

                    cc.mov(out_type, asmjit::x86::qword_ptr(ctx.return_object));
                    cc.mov(out_data, asmjit::x86::qword_ptr(ctx.return_object, object_data_offset));

                    pass.set_flags(pass_flag::clobber_return | pass_flag::clobber_return);
                    break;
                }
            case separator::operation_index:
                if (a_expression.count() == 2) {
                    REBAR_CC_DEBUG("Starting select operation.");

                    perform_node_pass(ctx, a_expression.get_operand(0), a_side);

                    REBAR_CC_DEBUG("Move LHS data into return object.");

                    cc.mov(asmjit::x86::qword_ptr(ctx.return_object), out_type);
                    cc.mov(asmjit::x86::qword_ptr(ctx.return_object, object_data_offset), out_data);

                    perform_node_pass(ctx, a_expression.get_operand(1), a_side);

                    REBAR_CC_DEBUG("Call select function.");

                    REBAR_CODE_GENERATION_GUARD({
                        asmjit::InvokeNode* select_invoke;
                        cc.invoke(&select_invoke, _ext_object_select, asmjit::FuncSignatureT<void, environment*, object*, type, size_t>(platform_call_convention));
                        select_invoke->setArg(0, ctx.environment);
                        select_invoke->setArg(1, ctx.return_object);
                        select_invoke->setArg(2, out_type);
                        select_invoke->setArg(3, out_data);
                    })

                    REBAR_CC_DEBUG("Move return data into correct registers.");

                    cc.mov(out_type, asmjit::x86::qword_ptr(ctx.return_object));
                    cc.mov(out_data, asmjit::x86::qword_ptr(ctx.return_object, object_data_offset));

                    pass.set_flags(pass_flag::clobber_return | pass_flag::clobber_return);
                    break;
                }
            case separator::operation_prefix_increment:
                break;
            case separator::operation_postfix_increment:
                break;
            case separator::operation_prefix_decrement:
                break;
            case separator::operation_postfix_decrement:
                break;
            case separator::operation_call: {
                const auto& callable_node = a_expression.get_operand(0);

                auto argument_memory = ctx.mark_argument_allocation();
                auto argument_allocation(argument_memory);
                size_t argument_offset = 0;

                bool dot_call = false;

                if (callable_node.is_expression() || callable_node.is_group()) {
                    const auto& expr = callable_node.get_expression();

                    if (expr.get_operation() == separator::dot) {
                        dot_call = true;

                        perform_node_pass(ctx, expr.get_operand(0), a_side);

                        cc.mov(argument_allocation, out_type);
                        argument_allocation.addOffset(object_data_offset);
                        cc.mov(argument_allocation, out_data);
                        argument_allocation.addOffset(object_data_offset);

                        ++argument_offset;
                    }
                }

                for (auto it = a_expression.get_operands().begin() + 1; it != a_expression.get_operands().cend(); ++it) {
                    REBAR_CC_DEBUG("Assign function argument. (%d)", argument_offset);

                    perform_node_pass(ctx, *it, a_side);

                    cc.mov(argument_allocation, out_type);
                    argument_allocation.addOffset(object_data_offset);
                    cc.mov(argument_allocation, out_data);
                    argument_allocation.addOffset(object_data_offset);

                    const auto& reference_needed = cc.newLabel();
                    const auto& reference_unneeded = cc.newLabel();

                    REBAR_CC_DEBUG("Test argument referencing.");

                    // Test if object needs explicit referencing.
                    cc.cmp(out_type, object::simple_type_end_boundary);
                    cc.jg(reference_needed);
                    cc.jmp(reference_unneeded);

                    cc.bind(reference_needed);

                    REBAR_CC_DEBUG("Call reference.");

                    REBAR_CODE_GENERATION_GUARD({
                        asmjit::InvokeNode* reference_func_invoke;
                        cc.invoke(&reference_func_invoke, _ext_reference_object, asmjit::FuncSignatureT<void, size_t, size_t>(platform_call_convention));
                        reference_func_invoke->setArg(0, out_type);
                        reference_func_invoke->setArg(1, out_data);
                    })

                    cc.bind(reference_unneeded);

                    ++argument_offset;
                }

                if (dot_call) {
                    REBAR_CC_DEBUG("Starting select operation.");

                    REBAR_CC_DEBUG("Move LHS data into return object.");

                    cc.lea(ctx.identifier, argument_memory);
                    cc.mov(out_type, asmjit::x86::qword_ptr(ctx.identifier));
                    cc.mov(out_data, asmjit::x86::qword_ptr(ctx.identifier, object_data_offset));
                    cc.mov(asmjit::x86::qword_ptr(ctx.return_object), out_type);
                    cc.mov(asmjit::x86::qword_ptr(ctx.return_object, object_data_offset), out_data);

                    pass.set_flags(pass_flag::clobber_return);

                    pass.target_flags(pass_flag::identifier_as_string);
                    perform_node_pass(ctx, a_expression.get_operand(0).get_expression().get_operand(1), a_side);

                    REBAR_CC_DEBUG("Call select function.");

                    REBAR_CODE_GENERATION_GUARD({
                        asmjit::InvokeNode* select_invoke;
                        cc.invoke(&select_invoke, _ext_object_select, asmjit::FuncSignatureT<void, environment*, object*, type, size_t>(platform_call_convention));
                        select_invoke->setArg(0, ctx.environment);
                        select_invoke->setArg(1, ctx.return_object);
                        select_invoke->setArg(2, out_type);
                        select_invoke->setArg(3, out_data);
                    })

                    REBAR_CC_DEBUG("Move return data into correct registers.");

                    cc.mov(out_type, asmjit::x86::qword_ptr(ctx.return_object));
                    cc.mov(out_data, asmjit::x86::qword_ptr(ctx.return_object, object_data_offset));
                } else {
                    perform_node_pass(ctx, callable_node, a_side);
                }

                REBAR_CC_DEBUG("Assign argument data. (%d)", a_expression.get_operands().size() - 1);

                cc.movabs(ctx.identifier, reinterpret_cast<size_t>(m_environment.get_arguments_size_pointer()));
                cc.mov(asmjit::x86::qword_ptr(ctx.identifier), a_expression.get_operands().size() - 1 + dot_call);

                pass.set_flags(pass_flag::clobber_identifier | pass_flag::clobber_return);

                auto [env_arg_pointer, arg_alloc] = ctx.expression_registers(!a_side);
                cc.mov(env_arg_pointer, reinterpret_cast<size_t>(m_environment.get_arguments_pointer_ref()));
                cc.lea(arg_alloc, argument_memory);
                cc.mov(asmjit::x86::qword_ptr(env_arg_pointer), arg_alloc);

                REBAR_CODE_GENERATION_GUARD({
                    // TODO: Proper call.
                    asmjit::InvokeNode* invoke_node;
                    cc.invoke(&invoke_node, out_data, asmjit::FuncSignatureT<void, object*, environment*>(platform_call_convention));
                    invoke_node->setArg(0, ctx.return_object);
                    invoke_node->setArg(1, ctx.environment);
                })

                argument_offset = dot_call ? 1 : 0;

                argument_allocation = argument_memory;

                if (dot_call) {
                    argument_allocation.addOffset(sizeof(object));
                }

                for (auto it = a_expression.get_operands().begin() + 1; it != a_expression.get_operands().cend(); ++it) {
                    REBAR_CC_DEBUG("Test argument dereferencing. (%d)", argument_offset);

                    cc.mov(out_type, argument_allocation);
                    argument_allocation.addOffset(object_data_offset);
                    cc.mov(out_data, argument_allocation);
                    argument_allocation.addOffset(object_data_offset);

                    const auto& dereference_needed = cc.newLabel();
                    const auto& dereference_unneeded = cc.newLabel();

                    // Test if object needs explicit referencing.
                    cc.cmp(out_type, object::simple_type_end_boundary);
                    cc.jg(dereference_needed);
                    cc.jmp(dereference_unneeded);

                    cc.bind(dereference_needed);

                    REBAR_CC_DEBUG("Dereference function.");

                    REBAR_CODE_GENERATION_GUARD({
                        asmjit::InvokeNode *dereference_func_invoke;
                        cc.invoke(&dereference_func_invoke, _ext_dereference_object, asmjit::FuncSignatureT<void, size_t, size_t>(platform_call_convention));
                        dereference_func_invoke->setArg(0, out_type);
                        dereference_func_invoke->setArg(1, out_data);
                    })

                    cc.bind(dereference_unneeded);

                    ++argument_offset;
                }

                cc.mov(out_type, asmjit::x86::qword_ptr(ctx.return_object));
                cc.mov(out_data, asmjit::x86::qword_ptr(ctx.return_object, object_data_offset));

                ctx.unmark_argument_allocation();

                break;
            }
            default:
                break;
        }
    }

    void compiler::perform_assignable_expression_pass(function_context& ctx, const node::expression& a_expression) {
        function_context::pass_control pass(ctx);

        pass.set_flags(pass_flag::clobber_identifier);

        auto& cc = ctx.assembler;

        switch (a_expression.get_operation()) {
            case separator::space: {
                bool flag_local = false;
                bool flag_constant = false;

                if (a_expression.count() > 1) {
                    for (auto it = a_expression.get_operands().begin(); it != a_expression.get_operands().end() - 1; ++it) {
                        if (it->is_token()) {
                            const auto& tok = it->get_token();

                            if (tok.is_keyword()) {
                                const auto kw = tok.get_keyword();

                                if (kw == keyword::local) {
                                    flag_local = true;
                                } else if (kw == keyword::constant) {
                                    flag_constant = true;
                                }
                            }
                        }
                    }
                }

                if (pass.flags_set(pass_flag::local_identifier)) {
                    flag_local = true;
                }

                const auto assignee_op = *(a_expression.get_operands().end() - 1);

                if (assignee_op.is_token()) {
                    const auto& tok = assignee_op.get_token();

                    if (tok.is_identifier()) {
                        if (flag_local) {
                            auto& local_table = ctx.local_variable_list.back();

                            if (local_table.find(tok.get_identifier()) == local_table.cend()) {
                                ctx.local_variable_list.back().emplace(tok.get_identifier(), ctx.local_stack_position);

                                REBAR_CC_DEBUG("Initialize local to null. (%s)", tok.get_identifier().data());

                                asmjit::x86::Mem locals_stack(ctx.locals_stack);
                                locals_stack.addOffset(ctx.local_stack_position * sizeof(object));
                                cc.lea(ctx.identifier, locals_stack);
                                cc.mov(asmjit::x86::qword_ptr(ctx.identifier), type::null);
                                cc.mov(asmjit::x86::qword_ptr(ctx.identifier, object_data_offset), 0);

                                pass.set_flags(pass_flag::clobber_identifier);

                                ++ctx.local_stack_position;
                                ++ctx.block_local_offsets.back();
                            }
                        } else if (flag_constant) {
                            ctx.constant_tables.back().emplace(tok.get_identifier(), object{});
                        }

                        load_identifier_pointer(ctx, tok.get_identifier());
                    }
                }

                perform_assignable_node_pass(ctx, assignee_op);
                break;
            }
            case separator::addition_assignment: {

            }
            case separator::subtraction_assignment: {

            }
            case separator::multiplication_assignment: {

            }
            case separator::division_assignment: {

            }
            case separator::modulus_assignment: {

            }
            case separator::exponent_assignment: {

            }
            case separator::bitwise_or_assignment: {

            }
            case separator::bitwise_xor_assignment: {

            }
            case separator::bitwise_and_assignment: {

            }
            case separator::shift_right_assignment: {

            }
            case separator::shift_left_assignment: {

            }
            case separator::operation_prefix_increment: {

            }
            case separator::operation_prefix_decrement: {

            }
            case separator::namespace_index:
            case separator::direct:
            case separator::operation_index: {
                auto [out_type, out_data] = ctx.expression_registers(output_side::lefthand);

                REBAR_CC_DEBUG("Starting index operation.");

                perform_node_pass(ctx, a_expression.get_operand(0), output_side::lefthand);

                REBAR_CC_DEBUG("Move LHS data into return object.");

                cc.mov(asmjit::x86::qword_ptr(ctx.return_object), out_type);
                cc.mov(asmjit::x86::qword_ptr(ctx.return_object, object_data_offset), out_data);

                perform_node_pass(ctx, a_expression.get_operand(1), output_side::lefthand);

                REBAR_CC_DEBUG("Call index function.");

                REBAR_CODE_GENERATION_GUARD({
                    asmjit::InvokeNode *index_invoke;
                    cc.invoke(&index_invoke, _ext_object_index, asmjit::FuncSignatureT<object *, environment *, object *, type, size_t>(platform_call_convention));
                    index_invoke->setArg(0, ctx.environment);
                    index_invoke->setArg(1, ctx.return_object);
                    index_invoke->setArg(2, out_type);
                    index_invoke->setArg(3, out_data);
                    index_invoke->setRet(0, ctx.identifier);
                })

                pass.set_flags(pass_flag::clobber_identifier | pass_flag::clobber_return);

                break;
            }
            case separator::dot: {
                auto [out_type, out_data] = ctx.expression_registers(output_side::lefthand);

                REBAR_CC_DEBUG("Starting index operation.");

                perform_node_pass(ctx, a_expression.get_operand(0), output_side::lefthand);

                REBAR_CC_DEBUG("Move LHS data into return object.");

                cc.mov(asmjit::x86::qword_ptr(ctx.return_object), out_type);
                cc.mov(asmjit::x86::qword_ptr(ctx.return_object, object_data_offset), out_data);

                pass.target_flags(pass_flag::identifier_as_string);
                perform_node_pass(ctx, a_expression.get_operand(1), output_side::lefthand);

                REBAR_CC_DEBUG("Call index function.");

                REBAR_CODE_GENERATION_GUARD({
                    asmjit::InvokeNode *index_invoke;
                    cc.invoke(&index_invoke, _ext_object_index, asmjit::FuncSignatureT<object *, environment *, object *, type, size_t>(platform_call_convention));
                    index_invoke->setArg(0, ctx.environment);
                    index_invoke->setArg(1, ctx.return_object);
                    index_invoke->setArg(2, out_type);
                    index_invoke->setArg(3, out_data);
                    index_invoke->setRet(0, ctx.identifier);
                })

                pass.set_flags(pass_flag::clobber_identifier | pass_flag::clobber_return);

                break;
            }
            default:
                break;
                // TODO: Throw unassignable expression error.
        }
    }
}

#endif //REBAR_EXPRESSION_PASS_HPP
