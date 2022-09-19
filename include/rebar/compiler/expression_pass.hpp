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

        bool evaluate_constant = pass.flags_set(pass_flag::evaluate_constant_expression);
        auto& const_side = ctx.constant_side(a_side);

        switch (a_expression.get_operation()) {
            case separator::space:
                perform_node_pass(ctx, a_expression.get_operand(0), a_side);
                break;
            case separator::assignment: {
                pass.set_flags(pass_flag::dynamic_expression);

                REBAR_CC_DEBUG("Perform assignment.");

                ctx.target_flags(pass_flag::void_code_generation);
                perform_assignable_node_pass(ctx, a_expression.get_operand(0));
                auto pre_assignable_pass = ctx.output_flags();

                ctx.target_flags(pass_flag::void_code_generation | pass_flag::evaluate_constant_expression);
                perform_node_pass(ctx, a_expression.get_operand(1), a_side);
                auto pre_value_pass = ctx.output_flags();

                if (pre_assignable_pass & pass_flag::constant_assignable) {
                    if (pre_value_pass & pass_flag::dynamic_expression) {
                        // TODO: Throw exception, dynamic expression assigned to constant.
                        m_environment.cout() << "CONSTANT EXCEPTION" << std::endl;
                    } else {
                        *ctx.constant_identifier = ctx.constant_side(a_side);
                    }
                } else {
                    ctx.target_flags(pass_flag::check_local_definition);
                    perform_assignable_node_pass(ctx, a_expression.get_operand(0));
                    bool dereference_required = !ctx.out_flags_set(pass_flag::local_definition);
                    ctx.unset_out_flags(pass_flag::local_definition);

                    REBAR_CC_DEBUG("Dereference origin data if needed.");

                    const auto& dereference_needed = cc.newLabel();
                    const auto& dereference_unneeded = cc.newLabel();

                    if (dereference_required) {
                        // Test if object needs explicit referencing.
                        cc.cmp(asmjit::x86::qword_ptr(ctx.identifier), object::simple_type_end_boundary);
                        cc.jg(dereference_needed);
                        cc.jmp(dereference_unneeded);

                        cc.bind(dereference_needed);

                        REBAR_CC_DEBUG("Dereference function.");

                        REBAR_CODE_GENERATION_GUARD({
                            asmjit::InvokeNode* dereference_func_invoke;
                            cc.invoke(&dereference_func_invoke, _ext_dereference_object, asmjit::FuncSignatureT<void, object*>(platform_call_convention));
                            dereference_func_invoke->setArg(0, ctx.identifier);
                        })

                        cc.bind(dereference_unneeded);
                    }

                    bool push_required = pre_value_pass & pass_flag::clobber_identifier;

                    if (push_required) {
                        ctx.push_identifier();
                    }

                    perform_node_pass(ctx, a_expression.get_operand(1), a_side);

                    if (push_required) {
                        ctx.pop_identifier();
                    }

                    REBAR_CC_DEBUG("Assign value.");

                    cc.mov(asmjit::x86::qword_ptr(ctx.identifier), out_type);
                    cc.mov(asmjit::x86::qword_ptr(ctx.identifier, object_data_offset), out_data);

                    const auto& reference_needed = cc.newLabel();
                    const auto& reference_unneeded = cc.newLabel();

                    REBAR_CC_DEBUG("Test referencing.");

                    // Test if object needs explicit referencing.
                    cc.cmp(asmjit::x86::qword_ptr(ctx.identifier), object::simple_type_end_boundary);
                    cc.jg(reference_needed);
                    cc.jmp(reference_unneeded);

                    cc.bind(reference_needed);

                    REBAR_CC_DEBUG("Call reference.");

                    REBAR_CODE_GENERATION_GUARD({
                        asmjit::InvokeNode* reference_func_invoke;
                        cc.invoke(&reference_func_invoke, _ext_reference_object, asmjit::FuncSignatureT<void, object*>(platform_call_convention));
                        reference_func_invoke->setArg(0, ctx.identifier);
                    })

                    cc.bind(reference_unneeded);
                }

                break;
            }
            case separator::addition: {
                object constant_lhs;

                perform_node_pass(ctx, a_expression.get_operand(0), a_side);

                if (evaluate_constant) {
                    constant_lhs = ctx.constant_side(a_side);
                }

                bool push_required = true;

                if (flags_set(compiler_flag::optimize_bypass_register_save)) {
                    ctx.target_flags(pass_flag::void_code_generation);
                    perform_node_pass(ctx, a_expression.get_operand(1), !a_side);
                    push_required = ctx.out_flags_set(side_clobber_flag(a_side));
                }

                if (push_required) {
                    ctx.push_side(a_side);
                }

                perform_node_pass(ctx, a_expression.get_operand(1), !a_side);

                if (evaluate_constant) {
                    ctx.constant_side(a_side) = object::add(m_environment, constant_lhs, ctx.constant_side(!a_side));
                }

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
                REBAR_CC_DEBUG("Addition assignment.");

                perform_assignable_node_pass(ctx, a_expression.get_operand(0));

                bool push_required = true;

                if (flags_set(compiler_flag::optimize_bypass_register_save)) {
                    ctx.target_flags(pass_flag::void_code_generation);
                    perform_node_pass(ctx, a_expression.get_operand(1), !a_side);
                    push_required = ctx.out_flags_set(pass_flag::clobber_identifier);
                }

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
            case separator::multiplication: {
                object constant_lhs;

                perform_node_pass(ctx, a_expression.get_operand(0), a_side);

                if (evaluate_constant) {
                    constant_lhs = ctx.constant_side(a_side);
                }

                bool push_required = true;

                if (flags_set(compiler_flag::optimize_bypass_register_save)) {
                    ctx.target_flags(pass_flag::void_code_generation);
                    perform_node_pass(ctx, a_expression.get_operand(1), !a_side);
                    push_required = ctx.out_flags_set(side_clobber_flag(a_side));
                }

                if (push_required) {
                    ctx.push_side(a_side);
                }

                perform_node_pass(ctx, a_expression.get_operand(1), !a_side);

                if (evaluate_constant) {
                    ctx.constant_side(a_side) = object::multiply(m_environment, constant_lhs, ctx.constant_side(!a_side));
                }

                if (push_required) {
                    ctx.pop_side(a_side);
                }

                cc.mov(asmjit::x86::qword_ptr(ctx.return_object), out_type);
                cc.mov(asmjit::x86::qword_ptr(ctx.return_object, object_data_offset), out_data);

                pass.set_flags(pass_flag::clobber_return);

                REBAR_CODE_GENERATION_GUARD({
                    asmjit::InvokeNode* multiply_invoke;
                    cc.invoke(&multiply_invoke, _ext_object_multiply, asmjit::FuncSignatureT<void, environment*, object*, type, size_t>(platform_call_convention));
                    multiply_invoke->setArg(0, ctx.environment);
                    multiply_invoke->setArg(1, ctx.return_object);
                    multiply_invoke->setArg(2, opp_out_type);
                    multiply_invoke->setArg(3, opp_out_data);
                })

                cc.mov(out_type, asmjit::x86::qword_ptr(ctx.return_object));
                cc.mov(out_data, asmjit::x86::qword_ptr(ctx.return_object, object_data_offset));

                break;
            }
            case separator::multiplication_assignment: {
                REBAR_CC_DEBUG("Multiplication assignment.");

                perform_assignable_node_pass(ctx, a_expression.get_operand(0));

                bool push_required = true;

                if (flags_set(compiler_flag::optimize_bypass_register_save)) {
                    ctx.target_flags(pass_flag::void_code_generation);
                    perform_node_pass(ctx, a_expression.get_operand(1), !a_side);
                    push_required = ctx.out_flags_set(pass_flag::clobber_identifier);
                }

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
                    asmjit::InvokeNode* multiply_invoke;
                    cc.invoke(&multiply_invoke, _ext_object_multiply, asmjit::FuncSignatureT<void, environment*, object*, type, size_t>(platform_call_convention));
                    multiply_invoke->setArg(0, ctx.environment);
                    multiply_invoke->setArg(1, ctx.return_object);
                    multiply_invoke->setArg(2, opp_out_type);
                    multiply_invoke->setArg(3, opp_out_data);
                })

                cc.movdqa(ctx.transfer, asmjit::x86::dqword_ptr(ctx.return_object));
                cc.movdqa(asmjit::x86::dqword_ptr(ctx.identifier), ctx.transfer);

                break;
            }
            case separator::division: {
                object constant_lhs;

                perform_node_pass(ctx, a_expression.get_operand(0), a_side);

                if (evaluate_constant) {
                    constant_lhs = ctx.constant_side(a_side);
                }

                bool push_required = true;

                if (flags_set(compiler_flag::optimize_bypass_register_save)) {
                    ctx.target_flags(pass_flag::void_code_generation);
                    perform_node_pass(ctx, a_expression.get_operand(1), !a_side);
                    push_required = ctx.out_flags_set(side_clobber_flag(a_side));
                }

                if (push_required) {
                    ctx.push_side(a_side);
                }

                perform_node_pass(ctx, a_expression.get_operand(1), !a_side);

                if (evaluate_constant) {
                    ctx.constant_side(a_side) = object::divide(m_environment, constant_lhs, ctx.constant_side(!a_side));
                }

                if (push_required) {
                    ctx.pop_side(a_side);
                }

                cc.mov(asmjit::x86::qword_ptr(ctx.return_object), out_type);
                cc.mov(asmjit::x86::qword_ptr(ctx.return_object, object_data_offset), out_data);

                pass.set_flags(pass_flag::clobber_return);

                REBAR_CODE_GENERATION_GUARD({
                    asmjit::InvokeNode* divide_invoke;
                    cc.invoke(&divide_invoke, _ext_object_divide, asmjit::FuncSignatureT<void, environment*, object*, type, size_t>(platform_call_convention));
                    divide_invoke->setArg(0, ctx.environment);
                    divide_invoke->setArg(1, ctx.return_object);
                    divide_invoke->setArg(2, opp_out_type);
                    divide_invoke->setArg(3, opp_out_data);
                })

                cc.mov(out_type, asmjit::x86::qword_ptr(ctx.return_object));
                cc.mov(out_data, asmjit::x86::qword_ptr(ctx.return_object, object_data_offset));

                break;
            }
            case separator::division_assignment: {
                REBAR_CC_DEBUG("Division assignment.");

                perform_assignable_node_pass(ctx, a_expression.get_operand(0));

                bool push_required = true;

                if (flags_set(compiler_flag::optimize_bypass_register_save)) {
                    ctx.target_flags(pass_flag::void_code_generation);
                    perform_node_pass(ctx, a_expression.get_operand(1), !a_side);
                    push_required = ctx.out_flags_set(pass_flag::clobber_identifier);
                }

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
                    asmjit::InvokeNode* divide_invoke;
                    cc.invoke(&divide_invoke, _ext_object_divide, asmjit::FuncSignatureT<void, environment*, object*, type, size_t>(platform_call_convention));
                    divide_invoke->setArg(0, ctx.environment);
                    divide_invoke->setArg(1, ctx.return_object);
                    divide_invoke->setArg(2, opp_out_type);
                    divide_invoke->setArg(3, opp_out_data);
                })

                cc.movdqa(ctx.transfer, asmjit::x86::dqword_ptr(ctx.return_object));
                cc.movdqa(asmjit::x86::dqword_ptr(ctx.identifier), ctx.transfer);

                break;
            }
            case separator::subtraction: {
                object constant_lhs;

                perform_node_pass(ctx, a_expression.get_operand(0), a_side);

                if (evaluate_constant) {
                    constant_lhs = ctx.constant_side(a_side);
                }

                bool push_required = true;

                if (flags_set(compiler_flag::optimize_bypass_register_save)) {
                    ctx.target_flags(pass_flag::void_code_generation);
                    perform_node_pass(ctx, a_expression.get_operand(1), !a_side);
                    push_required = ctx.out_flags_set(side_clobber_flag(a_side));
                }

                if (push_required) {
                    ctx.push_side(a_side);
                }

                perform_node_pass(ctx, a_expression.get_operand(1), !a_side);

                if (evaluate_constant) {
                    ctx.constant_side(a_side) = object::subtract(m_environment, constant_lhs, ctx.constant_side(!a_side));
                }

                if (push_required) {
                    ctx.pop_side(a_side);
                }

                cc.mov(asmjit::x86::qword_ptr(ctx.return_object), out_type);
                cc.mov(asmjit::x86::qword_ptr(ctx.return_object, object_data_offset), out_data);

                pass.set_flags(pass_flag::clobber_return);

                REBAR_CODE_GENERATION_GUARD({
                    asmjit::InvokeNode* subtract_invoke;
                    cc.invoke(&subtract_invoke, _ext_object_subtract, asmjit::FuncSignatureT<void, environment*, object*, type, size_t>(platform_call_convention));
                    subtract_invoke->setArg(0, ctx.environment);
                    subtract_invoke->setArg(1, ctx.return_object);
                    subtract_invoke->setArg(2, opp_out_type);
                    subtract_invoke->setArg(3, opp_out_data);
                })

                cc.mov(out_type, asmjit::x86::qword_ptr(ctx.return_object));
                cc.mov(out_data, asmjit::x86::qword_ptr(ctx.return_object, object_data_offset));

                break;
            }
            case separator::subtraction_assignment: {
                REBAR_CC_DEBUG("Subtraction assignment.");

                perform_assignable_node_pass(ctx, a_expression.get_operand(0));

                bool push_required = true;

                if (flags_set(compiler_flag::optimize_bypass_register_save)) {
                    ctx.target_flags(pass_flag::void_code_generation);
                    perform_node_pass(ctx, a_expression.get_operand(1), !a_side);
                    push_required = ctx.out_flags_set(pass_flag::clobber_identifier);
                }

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
                    asmjit::InvokeNode* subtract_invoke;
                    cc.invoke(&subtract_invoke, _ext_object_subtract, asmjit::FuncSignatureT<void, environment*, object*, type, size_t>(platform_call_convention));
                    subtract_invoke->setArg(0, ctx.environment);
                    subtract_invoke->setArg(1, ctx.return_object);
                    subtract_invoke->setArg(2, opp_out_type);
                    subtract_invoke->setArg(3, opp_out_data);
                })

                cc.movdqa(ctx.transfer, asmjit::x86::dqword_ptr(ctx.return_object));
                cc.movdqa(asmjit::x86::dqword_ptr(ctx.identifier), ctx.transfer);

                break;
            }
            case separator::equality: {
                REBAR_CC_DEBUG("Performing operation. (EQUALITY)");

                perform_node_pass(ctx, a_expression.get_operand(0), output_side::lefthand);

                bool push_required = true;

                if (flags_set(compiler_flag::optimize_bypass_register_save)) {
                    ctx.target_flags(pass_flag::void_code_generation);
                    perform_node_pass(ctx, a_expression.get_operand(1), output_side::righthand);
                    push_required = ctx.out_flags_set(pass_flag::clobber_left);
                }

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

                // TODO: Bypass pre-checks if LHS is native object.

                cc.cmp(ctx.lhs_type, ctx.rhs_type);
                cc.jne(label_bad_compare);

                REBAR_CC_DEBUG("Check simple comparison.");

                cc.cmp(ctx.lhs_type, object::simply_comparable_end_boundary);
                cc.jg(label_complex_compare);

                cc.mov(out_type, type::boolean);     // Load type and true data.
                cc.xor_(opp_out_type, opp_out_type); // Load type and true data.
                cc.cmp(ctx.lhs_data, ctx.rhs_data);  // Compare data values.
                cc.cmove(out_data, out_type);        // Load true if equal.
                cc.cmovne(out_data, opp_out_type);   // Load true if equal.
                cc.jmp(label_end);

                REBAR_CC_DEBUG("Complex comparison.");

                cc.bind(label_complex_compare);

                cc.mov(asmjit::x86::qword_ptr(ctx.return_object), ctx.lhs_type);
                cc.mov(asmjit::x86::qword_ptr(ctx.return_object, object_data_offset), ctx.lhs_data);

                REBAR_CODE_GENERATION_GUARD({
                    asmjit::InvokeNode* invoke;
                    cc.invoke(&invoke, _ext_object_equals, asmjit::FuncSignatureT<void, environment*, object*, type, size_t>(platform_call_convention));
                    invoke->setArg(0, ctx.environment);
                    invoke->setArg(1, ctx.return_object);
                    invoke->setArg(2, ctx.rhs_type);
                    invoke->setArg(3, ctx.rhs_data);
                })

                cc.mov(out_type, asmjit::x86::qword_ptr(ctx.return_object));
                cc.mov(out_data, asmjit::x86::qword_ptr(ctx.return_object, object_data_offset));

                cc.jmp(label_end);

                REBAR_CC_DEBUG("Simple comparison.");

                cc.bind(label_bad_compare);

                cc.mov(out_type, type::boolean);
                cc.xor_(out_data, out_data);

                cc.bind(label_end);

                REBAR_CC_DEBUG("End of operation. (EQUALITY)");

                break;
            }
            case separator::inverse_equality: {
                REBAR_CC_DEBUG("Performing operation. (INVERSE EQUALITY)");

                perform_node_pass(ctx, a_expression.get_operand(0), output_side::lefthand);

                bool push_required = true;

                if (flags_set(compiler_flag::optimize_bypass_register_save)) {
                    ctx.target_flags(pass_flag::void_code_generation);
                    perform_node_pass(ctx, a_expression.get_operand(1), output_side::righthand);
                    push_required = ctx.out_flags_set(pass_flag::clobber_left);
                }

                if (push_required) {
                    ctx.push_side(output_side::lefthand);
                }

                perform_node_pass(ctx, a_expression.get_operand(1), output_side::righthand);

                if (push_required) {
                    ctx.pop_side(output_side::lefthand);
                }

                const auto& label_end = cc.newLabel();
                const auto& label_good_compare = cc.newLabel();
                const auto& label_complex_compare = cc.newLabel();

                REBAR_CC_DEBUG("Comparing types.");

                // TODO: Bypass pre-checks if LHS is native object.

                cc.cmp(ctx.lhs_type, ctx.rhs_type);
                cc.jne(label_good_compare);

                REBAR_CC_DEBUG("Check simple comparison.");

                cc.cmp(ctx.lhs_type, object::simply_comparable_end_boundary);
                cc.jg(label_complex_compare);

                cc.mov(out_type, type::boolean);     // Load type and true data.
                cc.xor_(opp_out_type, opp_out_type); // Load type and true data.
                cc.cmp(ctx.lhs_data, ctx.rhs_data);  // Compare data values.
                cc.cmove(out_data, opp_out_type);    // Load false if equal.
                cc.cmovne(out_data, out_type);       // Load true if not equal.
                cc.jmp(label_end);

                REBAR_CC_DEBUG("Complex comparison.");

                cc.bind(label_complex_compare);

                cc.mov(asmjit::x86::qword_ptr(ctx.return_object), ctx.lhs_type);
                cc.mov(asmjit::x86::qword_ptr(ctx.return_object, object_data_offset), ctx.lhs_data);

                REBAR_CODE_GENERATION_GUARD({
                    asmjit::InvokeNode* invoke;
                    cc.invoke(&invoke, _ext_object_not_equals, asmjit::FuncSignatureT<void, environment*, object*, type, size_t>(platform_call_convention));
                    invoke->setArg(0, ctx.environment);
                    invoke->setArg(1, ctx.return_object);
                    invoke->setArg(2, ctx.rhs_type);
                    invoke->setArg(3, ctx.rhs_data);
                })

                cc.mov(out_type, asmjit::x86::qword_ptr(ctx.return_object));
                cc.mov(out_data, asmjit::x86::qword_ptr(ctx.return_object, object_data_offset));

                cc.jmp(label_end);

                REBAR_CC_DEBUG("Simple comparison.");

                cc.bind(label_good_compare);

                cc.mov(out_type, type::boolean);
                cc.mov(out_data, true);

                cc.bind(label_end);

                REBAR_CC_DEBUG("End of operation. (INVERSE EQUALITY)");

                break;
            }
            case separator::greater:
                break;
            case separator::lesser:
                break;
            case separator::greater_equality:
                break;
            case separator::lesser_equality:
                break;
            case separator::logical_or: {
                perform_node_pass(ctx, a_expression.get_operand(0), a_side);

                const auto& label_end = cc.newLabel();

                cc.cmp(out_data, 0);
                cc.jne(label_end);

                if (ctx.constant_side(a_side)) {
                    ctx.unset_target_flags(pass_flag::evaluate_constant_expression);
                }

                perform_node_pass(ctx, a_expression.get_operand(1), a_side);

                cc.bind(label_end);

                break;
            }
            case separator::logical_and: {
                perform_node_pass(ctx, a_expression.get_operand(0), a_side);

                const auto& label_end = cc.newLabel();

                cc.cmp(out_data, 0);
                cc.je(label_end);

                if (!ctx.constant_side(a_side)) {
                    ctx.unset_target_flags(pass_flag::evaluate_constant_expression);
                }

                perform_node_pass(ctx, a_expression.get_operand(1), a_side);

                cc.bind(label_end);

                break;
            }
            case separator::logical_not: {
                perform_node_pass(ctx, a_expression.get_operand(0), a_side);

                if (evaluate_constant) {
                    const_side = !const_side;
                }

                cc.mov(out_type, type::boolean);
                cc.not_(out_data);

                break;
            }
            case separator::bitwise_or: {
                object constant_lhs;

                perform_node_pass(ctx, a_expression.get_operand(0), a_side);

                if (evaluate_constant) {
                    constant_lhs = ctx.constant_side(a_side);
                }

                bool push_required = true;

                if (flags_set(compiler_flag::optimize_bypass_register_save)) {
                    ctx.target_flags(pass_flag::void_code_generation);
                    perform_node_pass(ctx, a_expression.get_operand(1), !a_side);
                    push_required = ctx.out_flags_set(side_clobber_flag(a_side));
                }

                if (push_required) {
                    ctx.push_side(a_side);
                }

                perform_node_pass(ctx, a_expression.get_operand(1), !a_side);

                if (evaluate_constant) {
                    ctx.constant_side(a_side) = object::bitwise_or(m_environment, constant_lhs, ctx.constant_side(!a_side));
                }

                if (push_required) {
                    ctx.pop_side(a_side);
                }

                cc.mov(asmjit::x86::qword_ptr(ctx.return_object), out_type);
                cc.mov(asmjit::x86::qword_ptr(ctx.return_object, object_data_offset), out_data);

                pass.set_flags(pass_flag::clobber_return);

                REBAR_CODE_GENERATION_GUARD({
                    asmjit::InvokeNode* bitwise_invoke;
                    cc.invoke(&bitwise_invoke, _ext_object_bitwise_or, asmjit::FuncSignatureT<void, environment*, object*, type, size_t>(platform_call_convention));
                    bitwise_invoke->setArg(0, ctx.environment);
                    bitwise_invoke->setArg(1, ctx.return_object);
                    bitwise_invoke->setArg(2, opp_out_type);
                    bitwise_invoke->setArg(3, opp_out_data);
                })

                cc.mov(out_type, asmjit::x86::qword_ptr(ctx.return_object));
                cc.mov(out_data, asmjit::x86::qword_ptr(ctx.return_object, object_data_offset));

                break;
            }
            case separator::bitwise_or_assignment: {
                REBAR_CC_DEBUG("Bitwise OR assignment.");

                perform_assignable_node_pass(ctx, a_expression.get_operand(0));

                bool push_required = true;

                if (flags_set(compiler_flag::optimize_bypass_register_save)) {
                    ctx.target_flags(pass_flag::void_code_generation);
                    perform_node_pass(ctx, a_expression.get_operand(1), !a_side);
                    push_required = ctx.out_flags_set(pass_flag::clobber_identifier);
                }

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
                    asmjit::InvokeNode* bitwise_invoke;
                    cc.invoke(&bitwise_invoke, _ext_object_bitwise_or, asmjit::FuncSignatureT<void, environment*, object*, type, size_t>(platform_call_convention));
                    bitwise_invoke->setArg(0, ctx.environment);
                    bitwise_invoke->setArg(1, ctx.return_object);
                    bitwise_invoke->setArg(2, opp_out_type);
                    bitwise_invoke->setArg(3, opp_out_data);
                })

                cc.movdqa(ctx.transfer, asmjit::x86::dqword_ptr(ctx.return_object));
                cc.movdqa(asmjit::x86::dqword_ptr(ctx.identifier), ctx.transfer);

                break;
            }
            case separator::bitwise_xor: {
                object constant_lhs;

                perform_node_pass(ctx, a_expression.get_operand(0), a_side);

                if (evaluate_constant) {
                    constant_lhs = ctx.constant_side(a_side);
                }

                bool push_required = true;

                if (flags_set(compiler_flag::optimize_bypass_register_save)) {
                    ctx.target_flags(pass_flag::void_code_generation);
                    perform_node_pass(ctx, a_expression.get_operand(1), !a_side);
                    push_required = ctx.out_flags_set(side_clobber_flag(a_side));
                }

                if (push_required) {
                    ctx.push_side(a_side);
                }

                perform_node_pass(ctx, a_expression.get_operand(1), !a_side);

                if (evaluate_constant) {
                    ctx.constant_side(a_side) = object::bitwise_xor(m_environment, constant_lhs, ctx.constant_side(!a_side));
                }

                if (push_required) {
                    ctx.pop_side(a_side);
                }

                cc.mov(asmjit::x86::qword_ptr(ctx.return_object), out_type);
                cc.mov(asmjit::x86::qword_ptr(ctx.return_object, object_data_offset), out_data);

                pass.set_flags(pass_flag::clobber_return);

                REBAR_CODE_GENERATION_GUARD({
                    asmjit::InvokeNode* bitwise_invoke;
                    cc.invoke(&bitwise_invoke, _ext_object_bitwise_xor, asmjit::FuncSignatureT<void, environment*, object*, type, size_t>(platform_call_convention));
                    bitwise_invoke->setArg(0, ctx.environment);
                    bitwise_invoke->setArg(1, ctx.return_object);
                    bitwise_invoke->setArg(2, opp_out_type);
                    bitwise_invoke->setArg(3, opp_out_data);
                })

                cc.mov(out_type, asmjit::x86::qword_ptr(ctx.return_object));
                cc.mov(out_data, asmjit::x86::qword_ptr(ctx.return_object, object_data_offset));

                break;
            }
            case separator::bitwise_xor_assignment: {
                REBAR_CC_DEBUG("Bitwise XOR assignment.");

                perform_assignable_node_pass(ctx, a_expression.get_operand(0));

                bool push_required = true;

                if (flags_set(compiler_flag::optimize_bypass_register_save)) {
                    ctx.target_flags(pass_flag::void_code_generation);
                    perform_node_pass(ctx, a_expression.get_operand(1), !a_side);
                    push_required = ctx.out_flags_set(pass_flag::clobber_identifier);
                }

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
                    asmjit::InvokeNode* bitwise_invoke;
                    cc.invoke(&bitwise_invoke, _ext_object_bitwise_xor, asmjit::FuncSignatureT<void, environment*, object*, type, size_t>(platform_call_convention));
                    bitwise_invoke->setArg(0, ctx.environment);
                    bitwise_invoke->setArg(1, ctx.return_object);
                    bitwise_invoke->setArg(2, opp_out_type);
                    bitwise_invoke->setArg(3, opp_out_data);
                })

                cc.movdqa(ctx.transfer, asmjit::x86::dqword_ptr(ctx.return_object));
                cc.movdqa(asmjit::x86::dqword_ptr(ctx.identifier), ctx.transfer);

                break;
            }
            case separator::bitwise_and: {
                object constant_lhs;

                perform_node_pass(ctx, a_expression.get_operand(0), a_side);

                if (evaluate_constant) {
                    constant_lhs = ctx.constant_side(a_side);
                }

                bool push_required = true;

                if (flags_set(compiler_flag::optimize_bypass_register_save)) {
                    ctx.target_flags(pass_flag::void_code_generation);
                    perform_node_pass(ctx, a_expression.get_operand(1), !a_side);
                    push_required = ctx.out_flags_set(side_clobber_flag(a_side));
                }

                if (push_required) {
                    ctx.push_side(a_side);
                }

                perform_node_pass(ctx, a_expression.get_operand(1), !a_side);

                if (evaluate_constant) {
                    ctx.constant_side(a_side) = object::bitwise_and(m_environment, constant_lhs, ctx.constant_side(!a_side));
                }

                if (push_required) {
                    ctx.pop_side(a_side);
                }

                cc.mov(asmjit::x86::qword_ptr(ctx.return_object), out_type);
                cc.mov(asmjit::x86::qword_ptr(ctx.return_object, object_data_offset), out_data);

                pass.set_flags(pass_flag::clobber_return);

                REBAR_CODE_GENERATION_GUARD({
                    asmjit::InvokeNode* bitwise_invoke;
                    cc.invoke(&bitwise_invoke, _ext_object_bitwise_and, asmjit::FuncSignatureT<void, environment*, object*, type, size_t>(platform_call_convention));
                    bitwise_invoke->setArg(0, ctx.environment);
                    bitwise_invoke->setArg(1, ctx.return_object);
                    bitwise_invoke->setArg(2, opp_out_type);
                    bitwise_invoke->setArg(3, opp_out_data);
                })

                cc.mov(out_type, asmjit::x86::qword_ptr(ctx.return_object));
                cc.mov(out_data, asmjit::x86::qword_ptr(ctx.return_object, object_data_offset));

                break;
            }
            case separator::bitwise_and_assignment: {
                REBAR_CC_DEBUG("Bitwise AND assignment.");

                perform_assignable_node_pass(ctx, a_expression.get_operand(0));

                bool push_required = true;

                if (flags_set(compiler_flag::optimize_bypass_register_save)) {
                    ctx.target_flags(pass_flag::void_code_generation);
                    perform_node_pass(ctx, a_expression.get_operand(1), !a_side);
                    push_required = ctx.out_flags_set(pass_flag::clobber_identifier);
                }

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
                    asmjit::InvokeNode* bitwise_invoke;
                    cc.invoke(&bitwise_invoke, _ext_object_bitwise_and, asmjit::FuncSignatureT<void, environment*, object*, type, size_t>(platform_call_convention));
                    bitwise_invoke->setArg(0, ctx.environment);
                    bitwise_invoke->setArg(1, ctx.return_object);
                    bitwise_invoke->setArg(2, opp_out_type);
                    bitwise_invoke->setArg(3, opp_out_data);
                })

                cc.movdqa(ctx.transfer, asmjit::x86::dqword_ptr(ctx.return_object));
                cc.movdqa(asmjit::x86::dqword_ptr(ctx.identifier), ctx.transfer);

                break;
            }
            case separator::bitwise_not:
                perform_node_pass(ctx, a_expression.get_operand(0), a_side);

                cc.mov(asmjit::x86::qword_ptr(ctx.return_object), out_type);
                cc.mov(asmjit::x86::qword_ptr(ctx.return_object, object_data_offset), out_data);

                REBAR_CODE_GENERATION_GUARD({
                    asmjit::InvokeNode* invoke;
                    cc.invoke(&invoke, _ext_object_bitwise_not, asmjit::FuncSignatureT<void, environment*, object*>(platform_call_convention));
                    invoke->setArg(0, ctx.environment);
                    invoke->setArg(1, ctx.return_object);
                })

                cc.mov(out_type, asmjit::x86::qword_ptr(ctx.return_object));
                cc.mov(out_data, asmjit::x86::qword_ptr(ctx.return_object, object_data_offset));

                break;
            case separator::shift_right: {
                object constant_lhs;

                perform_node_pass(ctx, a_expression.get_operand(0), a_side);

                if (evaluate_constant) {
                    constant_lhs = ctx.constant_side(a_side);
                }

                bool push_required = true;

                if (flags_set(compiler_flag::optimize_bypass_register_save)) {
                    ctx.target_flags(pass_flag::void_code_generation);
                    perform_node_pass(ctx, a_expression.get_operand(1), !a_side);
                    push_required = ctx.out_flags_set(side_clobber_flag(a_side));
                }

                if (push_required) {
                    ctx.push_side(a_side);
                }

                perform_node_pass(ctx, a_expression.get_operand(1), !a_side);

                if (evaluate_constant) {
                    ctx.constant_side(a_side) = object::shift_right(m_environment, constant_lhs, ctx.constant_side(!a_side));
                }

                if (push_required) {
                    ctx.pop_side(a_side);
                }

                cc.mov(asmjit::x86::qword_ptr(ctx.return_object), out_type);
                cc.mov(asmjit::x86::qword_ptr(ctx.return_object, object_data_offset), out_data);

                pass.set_flags(pass_flag::clobber_return);

                REBAR_CODE_GENERATION_GUARD({
                    asmjit::InvokeNode* shift_invoke;
                    cc.invoke(&shift_invoke, _ext_object_shift_right, asmjit::FuncSignatureT<void, environment*, object*, type, size_t>(platform_call_convention));
                    shift_invoke->setArg(0, ctx.environment);
                    shift_invoke->setArg(1, ctx.return_object);
                    shift_invoke->setArg(2, opp_out_type);
                    shift_invoke->setArg(3, opp_out_data);
                })

                cc.mov(out_type, asmjit::x86::qword_ptr(ctx.return_object));
                cc.mov(out_data, asmjit::x86::qword_ptr(ctx.return_object, object_data_offset));

                break;
            }
            case separator::shift_right_assignment: {
                REBAR_CC_DEBUG("Shift right assignment.");

                perform_assignable_node_pass(ctx, a_expression.get_operand(0));

                bool push_required = true;

                if (flags_set(compiler_flag::optimize_bypass_register_save)) {
                    ctx.target_flags(pass_flag::void_code_generation);
                    perform_node_pass(ctx, a_expression.get_operand(1), !a_side);
                    push_required = ctx.out_flags_set(pass_flag::clobber_identifier);
                }

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
                    asmjit::InvokeNode* shift_invoke;
                    cc.invoke(&shift_invoke, _ext_object_shift_right, asmjit::FuncSignatureT<void, environment*, object*, type, size_t>(platform_call_convention));
                    shift_invoke->setArg(0, ctx.environment);
                    shift_invoke->setArg(1, ctx.return_object);
                    shift_invoke->setArg(2, opp_out_type);
                    shift_invoke->setArg(3, opp_out_data);
                })

                cc.movdqa(ctx.transfer, asmjit::x86::dqword_ptr(ctx.return_object));
                cc.movdqa(asmjit::x86::dqword_ptr(ctx.identifier), ctx.transfer);

                break;
            }
            case separator::shift_left: {
                object constant_lhs;

                perform_node_pass(ctx, a_expression.get_operand(0), a_side);

                if (evaluate_constant) {
                    constant_lhs = ctx.constant_side(a_side);
                }

                bool push_required = true;

                if (flags_set(compiler_flag::optimize_bypass_register_save)) {
                    ctx.target_flags(pass_flag::void_code_generation);
                    perform_node_pass(ctx, a_expression.get_operand(1), !a_side);
                    push_required = ctx.out_flags_set(side_clobber_flag(a_side));
                }

                if (push_required) {
                    ctx.push_side(a_side);
                }

                perform_node_pass(ctx, a_expression.get_operand(1), !a_side);

                if (evaluate_constant) {
                    ctx.constant_side(a_side) = object::shift_left(m_environment, constant_lhs, ctx.constant_side(!a_side));
                }

                if (push_required) {
                    ctx.pop_side(a_side);
                }

                cc.mov(asmjit::x86::qword_ptr(ctx.return_object), out_type);
                cc.mov(asmjit::x86::qword_ptr(ctx.return_object, object_data_offset), out_data);

                pass.set_flags(pass_flag::clobber_return);

                REBAR_CODE_GENERATION_GUARD({
                    asmjit::InvokeNode* shift_invoke;
                    cc.invoke(&shift_invoke, _ext_object_shift_left, asmjit::FuncSignatureT<void, environment*, object*, type, size_t>(platform_call_convention));
                    shift_invoke->setArg(0, ctx.environment);
                    shift_invoke->setArg(1, ctx.return_object);
                    shift_invoke->setArg(2, opp_out_type);
                    shift_invoke->setArg(3, opp_out_data);
                })

                cc.mov(out_type, asmjit::x86::qword_ptr(ctx.return_object));
                cc.mov(out_data, asmjit::x86::qword_ptr(ctx.return_object, object_data_offset));

                break;
            }
            case separator::shift_left_assignment: {
                REBAR_CC_DEBUG("Shift left assignment.");

                perform_assignable_node_pass(ctx, a_expression.get_operand(0));

                bool push_required = true;

                if (flags_set(compiler_flag::optimize_bypass_register_save)) {
                    ctx.target_flags(pass_flag::void_code_generation);
                    perform_node_pass(ctx, a_expression.get_operand(1), !a_side);
                    push_required = ctx.out_flags_set(pass_flag::clobber_identifier);
                }

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
                    asmjit::InvokeNode* shift_invoke;
                    cc.invoke(&shift_invoke, _ext_object_shift_left, asmjit::FuncSignatureT<void, environment*, object*, type, size_t>(platform_call_convention));
                    shift_invoke->setArg(0, ctx.environment);
                    shift_invoke->setArg(1, ctx.return_object);
                    shift_invoke->setArg(2, opp_out_type);
                    shift_invoke->setArg(3, opp_out_data);
                })

                cc.movdqa(ctx.transfer, asmjit::x86::dqword_ptr(ctx.return_object));
                cc.movdqa(asmjit::x86::dqword_ptr(ctx.identifier), ctx.transfer);

                break;
            }
            case separator::exponent: {
                object constant_lhs;

                perform_node_pass(ctx, a_expression.get_operand(0), a_side);

                if (evaluate_constant) {
                    constant_lhs = ctx.constant_side(a_side);
                }

                bool push_required = true;

                if (flags_set(compiler_flag::optimize_bypass_register_save)) {
                    ctx.target_flags(pass_flag::void_code_generation);
                    perform_node_pass(ctx, a_expression.get_operand(1), !a_side);
                    push_required = ctx.out_flags_set(side_clobber_flag(a_side));
                }

                if (push_required) {
                    ctx.push_side(a_side);
                }

                perform_node_pass(ctx, a_expression.get_operand(1), !a_side);

                if (evaluate_constant) {
                    ctx.constant_side(a_side) = object::exponentiate(m_environment, constant_lhs, ctx.constant_side(!a_side));
                }

                if (push_required) {
                    ctx.pop_side(a_side);
                }

                cc.mov(asmjit::x86::qword_ptr(ctx.return_object), out_type);
                cc.mov(asmjit::x86::qword_ptr(ctx.return_object, object_data_offset), out_data);

                pass.set_flags(pass_flag::clobber_return);

                REBAR_CODE_GENERATION_GUARD({
                    asmjit::InvokeNode* exponent_invoke;
                    cc.invoke(&exponent_invoke, _ext_object_exponentiate, asmjit::FuncSignatureT<void, environment*, object*, type, size_t>(platform_call_convention));
                    exponent_invoke->setArg(0, ctx.environment);
                    exponent_invoke->setArg(1, ctx.return_object);
                    exponent_invoke->setArg(2, opp_out_type);
                    exponent_invoke->setArg(3, opp_out_data);
                })

                cc.mov(out_type, asmjit::x86::qword_ptr(ctx.return_object));
                cc.mov(out_data, asmjit::x86::qword_ptr(ctx.return_object, object_data_offset));

                break;
            }
            case separator::exponent_assignment: {
                REBAR_CC_DEBUG("Exponent assignment.");

                perform_assignable_node_pass(ctx, a_expression.get_operand(0));

                bool push_required = true;

                if (flags_set(compiler_flag::optimize_bypass_register_save)) {
                    ctx.target_flags(pass_flag::void_code_generation);
                    perform_node_pass(ctx, a_expression.get_operand(1), !a_side);
                    push_required = ctx.out_flags_set(pass_flag::clobber_identifier);
                }

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
                    asmjit::InvokeNode* exponent_invoke;
                    cc.invoke(&exponent_invoke, _ext_object_exponentiate, asmjit::FuncSignatureT<void, environment*, object*, type, size_t>(platform_call_convention));
                    exponent_invoke->setArg(0, ctx.environment);
                    exponent_invoke->setArg(1, ctx.return_object);
                    exponent_invoke->setArg(2, opp_out_type);
                    exponent_invoke->setArg(3, opp_out_data);
                })

                cc.movdqa(ctx.transfer, asmjit::x86::dqword_ptr(ctx.return_object));
                cc.movdqa(asmjit::x86::dqword_ptr(ctx.identifier), ctx.transfer);

                break;
            }
            case separator::modulus: {
                object constant_lhs;

                perform_node_pass(ctx, a_expression.get_operand(0), a_side);

                if (evaluate_constant) {
                    constant_lhs = ctx.constant_side(a_side);
                }

                bool push_required = true;

                if (flags_set(compiler_flag::optimize_bypass_register_save)) {
                    ctx.target_flags(pass_flag::void_code_generation);
                    perform_node_pass(ctx, a_expression.get_operand(1), !a_side);
                    push_required = ctx.out_flags_set(side_clobber_flag(a_side));
                }

                if (push_required) {
                    ctx.push_side(a_side);
                }

                perform_node_pass(ctx, a_expression.get_operand(1), !a_side);

                if (evaluate_constant) {
                    ctx.constant_side(a_side) = object::modulus(m_environment, constant_lhs, ctx.constant_side(!a_side));
                }

                if (push_required) {
                    ctx.pop_side(a_side);
                }

                cc.mov(asmjit::x86::qword_ptr(ctx.return_object), out_type);
                cc.mov(asmjit::x86::qword_ptr(ctx.return_object, object_data_offset), out_data);

                pass.set_flags(pass_flag::clobber_return);

                REBAR_CODE_GENERATION_GUARD({
                    asmjit::InvokeNode* modulus_invoke;
                    cc.invoke(&modulus_invoke, _ext_object_modulus, asmjit::FuncSignatureT<void, environment*, object*, type, size_t>(platform_call_convention));
                    modulus_invoke->setArg(0, ctx.environment);
                    modulus_invoke->setArg(1, ctx.return_object);
                    modulus_invoke->setArg(2, opp_out_type);
                    modulus_invoke->setArg(3, opp_out_data);
                })

                cc.mov(out_type, asmjit::x86::qword_ptr(ctx.return_object));
                cc.mov(out_data, asmjit::x86::qword_ptr(ctx.return_object, object_data_offset));

                break;
            }
            case separator::modulus_assignment: {
                REBAR_CC_DEBUG("Modulus assignment.");

                perform_assignable_node_pass(ctx, a_expression.get_operand(0));

                bool push_required = true;

                if (flags_set(compiler_flag::optimize_bypass_register_save)) {
                    ctx.target_flags(pass_flag::void_code_generation);
                    perform_node_pass(ctx, a_expression.get_operand(1), !a_side);
                    push_required = ctx.out_flags_set(pass_flag::clobber_identifier);
                }

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
                    asmjit::InvokeNode* modulus_invoke;
                    cc.invoke(&modulus_invoke, _ext_object_modulus, asmjit::FuncSignatureT<void, environment*, object*, type, size_t>(platform_call_convention));
                    modulus_invoke->setArg(0, ctx.environment);
                    modulus_invoke->setArg(1, ctx.return_object);
                    modulus_invoke->setArg(2, opp_out_type);
                    modulus_invoke->setArg(3, opp_out_data);
                })

                cc.movdqa(ctx.transfer, asmjit::x86::dqword_ptr(ctx.return_object));
                cc.movdqa(asmjit::x86::dqword_ptr(ctx.identifier), ctx.transfer);

                break;
            }
            case separator::ternary: {
                perform_node_pass(ctx, a_expression.get_operand(0), a_side);

                const auto& label_second = cc.newLabel();
                const auto& label_end = cc.newLabel();

                cc.cmp(out_data, 0);
                cc.je(label_second);

                perform_node_pass(ctx, a_expression.get_operand(1), a_side);

                cc.jmp(label_end);

                cc.bind(label_second);

                perform_node_pass(ctx, a_expression.get_operand(2), a_side);

                cc.bind(label_end);

                break;
            }
            case separator::length:
                perform_node_pass(ctx, a_expression.get_operand(0), a_side);

                cc.mov(asmjit::x86::qword_ptr(ctx.return_object), out_type);
                cc.mov(asmjit::x86::qword_ptr(ctx.return_object, object_data_offset), out_data);

                REBAR_CODE_GENERATION_GUARD({
                    asmjit::InvokeNode* length_invoke;
                    cc.invoke(&length_invoke, _ext_object_length, asmjit::FuncSignatureT<void, environment*, object*>(platform_call_convention));
                    length_invoke->setArg(0, ctx.environment);
                    length_invoke->setArg(1, ctx.return_object);
                })

                cc.mov(out_type, asmjit::x86::qword_ptr(ctx.return_object));
                cc.mov(out_data, asmjit::x86::qword_ptr(ctx.return_object, object_data_offset));

                break;
            case separator::new_object: {
                const auto& callable_node = a_expression.get_operand(0);

                auto argument_memory = ctx.mark_argument_allocation();
                auto argument_allocation(argument_memory);
                size_t argument_offset = 0;

                for (auto it = a_expression.get_operands().begin() + 1; it != a_expression.get_operands().cend(); ++it) {
                    REBAR_CC_DEBUG("Assign function argument. (%d)", argument_offset);

                    perform_node_pass(ctx, *it, a_side);

                    asmjit::x86::Mem argument(argument_allocation);

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

                    cc.lea(ctx.identifier, argument);

                    REBAR_CODE_GENERATION_GUARD({
                        asmjit::InvokeNode* reference_func_invoke;
                        cc.invoke(&reference_func_invoke, _ext_reference_object, asmjit::FuncSignatureT<void, object*>(platform_call_convention));
                        reference_func_invoke->setArg(0, ctx.identifier);
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

                    asmjit::x86::Mem argument(argument_allocation);

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

                    cc.lea(ctx.identifier, argument);

                    REBAR_CODE_GENERATION_GUARD({
                        asmjit::InvokeNode* dereference_func_invoke;
                        cc.invoke(&dereference_func_invoke, _ext_dereference_object, asmjit::FuncSignatureT<void, object*>(platform_call_convention));
                        dereference_func_invoke->setArg(0, ctx.identifier);
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

                    ctx.target_flags(pass_flag::identifier_as_string);
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
                perform_assignable_node_pass(ctx, a_expression.get_operand(0));

                REBAR_CODE_GENERATION_GUARD({
                    asmjit::InvokeNode* invoke;
                    cc.invoke(&invoke, _ext_object_prefix_increment, asmjit::FuncSignatureT<void, environment*, object*>(platform_call_convention));
                    invoke->setArg(0, ctx.environment);
                    invoke->setArg(1, ctx.identifier);
                })

                cc.mov(out_type, asmjit::x86::qword_ptr(ctx.identifier));
                cc.mov(out_data, asmjit::x86::qword_ptr(ctx.identifier, object_data_offset));

                break;
            case separator::operation_postfix_increment:
                perform_assignable_node_pass(ctx, a_expression.get_operand(0));

                cc.pxor(ctx.transfer, ctx.transfer);
                cc.movdqa(asmjit::x86::dqword_ptr(ctx.return_object), ctx.transfer);

                REBAR_CODE_GENERATION_GUARD({
                    asmjit::InvokeNode* invoke;
                    cc.invoke(&invoke, _ext_object_postfix_increment, asmjit::FuncSignatureT<void, environment*, object*, object*>(platform_call_convention));
                    invoke->setArg(0, ctx.environment);
                    invoke->setArg(1, ctx.return_object);
                    invoke->setArg(2, ctx.identifier);
                })

                cc.mov(out_type, asmjit::x86::qword_ptr(ctx.return_object));
                cc.mov(out_data, asmjit::x86::qword_ptr(ctx.return_object, object_data_offset));

                break;
            case separator::operation_prefix_decrement:
                perform_assignable_node_pass(ctx, a_expression.get_operand(0));

                REBAR_CODE_GENERATION_GUARD({
                    asmjit::InvokeNode* invoke;
                    cc.invoke(&invoke, _ext_object_prefix_decrement, asmjit::FuncSignatureT<void, environment*, object*>(platform_call_convention));
                    invoke->setArg(0, ctx.environment);
                    invoke->setArg(1, ctx.identifier);
                })

                cc.mov(out_type, asmjit::x86::qword_ptr(ctx.identifier));
                cc.mov(out_data, asmjit::x86::qword_ptr(ctx.identifier, object_data_offset));

                break;
            case separator::operation_postfix_decrement:
                perform_assignable_node_pass(ctx, a_expression.get_operand(0));

                cc.pxor(ctx.transfer, ctx.transfer);
                cc.movdqa(asmjit::x86::dqword_ptr(ctx.return_object), ctx.transfer);

                REBAR_CODE_GENERATION_GUARD({
                    asmjit::InvokeNode* invoke;
                    cc.invoke(&invoke, _ext_object_postfix_decrement, asmjit::FuncSignatureT<void, environment*, object*, object*>(platform_call_convention));
                    invoke->setArg(0, ctx.environment);
                    invoke->setArg(1, ctx.return_object);
                    invoke->setArg(2, ctx.identifier);
                })

                cc.mov(out_type, asmjit::x86::qword_ptr(ctx.return_object));
                cc.mov(out_data, asmjit::x86::qword_ptr(ctx.return_object, object_data_offset));

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

                    asmjit::x86::Mem argument(argument_allocation);

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

                    cc.lea(ctx.identifier, argument);

                    REBAR_CODE_GENERATION_GUARD({
                        asmjit::InvokeNode* reference_func_invoke;
                        cc.invoke(&reference_func_invoke, _ext_reference_object, asmjit::FuncSignatureT<void, object*>(platform_call_convention));
                        reference_func_invoke->setArg(0, ctx.identifier);
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

                    ctx.target_flags(pass_flag::identifier_as_string);
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

                    asmjit::x86::Mem argument(argument_allocation);

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

                    cc.lea(ctx.identifier, argument);

                    REBAR_CODE_GENERATION_GUARD({
                        asmjit::InvokeNode *dereference_func_invoke;
                        cc.invoke(&dereference_func_invoke, _ext_dereference_object, asmjit::FuncSignatureT<void, object*>(platform_call_convention));
                        dereference_func_invoke->setArg(0, ctx.identifier);
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

                            if (!pass.flags_set(pass_flag::void_code_generation) && local_table.find(tok.get_identifier()) == local_table.cend()) {
                                local_table.emplace(tok.get_identifier(), ctx.local_stack_position);

                                REBAR_CC_DEBUG("Initialize local to null. (%s)", tok.get_identifier().data());

                                asmjit::x86::Mem locals_stack(ctx.locals_stack);
                                locals_stack.addOffset(ctx.local_stack_position * sizeof(object));
                                cc.lea(ctx.identifier, locals_stack);
                                cc.mov(asmjit::x86::qword_ptr(ctx.identifier), type::null);
                                cc.mov(asmjit::x86::qword_ptr(ctx.identifier, object_data_offset), 0);

                                pass.set_flags(pass_flag::clobber_identifier);

                                if (pass.flags_set(pass_flag::check_local_definition)) {
                                    pass.set_flags(pass_flag::local_definition);
                                }

                                ++ctx.local_stack_position;
                                ++ctx.block_local_offsets.back();
                            }
                        } else if (flag_constant) {
                            ctx.constant_tables.back().emplace(tok.get_identifier(), object{});
                        }

                        load_identifier_pointer(ctx, tok.get_identifier());
                    }
                } else {
                    perform_assignable_node_pass(ctx, assignee_op);
                }

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

                ctx.target_flags(pass_flag::identifier_as_string);
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
