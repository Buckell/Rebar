//
// Created by maxng on 8/21/2022.
//

#ifndef REBAR_PRELIMINARY_SCAN_HPP
#define REBAR_PRELIMINARY_SCAN_HPP

#include "../compiler.hpp"

namespace rebar {
    void compiler::perform_preliminary_function_scan(function_context& a_ctx, const node::block& a_block) {
        size_t max_locals_count = 0;

        std::function<void (const node::block&, size_t)> block_pass;

        block_pass = [&a_ctx, &block_pass, &max_locals_count] (const node::block& a_block, const size_t a_current_count) {
            size_t current_count = 0;

            // TODO: Count locals in for-declarations, etc.
            for (const auto& node : a_block) {
                switch (node.m_type) {
                    case node::type::expression: {
                        const auto& expression = node.get_expression();

                        if (expression.get_operation() != separator::assignment) {
                            continue;
                        }

                        const auto& assignee = expression.get_operand(0);

                        if (assignee.is_expression()) {
                            const auto& assignee_expression = assignee.get_expression();

                            if (assignee_expression.count() > 1) {
                                const auto& operand1 = assignee_expression.get_operand(0);
                                const auto& operand2 = assignee_expression.get_operand(1);

                                if (operand1.is_token() && operand1.get_token().is_keyword() && operand1.get_token().get_keyword() == keyword::local && operand2.is_token() && operand2.get_token().is_identifier()) {
                                    ++current_count;
                                }
                            }
                        }

                        break;
                    }
                    case node::type::block:
                        block_pass(node.get_block(), a_current_count + current_count);
                        break;
                    case node::type::if_declaration:
                        block_pass(node.get_if_declaration().m_body, a_current_count + current_count);
                        break;
                    case node::type::else_if_declaration:
                        block_pass(node.get_else_if_declaration().m_body, a_current_count + current_count);
                        break;
                    case node::type::else_declaration:
                        block_pass(node.get_else_declaration(), a_current_count + current_count);
                        break;
                    case node::type::for_declaration:
                        block_pass(node.get_for_declaration().m_body, a_current_count + current_count);
                        break;
                    case node::type::while_declaration:
                        block_pass(node.get_while_declaration().m_body, a_current_count + current_count);
                        break;
                    case node::type::do_declaration:
                        block_pass(node.get_do_declaration().m_body, a_current_count + current_count);
                        break;
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

        a_ctx.max_locals_count = max_locals_count;
    }
}

#endif //REBAR_PRELIMINARY_SCAN_HPP
