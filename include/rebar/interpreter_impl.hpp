//
// Created by maxng on 6/22/2022.
//

#ifndef REBAR_INTERPRETER_IMPL_HPP
#define REBAR_INTERPRETER_IMPL_HPP

#include "interpreter.hpp"

#include "environment.hpp"

namespace rebar {
    object interpreter::interpreted_function_source::internal_call() {
        enum class node_tags : enum_base {
            none,
            identifier_as_string
        };

        std::vector<table> local_tables;

        const auto find_variable = [this, &local_tables](const object a_key) -> object& {
            for (size_t i = local_tables.size(); i >= 1; --i) {
                auto found = local_tables[i - 1].find(a_key);

                if (found != local_tables[i - 1].cend()) {
                    return found->second;
                }
            }

            return m_environment.global_table()[a_key];
        };

        std::function<object (const node::expression&)> evaluate_expression;
        std::function<object (const node&, const node_tags)> detail_resolve_node;

        std::function<object (const node&)> resolve_node = [this, &evaluate_expression, &find_variable, &resolve_node, &detail_resolve_node](const node& a_node, const node_tags a_nodes = node_tags::none) -> object {
            if (a_node.is_token()) {
                const token& tok = a_node.get_token();

                switch (tok.token_type()) {
                    case token::type::identifier:
                        return find_variable(m_environment.str(tok.get_identifier()));
                        break;
                    case token::type::string_literal:
                        return m_environment.str(tok.get_string_literal());
                        break;
                    case token::type::integer_literal:
                        return tok.get_integer_literal();
                        break;
                    case token::type::number_literal:
                        return tok.get_number_literal();
                        break;
                    case token::type::keyword: {
                        switch (tok.get_keyword()) {
                            case keyword::literal_true:
                                return object{ true };
                            case keyword::literal_false:
                                return object{ false };
                            case keyword::literal_null:
                            default:
                                return null;
                        }
                    }
                    default:
                        break;
                }
            } else if (a_node.is_group() || a_node.is_expression()) {
                return evaluate_expression(a_node.get_expression());
            } else if (a_node.is_immediate_table()) {
                const auto& immediate = a_node.get_immediate_table();

                auto* tbl = new table;

                for (const auto& entry : immediate.m_entries) {
                    (*tbl)[detail_resolve_node(entry.first, node_tags::identifier_as_string)] = evaluate_expression(entry.second);
                }

                return tbl;
            } else if (a_node.is_selector()) {
                const auto& sel = a_node.get_selector();

                array arr(1); // Size = 1.
                arr.push_back(evaluate_expression(sel));

                return arr;
            } else if (a_node.is_immediate_array()) {
                const auto& immediate = a_node.get_immediate_array();

                array arr(immediate.size());

                for (const auto& n : immediate) {
                    arr.push_back(resolve_node(n));
                }

                return arr;
            }

            return null;
        };

        detail_resolve_node = [this, &resolve_node](const node& a_node, const node_tags a_tags) -> object {
            switch (a_tags) {
                case node_tags::identifier_as_string:
                    if (a_node.is_token()) {
                        const token &tok = a_node.get_token();

                        if (tok.is_identifier()) {
                            return m_environment.str(tok.get_identifier());
                        }
                    }
                default:
                    return resolve_node(a_node);
            }
        };

        std::function<object& (const node&)> resolve_assignable;

        std::function<object& (const node::expression&)> resolve_assignable_expression = [this, &local_tables, &resolve_assignable, &resolve_node, &detail_resolve_node](const node::expression& a_expression) -> object& {
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

                    const auto assignee_op = *(a_expression.get_operands().end() - 1);

                    if (flag_local) {
                        if (assignee_op.is_token()) {
                            const auto& tok = assignee_op.get_token();

                            if (tok.is_identifier()) {
                                auto& tb = local_tables.back();

                                return tb[m_environment.str(tok.get_identifier())];
                            }
                        }
                    }

                    return resolve_assignable(assignee_op);
                }
                case separator::addition_assignment: {
                    object& assignee = resolve_assignable(a_expression.get_operand(0));

                    if (assignee.is_native_object()) {
                        assignee.get_native_object().overload_addition_assignment(m_environment, resolve_node(a_expression.get_operand(1)));
                    } else {
                        assignee = object::add(m_environment, assignee, resolve_node(a_expression.get_operand(1)));
                    }

                    return assignee;
                }
                case separator::subtraction_assignment: {
                    object& assignee = resolve_assignable(a_expression.get_operand(0));

                    if (assignee.is_native_object()) {
                        assignee.get_native_object().overload_subtraction_assignment(m_environment, resolve_node(a_expression.get_operand(1)));
                    } else {
                        assignee = object::subtract(m_environment, assignee, resolve_node(a_expression.get_operand(1)));
                    }

                    return assignee;
                }
                case separator::multiplication_assignment: {
                    object& assignee = resolve_assignable(a_expression.get_operand(0));

                    if (assignee.is_native_object()) {
                        assignee.get_native_object().overload_multiplication_assignment(m_environment, resolve_node(a_expression.get_operand(1)));
                    } else {
                        assignee = object::multiply(m_environment, assignee, resolve_node(a_expression.get_operand(1)));
                    }

                    return assignee;
                }
                case separator::division_assignment: {
                    object& assignee = resolve_assignable(a_expression.get_operand(0));

                    if (assignee.is_native_object()) {
                        assignee.get_native_object().overload_division_assignment(m_environment, resolve_node(a_expression.get_operand(1)));
                    } else {
                        assignee = object::divide(m_environment, assignee, resolve_node(a_expression.get_operand(1)));
                    }

                    return assignee;
                }
                case separator::modulus_assignment: {
                    object& assignee = resolve_assignable(a_expression.get_operand(0));

                    if (assignee.is_native_object()) {
                        assignee.get_native_object().overload_modulus_assignment(m_environment, resolve_node(a_expression.get_operand(1)));
                    } else {
                        assignee = assignee % resolve_node(a_expression.get_operand(1));
                    }

                    return assignee;
                }
                case separator::exponent_assignment: {
                    object& assignee = resolve_assignable(a_expression.get_operand(0));

                    if (assignee.is_native_object()) {
                        assignee.get_native_object().overload_exponent_assignment(m_environment, resolve_node(a_expression.get_operand(1)));
                    } else {
                        assignee = object::exponentiate(m_environment, assignee, resolve_node(a_expression.get_operand(1)));
                    }

                    return assignee;
                }
                case separator::bitwise_or_assignment: {
                    object& assignee = resolve_assignable(a_expression.get_operand(0));

                    if (assignee.is_native_object()) {
                        assignee.get_native_object().overload_bitwise_or_assignment(m_environment, resolve_node(a_expression.get_operand(1)));
                    } else {
                        assignee = object::bitwise_or(m_environment, assignee, resolve_node(a_expression.get_operand(1)));
                    }

                    return assignee;
                }
                case separator::bitwise_xor_assignment: {
                    object& assignee = resolve_assignable(a_expression.get_operand(0));

                    if (assignee.is_native_object()) {
                        assignee.get_native_object().overload_bitwise_xor_assignment(m_environment, resolve_node(a_expression.get_operand(1)));
                    } else {
                        assignee = object::bitwise_xor(m_environment, assignee, resolve_node(a_expression.get_operand(1)));
                    }

                    return assignee;
                }
                case separator::bitwise_and_assignment: {
                    object& assignee = resolve_assignable(a_expression.get_operand(0));

                    if (assignee.is_native_object()) {
                        assignee.get_native_object().overload_bitwise_and_assignment(m_environment, resolve_node(a_expression.get_operand(1)));
                    } else {
                        assignee = object::bitwise_and(m_environment, assignee, resolve_node(a_expression.get_operand(1)));
                    }

                    return assignee;
                }
                case separator::shift_right_assignment: {
                    object& assignee = resolve_assignable(a_expression.get_operand(0));

                    if (assignee.is_native_object()) {
                        assignee.get_native_object().overload_shift_right_assignment(m_environment, resolve_node(a_expression.get_operand(1)));
                    } else {
                        assignee = object::shift_right(m_environment, assignee, resolve_node(a_expression.get_operand(1)));
                    }

                    return assignee;
                }
                case separator::shift_left_assignment: {
                    object& assignee = resolve_assignable(a_expression.get_operand(0));

                    if (assignee.is_native_object()) {
                        assignee.get_native_object().overload_shift_left_assignment(m_environment, resolve_node(a_expression.get_operand(1)));
                    } else {
                        assignee = object::shift_left(m_environment, assignee, resolve_node(a_expression.get_operand(1)));
                    }

                    return assignee;
                }
                case separator::operation_prefix_increment: {
                    object& assignee = resolve_assignable(a_expression.get_operand(0));

                    if (assignee.is_native_object()) {
                        assignee.get_native_object().overload_prefix_increment(m_environment);
                    } else {
                        assignee = object::add(m_environment, assignee, 1);
                    }

                    return assignee;
                }
                case separator::operation_prefix_decrement: {
                    object& assignee = resolve_assignable(a_expression.get_operand(0));

                    if (assignee.is_native_object()) {
                        assignee.get_native_object().overload_prefix_decrement(m_environment);
                    } else {
                        assignee = object::subtract(m_environment, assignee, 1);
                    }

                    return assignee;
                }
                case separator::namespace_index:
                case separator::direct:
                case separator::dot:
                    if (a_expression.count() == 2) {
                        return resolve_node(a_expression.get_operand(0)).index(m_environment, detail_resolve_node(a_expression.get_operand(1), node_tags::identifier_as_string));
                    }
                case separator::operation_index:
                    if (a_expression.count() == 2) {
                        return resolve_node(a_expression.get_operand(0)).index(m_environment, resolve_node(a_expression.get_operand(1)));
                    }
                default:
                    return null;
                    // TODO: Throw unassignable expression error.
            }

            // TODO: Throw unassignable error.
            return null;
        };

        resolve_assignable = [this, &resolve_assignable_expression, &find_variable](const node& a_node) -> object& {
            if (a_node.is_token()) {
                const token& tok = a_node.get_token();

                if (tok.is_identifier()) {
                    return find_variable(m_environment.str(tok.get_identifier()));
                }
            } else if (a_node.is_group() || a_node.is_selector() || a_node.is_expression()) {
                return resolve_assignable_expression(a_node.get_expression());
            }

            return null;
        };

        evaluate_expression = [this, &resolve_node, &resolve_assignable, &detail_resolve_node](const node::expression& a_expression) -> object {
            if (a_expression.empty()) {
                return null;
            }

            switch (a_expression.get_operation()) {
                case separator::space:
                    return resolve_node(a_expression.get_operand(0));
                case separator::assignment:
                    return resolve_assignable(a_expression.get_operand(0)) = resolve_node(a_expression.get_operand(1));
                case separator::addition:
                    return object::add(
                            m_environment,
                            resolve_node(a_expression.get_operand(0)),
                            resolve_node(a_expression.get_operand(1))
                    );
                case separator::addition_assignment: {
                    object& assignee = resolve_assignable(a_expression.get_operand(0));

                    if (assignee.is_native_object()) {
                        assignee.get_native_object().overload_addition_assignment(m_environment, resolve_node(a_expression.get_operand(1)));
                    } else {
                        assignee = object::add(m_environment, assignee, resolve_node(a_expression.get_operand(1)));
                    }

                    return assignee;
                }
                case separator::multiplication:
                    return object::multiply(
                            m_environment,
                            resolve_node(a_expression.get_operand(0)),
                            resolve_node(a_expression.get_operand(1))
                    );
                case separator::multiplication_assignment: {
                    object& assignee = resolve_assignable(a_expression.get_operand(0));

                    if (assignee.is_native_object()) {
                        assignee.get_native_object().overload_multiplication_assignment(m_environment, resolve_node(a_expression.get_operand(1)));
                    } else {
                        assignee = object::multiply(m_environment, assignee, resolve_node(a_expression.get_operand(1)));
                    }

                    return assignee;
                }
                case separator::division:
                    return object::divide(
                            m_environment,
                            resolve_node(a_expression.get_operand(0)),
                            resolve_node(a_expression.get_operand(1))
                    );
                case separator::division_assignment: {
                    object& assignee = resolve_assignable(a_expression.get_operand(0));

                    if (assignee.is_native_object()) {
                        assignee.get_native_object().overload_division_assignment(m_environment, resolve_node(a_expression.get_operand(1)));
                    } else {
                        assignee = object::divide(m_environment, assignee, resolve_node(a_expression.get_operand(1)));
                    }

                    return assignee;
                }
                case separator::subtraction:
                    return object::subtract(
                            m_environment,
                            resolve_node(a_expression.get_operand(0)),
                            resolve_node(a_expression.get_operand(1))
                    );
                case separator::subtraction_assignment: {
                    object& assignee = resolve_assignable(a_expression.get_operand(0));

                    if (assignee.is_native_object()) {
                        assignee.get_native_object().overload_subtraction_assignment(m_environment, resolve_node(a_expression.get_operand(1)));
                    } else {
                        assignee = object::subtract(m_environment, assignee, resolve_node(a_expression.get_operand(1)));
                    }

                    return assignee;
                }
                    //case separator::increment:
                    //case separator::decrement:
                    //case separator::group_open:
                    //case separator::group_close:
                    //case separator::selector_open:
                    //case separator::selector_close:
                    //case separator::scope_open:
                    //case separator::scope_close:
                case separator::equality:
                    return object::equals(m_environment, resolve_node(a_expression.get_operand(0)), resolve_node(a_expression.get_operand(1)));
                case separator::inverse_equality:
                    return object::not_equals(m_environment, resolve_node(a_expression.get_operand(0)), resolve_node(a_expression.get_operand(1)));
                case separator::greater:
                    return object::greater_than(m_environment, resolve_node(a_expression.get_operand(0)), resolve_node(a_expression.get_operand(1)));
                case separator::lesser:
                    return object::lesser_than(m_environment, resolve_node(a_expression.get_operand(0)), resolve_node(a_expression.get_operand(1)));
                case separator::greater_equality:
                    return object::greater_than_equal_to(m_environment, resolve_node(a_expression.get_operand(0)), resolve_node(a_expression.get_operand(1)));
                case separator::lesser_equality:
                    return object::lesser_than_equal_to(m_environment, resolve_node(a_expression.get_operand(0)), resolve_node(a_expression.get_operand(1)));
                case separator::logical_or: {
                    auto lhs = resolve_node(a_expression.get_operand(0));

                    if (lhs.is_native_object()) {
                        return lhs.get_native_object().overload_logical_or(m_environment, resolve_node(a_expression.get_operand(1)));
                    } else {
                        return lhs ? lhs : resolve_node(a_expression.get_operand(1));
                    }
                }
                case separator::logical_and: {
                    auto lhs = resolve_node(a_expression.get_operand(0));

                    if (lhs.is_native_object()) {
                        return lhs.get_native_object().overload_logical_and(m_environment, resolve_node(a_expression.get_operand(1)));
                    } else {
                        return lhs ? resolve_node(a_expression.get_operand(1)) : object(false);
                    }
                }
                case separator::logical_not:
                    return object::logical_not(m_environment, resolve_node(a_expression.get_operand(0)));
                case separator::bitwise_or:
                    return object::bitwise_or(m_environment, resolve_node(a_expression.get_operand(0)), resolve_node(a_expression.get_operand(1)));
                case separator::bitwise_or_assignment: {
                    object& assignee = resolve_assignable(a_expression.get_operand(0));

                    if (assignee.is_native_object()) {
                        assignee.get_native_object().overload_bitwise_or_assignment(m_environment, resolve_node(a_expression.get_operand(1)));
                    } else {
                        assignee = object::bitwise_or(m_environment, assignee, resolve_node(a_expression.get_operand(1)));
                    }

                    return assignee;
                }
                case separator::bitwise_xor:
                    return object::bitwise_or(m_environment, resolve_node(a_expression.get_operand(0)), resolve_node(a_expression.get_operand(1)));
                case separator::bitwise_xor_assignment: {
                    object& assignee = resolve_assignable(a_expression.get_operand(0));

                    if (assignee.is_native_object()) {
                        assignee.get_native_object().overload_bitwise_xor_assignment(m_environment, resolve_node(a_expression.get_operand(1)));
                    } else {
                        assignee = object::bitwise_xor(m_environment, assignee, resolve_node(a_expression.get_operand(1)));
                    }

                    return assignee;
                }
                case separator::bitwise_and:
                    return object::bitwise_and(m_environment, resolve_node(a_expression.get_operand(0)), resolve_node(a_expression.get_operand(1)));
                case separator::bitwise_and_assignment: {
                    object& assignee = resolve_assignable(a_expression.get_operand(0));

                    if (assignee.is_native_object()) {
                        assignee.get_native_object().overload_bitwise_and_assignment(m_environment, resolve_node(a_expression.get_operand(1)));
                    } else {
                        assignee = object::bitwise_and(m_environment, assignee, resolve_node(a_expression.get_operand(1)));
                    }

                    return assignee;
                }
                case separator::bitwise_not:
                    return object::bitwise_not(m_environment, resolve_node(a_expression.get_operand(0)));
                case separator::shift_right:
                    return object::shift_right(m_environment, resolve_node(a_expression.get_operand(0)), resolve_node(a_expression.get_operand(1)));
                case separator::shift_right_assignment: {
                    object& assignee = resolve_assignable(a_expression.get_operand(0));

                    if (assignee.is_native_object()) {
                        assignee.get_native_object().overload_shift_right_assignment(m_environment, resolve_node(a_expression.get_operand(1)));
                    } else {
                        assignee = object::shift_right(m_environment, assignee, resolve_node(a_expression.get_operand(1)));
                    }

                    return assignee;
                }
                case separator::shift_left:
                    return object::shift_left(m_environment, resolve_node(a_expression.get_operand(0)), resolve_node(a_expression.get_operand(1)));
                case separator::shift_left_assignment: {
                    object& assignee = resolve_assignable(a_expression.get_operand(0));

                    if (assignee.is_native_object()) {
                        assignee.get_native_object().overload_shift_left_assignment(m_environment, resolve_node(a_expression.get_operand(1)));
                    } else {
                        assignee = object::shift_left(m_environment, assignee, resolve_node(a_expression.get_operand(1)));
                    }

                    return assignee;
                }
                case separator::exponent:
                    return object::exponentiate(m_environment, resolve_node(a_expression.get_operand(0)), resolve_node(a_expression.get_operand(1)));
                case separator::exponent_assignment: {
                    object& assignee = resolve_assignable(a_expression.get_operand(0));

                    if (assignee.is_native_object()) {
                        assignee.get_native_object().overload_exponent_assignment(m_environment, resolve_node(a_expression.get_operand(1)));
                    } else {
                        assignee = object::exponentiate(m_environment, assignee, resolve_node(a_expression.get_operand(1)));
                    }

                    return assignee;
                }
                case separator::modulus:
                    return object::modulus(m_environment, resolve_node(a_expression.get_operand(0)), resolve_node(a_expression.get_operand(1)));
                case separator::modulus_assignment: {
                    object& assignee = resolve_assignable(a_expression.get_operand(0));

                    if (assignee.is_native_object()) {
                        assignee.get_native_object().overload_modulus_assignment(m_environment, resolve_node(a_expression.get_operand(1)));
                    } else {
                        assignee = object::modulus(m_environment, assignee, resolve_node(a_expression.get_operand(1)));
                    }

                    return assignee;
                }
                    //case separator::seek:
                case separator::ternary:
                    return resolve_node(a_expression.get_operand(0)) ? resolve_node(a_expression.get_operand(1)) : resolve_node(a_expression.get_operand(2));
                case separator::namespace_index:
                case separator::direct:
                case separator::dot:
                    if (a_expression.count() == 2) {
                        return resolve_assignable(a_expression.get_operand(0)).index(m_environment, detail_resolve_node(a_expression.get_operand(1), node_tags::identifier_as_string));
                    }
                    //case separator::list:
                case separator::length:
                    return resolve_node(a_expression.get_operand(0)).length(m_environment);
                    //case separator::ellipsis:
                    //case separator::end_statement:
                case separator::operation_prefix_increment: {
                    object& assignee = resolve_assignable(a_expression.get_operand(0));

                    if (assignee.is_native_object()) {
                        assignee.get_native_object().overload_prefix_increment(m_environment);
                    } else {
                        assignee = object::add(m_environment, assignee, 1);
                    }

                    return assignee;
                }
                case separator::operation_postfix_increment: {
                    object& assignable = resolve_assignable(a_expression.get_operand(0));

                    if (assignable.is_native_object()) {
                        return assignable.get_native_object().overload_postfix_increment(m_environment);
                    }

                    object initial = assignable;

                    assignable = object::add(m_environment, assignable, 1);

                    return initial;
                }
                case separator::operation_prefix_decrement: {
                    object& assignee = resolve_assignable(a_expression.get_operand(0));

                    if (assignee.is_native_object()) {
                        assignee.get_native_object().overload_prefix_decrement(m_environment);
                    } else {
                        assignee = object::subtract(m_environment, assignee, 1);
                    }

                    return assignee;
                }
                case separator::operation_postfix_decrement: {
                    object& assignable = resolve_assignable(a_expression.get_operand(0));

                    if (assignable.is_native_object()) {
                        return assignable.get_native_object().overload_postfix_decrement(m_environment);
                    }

                    object initial = assignable;

                    assignable = object::subtract(m_environment, assignable, 1);

                    return initial;
                }
                case separator::operation_index:
                    if (a_expression.count() == 2) {
                        return resolve_node(a_expression.get_operand(0)).select(m_environment, resolve_node(a_expression.get_operand(1)));
                    } else if (a_expression.count() > 2) {
                        return resolve_node(a_expression.get_operand(0)).select(
                                m_environment,
                                resolve_node(a_expression.get_operand(1)),
                                resolve_node(a_expression.get_operand(2))
                        );
                    }
                case separator::operation_call: {
                    const auto& callable_node = a_expression.get_operand(0);
                    auto callee = null;
                    std::vector<object> args;

                    if (callable_node.is_expression() || callable_node.is_group()) {
                        const auto& expr = callable_node.get_expression();

                        if (expr.get_operation() == separator::dot) {
                            auto this_object = resolve_node(expr.get_operand(0));
                            callee = this_object.select(m_environment, detail_resolve_node(expr.get_operand(1), node_tags::identifier_as_string));
                            args.push_back(this_object);
                        }
                    } else {
                        callee = resolve_node(callable_node);
                    }

                    for (auto it = a_expression.get_operands().begin() + 1; it != a_expression.get_operands().cend(); ++it) {
                        args.push_back(resolve_node(*it));
                    }

                    return callee.call(m_environment, args);
                }
                case separator::new_object:
                    if (a_expression.count() > 1) {
                        std::vector<object> args;

                        for (auto it = a_expression.get_operands().begin() + 1; it != a_expression.get_operands().cend(); ++it) {
                            args.push_back(resolve_node(*it));
                        }

                        return resolve_node(a_expression.get_operand(0)).new_object(m_environment, args);
                    } else {
                        return resolve_node(a_expression.get_operand(0)).new_object(m_environment);
                    }
                default:
                    return null;
            }
        };

        enum class return_status : enum_base {
            normal,
            function_return,
            loop_continue,
            loop_break
        };

        struct return_state {
            return_status status = return_status::normal;
            object result = null;
        };

        std::function<return_state (const span<node>)> evaluate_block = [this, &local_tables, &evaluate_expression, &evaluate_block, &resolve_assignable_expression](const span<node> a_block) -> return_state {
            local_tables.emplace_back();
            table& local_table = local_tables.back();

            bool prior_eval = true;

            for (const node& n : a_block) {
                switch (n.m_type) {
                    case node::type::expression:
                        evaluate_expression(n.get_expression());
                        break;
                    case node::type::block: {
                        return_state state{ evaluate_block(n.get_block()) };

                        if (state.status != return_status::normal) {
                            return state;
                        }

                        break;
                    }
                    case node::type::if_declaration: {
                        const auto& decl = n.get_if_declaration();

                        prior_eval = evaluate_expression(decl.m_conditional).boolean_evaluate();

                        if (prior_eval) {
                            return_state state{ evaluate_block(decl.m_body) };

                            if (state.status != return_status::normal) {
                                return state;
                            }
                        }

                        break;
                    }
                    case node::type::else_if_declaration: {
                        const auto& decl = n.get_else_if_declaration();

                        if (!prior_eval) {
                            prior_eval = evaluate_expression(decl.m_conditional).boolean_evaluate();

                            if (prior_eval) {
                                return_state state{ evaluate_block(decl.m_body) };

                                if (state.status != return_status::normal) {
                                    return state;
                                }
                            }
                        }

                        break;
                    }
                    case node::type::else_declaration: {
                        const auto& decl = n.get_else_declaration();

                        if (!prior_eval) {
                            return_state state{ evaluate_block(decl) };

                            if (state.status != return_status::normal) {
                                return state;
                            }
                        }

                        break;
                    }
                    case node::type::for_declaration: {
                        const auto& decl = n.get_for_declaration();

                        local_tables.emplace_back();
                        auto& tbl = local_tables.back();

                        evaluate_expression(decl.m_initialization);

                        while (evaluate_expression(decl.m_conditional).boolean_evaluate()) {
                            evaluate_block(decl.m_body);
                            evaluate_expression(decl.m_iteration);
                        }

                        local_tables.pop_back();

                        break;
                    }
                    case node::type::function_declaration: {
                        const auto& decl = n.get_function_declaration();

                        object& assignee = resolve_assignable_expression(decl.m_identifier);

                        auto& env_interpreter = dynamic_cast<interpreter&>(m_environment.execution_provider());

                        env_interpreter.m_function_sources.emplace_back(dynamic_cast<function_source*>(new interpreted_function_source(m_environment, decl.m_parameters, decl.m_body)));

                        function func { m_environment, reinterpret_cast<void*>(env_interpreter.m_function_sources.back().get()) };

                        m_environment.emplace_function_info(func, {
                                decl.m_identifier.to_string(), // TODO: Generate "useful" name for functions.
                                "REBAR INTERNAL;"s + std::to_string(m_environment.get_current_function_id_stack()),
                                m_environment.get_current_function_id_stack(),
                                {} // TODO: Implement full function info for interpreter functions.
                        });

                        assignee = func;

                        break;
                    }
                    case node::type::while_declaration: {
                        const auto& decl = n.get_while_declaration();

                        while (evaluate_expression(decl.m_conditional).boolean_evaluate()) {
                            return_state state{ evaluate_block(decl.m_body) };

                            if (state.status == return_status::function_return) {
                                return state;
                            } else if (state.status == return_status::loop_break) {
                                break;
                            }
                        }

                        break;
                    }
                    case node::type::do_declaration:
                        break;
                    case node::type::switch_declaration:
                        break;
                    case node::type::class_declaration:
                        break;
                    case node::type::return_statement:
                        return { return_status::function_return, evaluate_expression(n.get_return_statement()) };
                    case node::type::break_statement:
                        return { return_status::loop_break, null };
                    case node::type::continue_statement:
                        return { return_status::loop_continue, null };
                    default:
                        break;
                }
            }

            local_tables.pop_back();

            return { return_status::normal, null };
        };

        local_tables.emplace_back();

        auto& arg_table = local_tables.back();

        for (size_t i = 0; i < m_arguments.size(); ++i) {
            const auto& arg = m_arguments[i];
            const auto& identifier = arg.is_token() ? arg : *(arg.get_expression().get_operands().end() - 1);

            if (identifier.is_token()) {
                const auto& tok = identifier.get_token();

                if (tok.is_identifier()) {
                    arg_table[m_environment.str(tok.get_identifier())] = m_environment.arg(i);
                }
            }
        }

        return_state state{ evaluate_block(m_body) };

        local_tables.pop_back();

        return state.result;
    }
}

#endif //REBAR_INTERPRETER_IMPL_HPP
