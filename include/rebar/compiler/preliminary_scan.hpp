//
// Created by maxng on 8/21/2022.
//

#ifndef REBAR_PRELIMINARY_SCAN_HPP
#define REBAR_PRELIMINARY_SCAN_HPP

#include "../compiler.hpp"

namespace rebar {
    void compiler::perform_preliminary_function_scan(function_context& ctx, const node::block& a_block) {
        size_t max_locals_count = 0;

        std::function<void (const node::block&, size_t)> block_pass;
        std::function<void (const node::expression&, size_t&)> expression_pass;

        expression_pass = [&expression_pass] (const node::expression& a_expression, size_t& a_current) noexcept {
            if (a_expression.get_operation() == separator::space) {
                if (a_expression.count() > 1) {
                    const auto& operand1 = a_expression.get_operand(0);
                    const auto& operand2 = a_expression.get_operand(1);

                    if (operand1.is_token() && operand1.get_token().is_keyword() && operand1.get_token().get_keyword() == keyword::local && operand2.is_token() && operand2.get_token().is_identifier()) {
                        ++a_current;
                        return;
                    }
                }
            }

            for (const auto& operand : a_expression.get_operands()) {
                if (operand.is_expression() || operand.is_group()) {
                    expression_pass(operand.get_expression(), a_current);
                }
            }
        };

        block_pass = [&block_pass, &expression_pass, &max_locals_count] (const node::block& a_block, const size_t a_current_count) {
            size_t current_count = 0;

            // TODO: Count locals in for-declarations, etc.
            for (const auto& node : a_block) {
                switch (node.m_type) {
                    case node::type::expression: {
                        expression_pass(node.get_expression(), current_count);
                        break;
                    }
                    case node::type::block:
                        block_pass(node.get_block(), a_current_count + current_count);
                        break;
                    case node::type::if_declaration: {
                        const auto& decl = node.get_if_declaration();
                        expression_pass(decl.m_conditional, current_count);
                        block_pass(decl.m_body, a_current_count + current_count);
                        break;
                    }
                    case node::type::else_if_declaration: {
                        const auto& decl = node.get_else_if_declaration();
                        expression_pass(decl.m_conditional, current_count);
                        block_pass(decl.m_body, a_current_count + current_count);
                        break;
                    }
                    case node::type::else_declaration:
                        block_pass(node.get_else_declaration(), a_current_count + current_count);
                        break;
                    case node::type::for_declaration: {
                        const auto& decl = node.get_for_declaration();
                        expression_pass(decl.m_initialization, current_count);
                        expression_pass(decl.m_conditional, current_count);
                        expression_pass(decl.m_iteration, current_count);
                        block_pass(decl.m_body, a_current_count + current_count);
                        break;
                    }
                    case node::type::while_declaration: {
                        const auto& decl = node.get_while_declaration();
                        expression_pass(decl.m_conditional, current_count);
                        block_pass(decl.m_body, a_current_count + current_count);
                        break;
                    }
                    case node::type::do_declaration: {
                        const auto& decl = node.get_do_declaration();
                        expression_pass(decl.m_conditional, current_count);
                        block_pass(decl.m_body, a_current_count + current_count);
                        break;
                    }
                    case node::type::switch_declaration:
                        // TODO: Implement switch cases;
                        break;
                    case node::type::function_declaration: {
                        const auto& decl = node.get_function_declaration();

                        if (decl.m_tags == function_tags::basic) {
                            ++current_count;
                        }

                        break;
                    }
                    default:
                        break;
                }
            }

            max_locals_count = std::max(a_current_count + current_count, max_locals_count);
        };

        // Perform pass to map local variables to stack space;
        block_pass(a_block, 0);

        ctx.max_locals_count = max_locals_count;
    }
}

#endif //REBAR_PRELIMINARY_SCAN_HPP
