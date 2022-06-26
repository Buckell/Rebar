//
// Created by maxng on 6/21/2022.
//

#ifndef REBAR_PARSER_HPP
#define REBAR_PARSER_HPP

#include <vector>
#include <variant>

#include "definitions.hpp"
#include "lexer.hpp"
#include "operator_precedence.hpp"

namespace rebar {
    struct node : public origin_locked {
        enum class type : enum_base {
            empty,
            token,
            expression,
            block,
            group,
            selector,
            ranged_selector,
            argument_list,
            if_declaration,
            else_if_declaration,
            else_declaration,
            for_declaration,
            function_declaration,
            while_declaration,
            do_declaration,
            switch_declaration,
            class_declaration,
            return_statement,
            immediate_table,
            break_statement,
            continue_statement,
            immediate_array
        };

        struct abstract_syntax_tree {
            separator m_operation;
            std::vector<node> m_operands;

            abstract_syntax_tree() noexcept : m_operation(separator::space) {}

            explicit abstract_syntax_tree(node a_value) noexcept : m_operation(separator::space) {
                m_operands.emplace_back(a_value);
            }

            abstract_syntax_tree(separator a_operation, node lhs, node rhs) noexcept : m_operation(a_operation) {
                m_operands.emplace_back(lhs);
                m_operands.emplace_back(rhs);
            }

            explicit abstract_syntax_tree(const separator a_operation) noexcept : m_operation(a_operation) {}

            [[nodiscard]] bool empty() const noexcept {
                return m_operands.empty();
            }

            [[nodiscard]] size_t count() const noexcept {
                return m_operands.size();
            }

            [[nodiscard]] separator get_operation() const noexcept {
                return m_operation;
            }

            [[nodiscard]] const node& get_operand(size_t a_index) const noexcept {
                return m_operands[a_index];
            }

            [[nodiscard]] const span<node> get_operands() const noexcept {
                return m_operands;
            }

            void add_operand(node a_value) noexcept {
                m_operands.push_back(std::move(a_value));
            }

            [[nodiscard]] std::string to_string() const noexcept {
                if (m_operation == separator::space) {
                    std::string string;

                    for (const auto& operand : m_operands) {
                        string += operand.to_string();
                    }

                    return string;
                }

                std::string string{ separator_to_string(m_operation) };

                string += " ( ";

                for (const auto& operand : m_operands) {
                    string += operand.to_string();
                }

                return string + "); ";
            }
        };

        using block = std::vector<node>;
        using expression = abstract_syntax_tree;
        using selector = expression;
        using ranged_selector = std::vector<node>;
        using argument_list = std::vector<node>;
        using return_statement = expression;
        using immediate_array = std::vector<node>;

        struct group : public expression, public origin_locked {};

        struct if_declaration {
            group m_conditional;
            block m_body;

            if_declaration(group a_conditional, block a_body) noexcept : m_conditional(std::move(a_conditional)), m_body(std::move(a_body)) {}

            if_declaration(const if_declaration& a_decl) = default;
            if_declaration(if_declaration&& a_decl) noexcept = default;
        };

        using else_if_declaration = if_declaration;

        using else_declaration = block;

        struct for_declaration {
            group m_initialization;
            group m_conditional;
            group m_iteration;
            block m_body;

            for_declaration(group a_initialization, group a_conditional, group a_iteration, block a_body) noexcept :
                    m_initialization(std::move(a_initialization)), m_conditional(std::move(a_conditional)),
                    m_iteration(std::move(a_iteration)), m_body(std::move(a_body)) {}

            for_declaration(const for_declaration& a_decl) = default;
            for_declaration(for_declaration&& a_decl) noexcept = default;
        };

        struct function_declaration {
            group m_identifier;
            function_tags m_tags;
            argument_list m_parameters;
            block m_body;

            function_declaration(group a_identifier, const function_tags a_tags, argument_list a_parameters, block a_body) noexcept :
                    m_identifier(std::move(a_identifier)), m_tags(a_tags), m_parameters(std::move(a_parameters)), m_body(std::move(a_body)) {}

            function_declaration(const function_declaration& a_decl) = default;
            function_declaration(function_declaration&& a_decl) noexcept = default;
        };

        using while_declaration = if_declaration;
        using do_declaration = if_declaration;

        struct switch_declaration {
            struct case_declaration {
                bool m_ranged;
                std::pair<group, group> m_data;
                block m_body;

                case_declaration(group a_group, block a_body) noexcept : m_ranged(false), m_data(std::move(a_group), group()), m_body(std::move(a_body)) {}
                case_declaration(group a_begin, group a_end, block a_body) noexcept : m_ranged(true), m_data(std::move(a_begin), std::move(a_end)), m_body(std::move(a_body)) {}

                case_declaration(const case_declaration& a_decl) = default;
                case_declaration(case_declaration&& a_decl) noexcept = default;
            };

            group m_expression;
            std::vector<case_declaration> m_cases;

            switch_declaration(group a_expression, std::vector<case_declaration> a_cases) noexcept : m_expression(std::move(a_expression)), m_cases(std::move(a_cases)) {}

            switch_declaration(const switch_declaration& a_decl) = default;
            switch_declaration(switch_declaration&& a_decl) noexcept = default;
        };

        struct class_declaration {
            std::string_view m_identifier;
            class_tags m_tags;
            std::vector<function_declaration> m_functions;

            class_declaration(const std::string_view a_identifier, const class_tags a_tags, std::vector<function_declaration> a_functions) noexcept
                    : m_identifier(a_identifier), m_tags(a_tags), m_functions(std::move(a_functions)) {}

            class_declaration(const class_declaration& a_decl) = default;
            class_declaration(class_declaration&& a_decl) noexcept = default;
        };

        struct immediate_table {
            std::vector<std::pair<node, expression>> m_entries;

            immediate_table() noexcept {}

            immediate_table(const immediate_table& a_decl) = default;
            immediate_table(immediate_table&& a_decl) noexcept = default;
        };

        using data_type = std::variant<std::nullptr_t, const token*, expression, std::vector<node>, if_declaration, for_declaration, function_declaration, switch_declaration, class_declaration, immediate_table>;

        type m_type;
        data_type m_data;

        node() noexcept : m_type(type::empty), m_data(nullptr) {}
        node(const span<token> a_tokens, const span<source_position> a_source_positions, const type a_type, data_type a_data) noexcept : origin_locked { a_tokens, a_source_positions }, m_type(a_type), m_data(std::move(a_data)) {}

        template <typename t_type, typename... t_args>
        node(const span<token> a_tokens, const span<source_position> a_source_positions, const type a_type, std::in_place_type_t<t_type> a_in_place_type, t_args... a_args) noexcept : origin_locked { a_tokens, a_source_positions }, m_type(a_type), m_data(a_in_place_type, std::forward<t_args>(a_args)...) {}

        node(const node& a_node) = default;
        node(node&& a_node) noexcept = default;

        [[nodiscard]] bool is_empty() const noexcept {
            return m_type == type::empty;
        }

        [[nodiscard]] bool is_token() const noexcept {
            return m_type == type::token;
        }

        [[nodiscard]] bool is_expression() const noexcept {
            return m_type == type::expression;
        }

        [[nodiscard]] bool is_block() const noexcept {
            return m_type == type::block;
        }

        [[nodiscard]] bool is_group() const noexcept {
            return m_type == type::group;
        }

        [[nodiscard]] bool is_selector() const noexcept {
            return m_type == type::selector;
        }

        [[nodiscard]] bool is_ranged_selector() const noexcept {
            return m_type == type::ranged_selector;
        }

        [[nodiscard]] bool is_argument_list() const noexcept {
            return m_type == type::argument_list;
        }

        [[nodiscard]] bool is_if_declaration() const noexcept {
            return m_type == type::if_declaration;
        }

        [[nodiscard]] bool is_else_if_declaration() const noexcept {
            return m_type == type::else_if_declaration;
        }

        [[nodiscard]] bool is_else_declaration() const noexcept {
            return m_type == type::else_declaration;
        }

        [[nodiscard]] bool is_for_declaration() const noexcept {
            return m_type == type::for_declaration;
        }

        [[nodiscard]] bool is_function_declaration() const noexcept {
            return m_type == type::function_declaration;
        }

        [[nodiscard]] bool is_while_declaration() const noexcept {
            return m_type == type::while_declaration;
        }

        [[nodiscard]] bool is_do_declaration() const noexcept {
            return m_type == type::do_declaration;
        }

        [[nodiscard]] bool is_switch_declaration() const noexcept {
            return m_type == type::switch_declaration;
        }

        [[nodiscard]] bool is_class_declaration() const noexcept {
            return m_type == type::class_declaration;
        }

        [[nodiscard]] bool is_return_statement() const noexcept {
            return m_type == type::return_statement;
        }

        [[nodiscard]] bool is_immediate_table() const noexcept {
            return m_type == type::immediate_table;
        }

        [[nodiscard]] bool is_break_statement() const noexcept {
            return m_type == type::break_statement;
        }

        [[nodiscard]] bool is_continue_statement() const noexcept {
            return m_type == type::continue_statement;
        }

        [[nodiscard]] bool is_immediate_array() const noexcept {
            return m_type == type::immediate_array;
        }

        [[nodiscard]] const token& get_token() const noexcept {
            return *std::get<const token*>(m_data);
        }

        [[nodiscard]] expression& get_expression() noexcept {
            return std::get<expression>(m_data);
        }

        [[nodiscard]] const expression& get_expression() const noexcept {
            return std::get<expression>(m_data);
        }

        [[nodiscard]] block& get_block() noexcept {
            return std::get<block>(m_data);
        }

        [[nodiscard]] const block& get_block() const noexcept {
            return std::get<block>(m_data);
        }

        [[nodiscard]] selector& get_selector() noexcept {
            return std::get<selector>(m_data);
        }

        [[nodiscard]] const selector& get_selector() const noexcept {
            return std::get<selector>(m_data);
        }

        [[nodiscard]] ranged_selector& get_ranged_selector() noexcept {
            return std::get<ranged_selector>(m_data);
        }

        [[nodiscard]] const ranged_selector& get_ranged_selector() const noexcept {
            return std::get<ranged_selector>(m_data);
        }

        [[nodiscard]] argument_list& get_argument_list() noexcept {
            return std::get<argument_list>(m_data);
        }

        [[nodiscard]] const argument_list& get_argument_list() const noexcept {
            return std::get<argument_list>(m_data);
        }

        [[nodiscard]] if_declaration& get_if_declaration() noexcept {
            return std::get<if_declaration>(m_data);
        }

        [[nodiscard]] const if_declaration& get_if_declaration() const noexcept {
            return std::get<if_declaration>(m_data);
        }

        [[nodiscard]] else_if_declaration& get_else_if_declaration() noexcept {
            return std::get<else_if_declaration>(m_data);
        }

        [[nodiscard]] const else_if_declaration& get_else_if_declaration() const noexcept {
            return std::get<else_if_declaration>(m_data);
        }

        [[nodiscard]] else_declaration& get_else_declaration() noexcept {
            return std::get<else_declaration>(m_data);
        }

        [[nodiscard]] const else_declaration& get_else_declaration() const noexcept {
            return std::get<else_declaration>(m_data);
        }

        [[nodiscard]] for_declaration& get_for_declaration() noexcept {
            return std::get<for_declaration>(m_data);
        }

        [[nodiscard]] const for_declaration& get_for_declaration() const noexcept {
            return std::get<for_declaration>(m_data);
        }

        [[nodiscard]] function_declaration& get_function_declaration() noexcept {
            return std::get<function_declaration>(m_data);
        }

        [[nodiscard]] const function_declaration& get_function_declaration() const noexcept {
            return std::get<function_declaration>(m_data);
        }

        [[nodiscard]] while_declaration& get_while_declaration() noexcept {
            return std::get<while_declaration>(m_data);
        }

        [[nodiscard]] const while_declaration& get_while_declaration() const noexcept {
            return std::get<while_declaration>(m_data);
        }

        [[nodiscard]] do_declaration& get_do_declaration() noexcept {
            return std::get<do_declaration>(m_data);
        }

        [[nodiscard]] const do_declaration& get_do_declaration() const noexcept {
            return std::get<do_declaration>(m_data);
        }

        [[nodiscard]] switch_declaration& get_switch_declaration() noexcept {
            return std::get<switch_declaration>(m_data);
        }

        [[nodiscard]] const switch_declaration& get_switch_declaration() const noexcept {
            return std::get<switch_declaration>(m_data);
        }

        [[nodiscard]] class_declaration& get_class_declaration() noexcept {
            return std::get<class_declaration>(m_data);
        }

        [[nodiscard]] const class_declaration& get_class_declaration() const noexcept {
            return std::get<class_declaration>(m_data);
        }

        [[nodiscard]] return_statement& get_return_statement() noexcept {
            return std::get<return_statement>(m_data);
        }

        [[nodiscard]] const return_statement& get_return_statement() const noexcept {
            return std::get<return_statement>(m_data);
        }

        [[nodiscard]] immediate_table& get_immediate_table() noexcept {
            return std::get<immediate_table>(m_data);
        }

        [[nodiscard]] const immediate_table& get_immediate_table() const noexcept {
            return std::get<immediate_table>(m_data);
        }

        [[nodiscard]] immediate_array& get_immediate_array() noexcept {
            return std::get<immediate_array>(m_data);
        }

        [[nodiscard]] const immediate_array& get_immediate_array() const noexcept {
            return std::get<immediate_array>(m_data);
        }

        [[nodiscard]] std::string to_string() const noexcept {
            switch (m_type) {
                case type::empty:
                    return "EMPTY; ";
                case type::token:
                    return get_token().to_string();
                case type::expression:
                case type::group:
                    return std::string{ "EXPRESSION { " } + get_expression().to_string() + "}; ";
                case type::block: {
                    std::string string{ "BLOCK { " };

                    for (const auto &n : get_block()) {
                        string += n.to_string();
                    }

                    return string + "}; ";
                }
                case type::selector: {
                    return std::string{ "SELECTOR { " } + get_expression().to_string() + "}; ";
                }
                case type::ranged_selector: {
                    const auto& range = get_ranged_selector();
                    return std::string{ "RANGED SELECTOR { " } + range[0].to_string() + range[1].to_string() + "}; ";
                }
                case type::argument_list: {
                    std::string string{ "ARGUMENT LIST { " };

                    for (const auto &n : get_argument_list()) {
                        string += "ARGUMENT GROUP { ";
                        string += n.to_string();
                        string += "}; ";
                    }

                    return string + "}; ";
                }
                case type::if_declaration: {
                    const auto& declaration = get_if_declaration();

                    std::string string{ "IF DECLARATION { CONDITIONAL GROUP { " };

                    string += declaration.m_conditional.to_string();

                    string += "}; BODY BLOCK { ";

                    for (const auto &n : declaration.m_body) {
                        string += n.to_string();
                    }

                    return string + "}; }; ";
                }
                case type::else_if_declaration: {
                    const auto& declaration = get_else_if_declaration();

                    std::string string{ "ELSE IF DECLARATION { CONDITIONAL GROUP { " };

                    string += declaration.m_conditional.to_string();

                    string += "}; BODY BLOCK { ";

                    for (const auto &n : declaration.m_body) {
                        string += n.to_string();
                    }

                    return string + "}; }; ";
                }
                case type::else_declaration: {
                    std::string string{ "ELSE DECLARATION { " };

                    for (const auto &n : get_block()) {
                        string += n.to_string();
                    }

                    return string + "}; ";
                }
                case type::for_declaration: {
                    const auto& declaration = get_for_declaration();

                    std::string string{ "FOR DECLARATION { INITIALIZATION GROUP { " };
                    string += declaration.m_initialization.to_string();
                    string += "}; CONDITIONAL GROUP { ";
                    string += declaration.m_conditional.to_string();
                    string += "}; ITERATION GROUP { ";
                    string += declaration.m_iteration.to_string();
                    string += "}; BODY BLOCK { ";

                    for (const auto &n : declaration.m_body) {
                        string += n.to_string();
                    }

                    return string + "}; }; ";
                }
                case type::function_declaration: {
                    const auto& declaration = get_function_declaration();

                    std::string string{ "FUNCTION DECLARATION { IDENTIFIER GROUP { " };
                    string += declaration.m_identifier.to_string();
                    string += "}; FUNCTION TAGS: ";
                    string += function_tags_to_string(declaration.m_tags);
                    string += "; PARAMETER LIST { ";

                    for (const auto &n : declaration.m_parameters) {
                        string += "PARAMETER GROUP { ";
                        string += n.to_string();
                        string += "}; ";
                    }

                    string += "}; BODY BLOCK { ";

                    for (const auto &n : declaration.m_body) {
                        string += n.to_string();
                    }

                    return string + "}; }; ";
                }
                case type::while_declaration: {
                    const auto& declaration = get_while_declaration();

                    std::string string{ "WHILE DECLARATION { CONDITIONAL GROUP { " };
                    string += declaration.m_conditional.to_string();
                    string += "}; BODY BLOCK { ";

                    for (const auto &n : declaration.m_body) {
                        string += n.to_string();
                    }

                    return string + "}; }; ";
                }
                case type::do_declaration: {
                    const auto& declaration = get_do_declaration();

                    std::string string{ "DO DECLARATION { CONDITIONAL GROUP { " };
                    string += declaration.m_conditional.to_string();
                    string += "}; BODY BLOCK { ";

                    for (const auto &n : declaration.m_body) {
                        string += n.to_string();
                    }

                    return string + "}; }; ";
                }
                case type::switch_declaration: {
                    const auto& declaration = get_switch_declaration();

                    std::string string{ "SWITCH DECLARATION { GROUP { " };
                    string += declaration.m_expression.to_string();
                    string += "}; CASES { ";

                    for (const auto &n : declaration.m_cases) {
                        if (n.m_ranged) {
                            string += "RANGED CASE DECLARATION { RANGE { { ";
                            string += n.m_data.first.to_string();
                            string += "}; { ";
                            string += n.m_data.second.to_string();
                            string += "}; }; BODY BLOCK { ";

                            for (const auto &nn : n.m_body) {
                                string += nn.to_string();
                            }

                            string += "}; }; ";
                        } else {
                            string += "CASE DECLARATION { GROUP { ";
                            string += n.m_data.first.to_string();
                            string += "}; BODY BLOCK { ";

                            for (const auto &nn : n.m_body) {
                                string += nn.to_string();
                            }

                            string += "}; }; ";
                        }
                    }

                    return string + "}; }; ";
                }
                case type::class_declaration: {
                    std::string string{ "CLASS DECLARATION { IDENTIFIER: \"" };

                    const auto& declaration = get_class_declaration();

                    string += declaration.m_identifier;

                    string += "\"; CLASS TAGS: ";
                    string += class_tags_to_string(declaration.m_tags);
                    string += "; FUNCTION LIST { ";

                    for (const auto &func : declaration.m_functions) {
                        string += "FUNCTION { IDENTIFIER GROUP { ";
                        string += func.m_identifier.to_string();
                        string += "}; FUNCTION TAGS: ";
                        string += function_tags_to_string(func.m_tags);
                        string += "; PARAMETER LIST { ";

                        for (const auto &nn : func.m_parameters) {
                            string += "PARAMETER GROUP { ";
                            string += nn.to_string();
                            string += "}; ";
                        }

                        string += "}; BODY BLOCK { ";

                        for (const auto &n : func.m_body) {
                            string += n.to_string();
                        }

                        return string + "}; }; ";
                    }

                    return string + "}; }; ";
                }
                case type::return_statement:
                    return std::string{ "RETURN EXPRESSION { " } + get_expression().to_string() + "}; ";
                case type::immediate_table: {
                    std::string string{ "IMMEDIATE TABLE { " };

                    const auto& tbl = get_immediate_table();

                    for (const auto& entry : tbl.m_entries) {
                        string += "ENTRY { KEY { ";
                        string += entry.first.to_string();
                        string += "}; VALUE { ";
                        string += entry.second.to_string();
                        string += "}; ";
                    }

                    return string + "}; ";
                }
                case type::immediate_array: {
                    std::string string{ "IMMEDIATE ARRAY { " };

                    const auto& arr = get_immediate_array();

                    for (const auto& n : arr) {
                        string += n.to_string();
                    }

                    return string + "}; ";
                }
                case type::break_statement:
                    return "BREAK; ";
                case type::continue_statement:
                    return "CONTINUE; ";
            }

            return "";
        }

        [[nodiscard]] bool operator==(const type rhs) const noexcept {
            return m_type == rhs;
        }
    };

    /* Old version; use ambiguous version, find_next. New version moved to global/rebar namespace.
    [[nodiscard]] span<token>::const_iterator find_next_separator(const span<token> a_tokens, const separator a_separator, const separator a_open_exclude = separator::space, const separator a_close_exclude = separator::space) noexcept {
        span<token>::iterator it = a_tokens.begin();
        span<token>::iterator end_it = a_tokens.end();

        if (a_open_exclude != separator::space) {
            size_t exclude_increment = 0;

            for (; it != end_it; ++it) {
                if (*it == a_separator && exclude_increment == 0) {
                    return it;
                }

                exclude_increment += *it == a_open_exclude;
                exclude_increment -= *it == a_close_exclude;
            }
        } else {
            for (; it != end_it; ++it) {
                if (*it == a_separator) {
                    return it;
                }
            }
        }

        return end_it;
    }
    */

    [[nodiscard]] node::group parse_group(const span<token> a_tokens, const span<source_position> a_source_positions) noexcept;

    [[nodiscard]] node::argument_list parse_arguments(const span<token> a_tokens, const span<source_position> a_source_positions) noexcept {
        std::vector<node> groups;

        span<token>::iterator last_token = a_tokens.begin();

        for (size_t i = 0; i < a_tokens.size(); ++i) {
            span<token>::iterator next_token = find_next(a_tokens.subspan(i), separator::list, separator::group_open, separator::group_close);

            if (next_token == a_tokens.end()) {
                groups.emplace_back(
                    a_tokens.subspan(i),
                    a_source_positions.subspan(i),
                    node::type::expression,
                    parse_group(a_tokens.subspan(i), a_source_positions.subspan(i))
                );

                break;
            }

            span<token>::iterator::difference_type difference = std::distance(next_token, last_token);

            groups.emplace_back(
                a_tokens.subspan(i, difference),
                a_source_positions.subspan(i, difference),
                node::type::expression,
                parse_group(a_tokens.subspan(i, difference), a_source_positions.subspan(i, difference))
            );

            i += difference;

            last_token = next_token + 1;
        }

        return groups;
    }

    [[nodiscard]] node::abstract_syntax_tree parse_ast(const span<node>& a_nodes) {
        if (a_nodes.empty()) {
            return {};
        } else if (a_nodes.size() == 1) {
            return node::abstract_syntax_tree{ { a_nodes[0] } };
        } else if (a_nodes.size() == 2) {
            if (a_nodes[0].is_token()) {
                const auto &first_operand = a_nodes[0].get_token();

                if (first_operand.is_separator()) {
                    switch (first_operand.get_separator()) {
                        case separator::increment: {
                            node::abstract_syntax_tree ast{ separator::operation_prefix_increment };
                            ast.add_operand(a_nodes[1]);
                            return ast;
                        }
                        case separator::decrement: {
                            node::abstract_syntax_tree ast{ separator::operation_prefix_decrement };
                            ast.add_operand(a_nodes[1]);
                            return ast;
                        }
                        case separator::logical_not: {
                            node::abstract_syntax_tree ast{ separator::logical_not };
                            ast.add_operand(a_nodes[1]);
                            return ast;
                        }
                        case separator::bitwise_not: {
                            node::abstract_syntax_tree ast{ separator::bitwise_not };
                            ast.add_operand(a_nodes[1]);
                            return ast;
                        }
                        case separator::new_object: {
                            node::abstract_syntax_tree ast{ separator::new_object };
                            ast.add_operand(a_nodes[1]);
                            return ast;
                        }
                        default:
                            break;
                    }
                }
            }

            if (a_nodes[1].is_token()) {
                const auto &second_operand = a_nodes[1].get_token();

                if (second_operand.is_separator()) {
                    switch (second_operand.get_separator()) {
                        case separator::increment: {
                            node::abstract_syntax_tree ast{ separator::operation_postfix_increment };
                            ast.add_operand(a_nodes[0]);
                            return ast;
                        }
                        case separator::decrement: {
                            node::abstract_syntax_tree ast{ separator::operation_postfix_decrement };
                            ast.add_operand(a_nodes[0]);
                            return ast;
                        }
                        default:
                            break;
                    }
                }
            } else if (a_nodes[1].is_group()) {
                return {
                        separator::operation_call,
                        a_nodes[0],
                        a_nodes[1]
                };
            } else if (a_nodes[1].is_argument_list()) {
                node::abstract_syntax_tree ast{ separator::operation_call };
                ast.add_operand(a_nodes[0]);

                for (const auto& arg : a_nodes[1].get_argument_list()) {
                    ast.add_operand(arg);
                }

                return ast;
            } else if (a_nodes[1].is_selector()) {
                return {
                        separator::operation_index,
                        a_nodes[0],
                        {
                            a_nodes[1].m_origin_tokens,
                            a_nodes[1].m_origin_source_positions,
                            node::type::expression,
                            a_nodes[1].get_selector()
                        }
                };
            } else if (a_nodes[1].is_ranged_selector()) {
                node::abstract_syntax_tree ast{ separator::operation_index };
                ast.add_operand(a_nodes[0]);

                const auto& ranged = a_nodes[1].get_ranged_selector();

                ast.add_operand(ranged[0]);
                ast.add_operand(ranged[1]);

                return ast;
            }
        }

        const auto found = std::find_if(a_nodes.begin(), a_nodes.end(), [](const node& a_node) noexcept -> bool {
            return a_node.is_token() && a_node.get_token().is_separator();
        });

        if (found == a_nodes.cend()) {
            bool flags = true;

            for (auto it = a_nodes.begin(); it != a_nodes.cend() - 1; ++it) {
                if (it->is_token()) {
                    const auto& tok = it->get_token();

                    if (tok.is_keyword()) {
                        const auto kw = tok.get_keyword();

                        if (kw != keyword::local && kw != keyword::constant && kw != keyword::function) {
                            flags = false;
                            break;
                        }
                    }
                }
            }

            if (flags) {
                node::abstract_syntax_tree ast{ separator::space };

                for (const auto& n : a_nodes) {
                    ast.add_operand(n);
                }

                return ast;
            }

            // TODO: Throw error--invalid expression.
        }

        size_t min_precedence = get_separator_info(found->get_token().get_separator()).precedence();
        auto min_separator_it = found;

        for (auto it = min_separator_it; it != a_nodes.cend(); ++it) {
            if (it->is_token()) {
                const auto &tok = it->get_token();

                if (tok.is_separator()) {
                    separator_info info = get_separator_info(tok.get_separator());
                    min_precedence = std::min(min_precedence, info.precedence());

                    if (info.precedence() == min_precedence) {
                        min_separator_it = it;
                    }
                }
            }
        }

        separator sep = min_separator_it->get_token().get_separator();
        separator_info info = get_separator_info(sep);

        const auto& begin_node = *a_nodes.begin();
        const auto& end_node = *(a_nodes.end() - 1);

        if (info.has_single_operand()) {
            node::abstract_syntax_tree ast{ sep };

            // TODO: Remove undesirable span constructions. Assign to independent intermediate variables.

            if (begin_node.is_token() && begin_node.get_token() == sep) {
                ast.add_operand(a_nodes.size() == 2 ? end_node : node {
                    span<token>(a_nodes[1].m_origin_tokens.begin(), end_node.m_origin_tokens.end()),
                    span<source_position>(a_nodes[1].m_origin_source_positions.begin(), end_node.m_origin_source_positions.end()),
                    node::type::expression,
                    parse_ast(a_nodes.subspan(1))
                });
            } else if (end_node.is_token() && end_node.get_token() == sep) {
                ast.add_operand(a_nodes.size() == 2 ? begin_node : node {
                    span<token>(begin_node.m_origin_tokens.begin(), (a_nodes.end() - 2)->m_origin_tokens.end()),
                    span<source_position>(begin_node.m_origin_source_positions.begin(), (a_nodes.end() - 2)->m_origin_source_positions.end()),
                    node::type::expression,
                    parse_ast(a_nodes.subspan(0, a_nodes.size() - 1))
                });
            }

            return ast;
        }

        if ((end_node.is_group() || end_node.is_selector() || end_node.is_ranged_selector() || end_node.is_expression() || end_node.is_argument_list()) && !((a_nodes.end() - 2)->is_token() && (a_nodes.end() - 2)->get_token().is_separator())) {
            if (min_precedence >= get_separator_info(separator::group_open).precedence()) {
                span<node> lhs_nodes(a_nodes.begin(), a_nodes.end() - 1);
                node lhs = (lhs_nodes.size() == 1) ? lhs_nodes[0] : node {
                    span<token>(lhs_nodes.begin()->m_origin_tokens.begin(), (lhs_nodes.end() - 1)->m_origin_tokens.end()),
                    span<source_position>(lhs_nodes.begin()->m_origin_source_positions.begin(), (lhs_nodes.end() - 1)->m_origin_source_positions.end()),
                    node::type::expression,
                    parse_ast(lhs_nodes)
                };

                if (end_node.is_group()) {
                    return {
                        separator::operation_call,
                        lhs,
                        end_node
                    };
                } else if (end_node.is_argument_list()) {
                    const auto& args = end_node.get_argument_list();

                    node::abstract_syntax_tree ast{ separator::operation_call };

                    ast.add_operand(lhs);

                    for (const auto& arg : args) {
                        ast.add_operand(arg);
                    }

                    return ast;
                } else if (end_node.is_selector()) {
                    return {
                        separator::operation_index,
                        lhs,
                        end_node
                    };
                } else if (end_node.is_ranged_selector()) {
                    const auto& ranged = end_node.get_ranged_selector();

                    node::abstract_syntax_tree ast{ separator::operation_index };

                    ast.add_operand(lhs);
                    ast.add_operand(ranged[0]);
                    ast.add_operand(ranged[1]);

                    return ast;
                }
            }
        }

        span<node> lhs_nodes(a_nodes.begin(), min_separator_it);
        span<node> rhs_nodes(min_separator_it + 1, a_nodes.end());

        node lhs = lhs_nodes.size() == 1 ? lhs_nodes[0] : node {
            span<token>(lhs_nodes.begin()->m_origin_tokens.begin(), (lhs_nodes.end() - 1)->m_origin_tokens.end()),
            span<source_position>(lhs_nodes.begin()->m_origin_source_positions.begin(), (lhs_nodes.end() - 1)->m_origin_source_positions.end()),
            node::type::expression,
            parse_ast(lhs_nodes)
        };

        if (sep == separator::ternary) {
            node::abstract_syntax_tree ast{ sep };

            ast.add_operand(lhs);

            span<node>::iterator ternary_break_find = find_next(rhs_nodes, [](const node& a_node) noexcept -> bool {
                return a_node.is_token() && a_node.get_token() == separator::seek;
            }, [](const node& a_node) noexcept -> find_next_exclude {
                if (a_node.is_token()) {
                    const auto& tok = a_node.get_token();

                    if (tok == separator::ternary) {
                        return find_next_exclude::open;
                    } else if (tok == separator::seek) {
                        return find_next_exclude::close;
                    }
                }
            });

            span<node> ternary_lhs_nodes(min_separator_it + 1, ternary_break_find);
            span<node> ternary_rhs_nodes(ternary_break_find + 1, rhs_nodes.end());

            node lhs = ternary_lhs_nodes.size() == 1 ? ternary_lhs_nodes[0] : node {
                    span<token>(ternary_lhs_nodes.begin()->m_origin_tokens.begin(), (ternary_lhs_nodes.end() - 1)->m_origin_tokens.end()),
                    span<source_position>(ternary_lhs_nodes.begin()->m_origin_source_positions.begin(), (ternary_lhs_nodes.end() - 1)->m_origin_source_positions.end()),
                    node::type::expression,
                parse_ast(ternary_lhs_nodes)
            };

            node rhs = ternary_rhs_nodes.size() == 1 ? ternary_rhs_nodes[0] : node {
                span<token>(ternary_rhs_nodes.begin()->m_origin_tokens.begin(), (ternary_rhs_nodes.end() - 1)->m_origin_tokens.end()),
                span<source_position>(ternary_rhs_nodes.begin()->m_origin_source_positions.begin(), (ternary_rhs_nodes.end() - 1)->m_origin_source_positions.end()),
                node::type::expression,
                parse_ast(ternary_rhs_nodes)
            };

            ast.add_operand(lhs);
            ast.add_operand(rhs);

            return ast;
        }

        node rhs = rhs_nodes.size() == 1 ? rhs_nodes[0] : node{
            span<token>(rhs_nodes.begin()->m_origin_tokens.begin(), (rhs_nodes.end() - 1)->m_origin_tokens.end()),
            span<source_position>(rhs_nodes.begin()->m_origin_source_positions.begin(), (rhs_nodes.end() - 1)->m_origin_source_positions.end()),
            node::type::expression,
            parse_ast(rhs_nodes)
        };

        return { sep, lhs, rhs };
    }

    // Routine to parse groups.
    [[nodiscard]] node::group parse_group(const span<token> a_tokens, const span<source_position> a_source_positions) noexcept {
        std::vector<node> nodes;

        for (size_t i = 0; i < a_tokens.size(); ++i) {
            const token& tok = a_tokens[i];

            if (tok == separator::group_open) {
                // Parsing group.

                span<token>::iterator begin_group = a_tokens.begin() + i + 1;
                span<token>::iterator end_group = find_next(a_tokens.subspan(i + 1), separator::group_close, separator::group_open, separator::group_close);

                span<token> captured_tokens(begin_group, end_group);
                span<source_position> captured_source_positions = a_source_positions.subspan(i + 1, end_group - begin_group);

                span<token> group_tokens(a_tokens.begin() + i, end_group + 1);
                span<source_position> group_source_positions = a_source_positions.subspan(i, group_tokens.size());

                span<token>::iterator arg_list_token = find_next(captured_tokens, separator::list, [](const token& a_token) noexcept -> find_next_exclude {
                    if (a_token == separator::group_open || a_token == separator::selector_open || a_token == separator::scope_open) {
                        return find_next_exclude::open;
                    } else if (a_token == separator::group_close || a_token == separator::selector_close || a_token == separator::scope_close) {
                        return find_next_exclude::close;
                    } else {
                        return find_next_exclude::none;
                    }
                });

                if (arg_list_token != captured_tokens.cend()) {
                    nodes.emplace_back(
                        group_tokens,
                        group_source_positions,
                        node::type::argument_list,
                        parse_arguments(captured_tokens, captured_source_positions)
                    );
                } else {
                    nodes.emplace_back(
                        group_tokens,
                        group_source_positions,
                        node::type::group,
                        parse_group(captured_tokens, captured_source_positions)
                    );
                }

                i += std::distance(end_group, a_tokens.begin() + i);
            } else if (tok == separator::selector_open) {
                // Parsing selector.

                span<token>::iterator end_selector_token = find_next(a_tokens.subspan(i + 1), separator::selector_close, separator::selector_open, separator::selector_close);

                span<token> captured_tokens(a_tokens.begin() + i + 1, end_selector_token);
                span<source_position> captured_source_positions = a_source_positions.subspan(i + 1, captured_tokens.size());

                span<token> body_tokens(a_tokens.begin() + i, end_selector_token + 1);
                span<source_position> body_source_positions = a_source_positions.subspan(i, body_tokens.size());

                const auto find_next_entry = [](const span<token> a_tokens) noexcept -> span<token>::iterator {
                    return find_next(a_tokens, separator::list, [](const token& a_token) -> find_next_exclude {
                        if (a_token == separator::group_open || a_token == separator::selector_open || a_token == separator::scope_open) {
                            return find_next_exclude::open;
                        } else if (a_token == separator::group_close || a_token == separator::selector_close || a_token == separator::scope_close) {
                            return find_next_exclude::close;
                        } else {
                            return find_next_exclude::none;
                        }
                    });
                };

                span<token>::iterator array_list_token = find_next_entry(captured_tokens);

                if (array_list_token != captured_tokens.cend()) {
                    // Immediate array.

                    std::vector<node> array;

                    span<token>::iterator last_entry = a_tokens.begin() + i + 1;
                    span<token>::iterator entry_end = find_next_entry(span<token>(last_entry, end_selector_token));

                    span<source_position>::iterator last_entry_sp = a_source_positions.begin() + i + 1;
                    span<source_position>::iterator entry_end_sp = last_entry_sp + (entry_end - last_entry);

                    do {
                        span<token> entry_tokens(last_entry, entry_end);
                        span<source_position> entry_source_positions(last_entry_sp, entry_end_sp);

                        if (!entry_tokens.empty()) {
                            if (entry_tokens.size() == 1) {
                                array.emplace_back(
                                    entry_tokens,
                                    entry_source_positions,
                                    node::type::token,
                                    &entry_tokens[0]
                                );
                            } else {
                                array.emplace_back(
                                    entry_tokens,
                                    entry_source_positions,
                                    node::type::expression,
                                    parse_group(entry_tokens, entry_source_positions)
                                );
                            }
                        }

                        last_entry = entry_end + 1;

                        if (last_entry >= end_selector_token) {
                            break;
                        }

                        entry_end = find_next_entry(span<token>(last_entry, end_selector_token));
                    } while (entry_end != last_entry);

                    nodes.emplace_back(
                        body_tokens,
                        body_source_positions,
                        node::type::immediate_array,
                        std::move(array)
                    );
                } else {
                    // Selector.

                    span<token>::iterator selector_seek = find_next(captured_tokens, separator::seek, separator::selector_open, separator::selector_close);

                    span<token>::iterator selector_begin = a_tokens.begin() + i + 1;
                    span<token>::iterator secondary_begin = selector_seek + 1;

                    span<token> first_tokens(selector_begin, selector_seek);
                    span<token> second_tokens(selector_seek + 1, end_selector_token);

                    span<source_position> first_source_positions = a_source_positions.subspan(i + 1, first_tokens.size());
                    span<source_position> second_source_positions = a_source_positions.subspan(i + 1 + (selector_seek - selector_begin), second_tokens.size());

                    if (selector_seek != captured_tokens.cend()) {
                        nodes.emplace_back(
                            body_tokens,
                            body_source_positions,
                            node::type::ranged_selector,
                            node::ranged_selector {{
                                {
                                    first_tokens,
                                    first_source_positions,
                                    node::type::expression,
                                    parse_group(first_tokens, first_source_positions)
                                },
                                {
                                    second_tokens,
                                    second_source_positions,
                                    node::type::expression,
                                    parse_group(second_tokens, second_source_positions)
                                }
                            }}
                        );
                    } else {
                        nodes.emplace_back(
                            body_tokens,
                            body_source_positions,
                            node::type::selector,
                            parse_group(captured_tokens, captured_source_positions)
                        );
                    }
                }

                i += std::distance(end_selector_token, a_tokens.begin() + i);
            } else if (tok == separator::scope_open) {
                // Parsing immediate table.

                span<token>::iterator end_scope_token = find_next(a_tokens.subspan(i + 1), separator::scope_close, separator::scope_open, separator::scope_close);

                const auto find_next_entry = [](const span<token> a_tokens) noexcept -> span<token>::iterator {
                    return find_next(a_tokens, separator::list, [](const token& a_token) -> find_next_exclude {
                        if (a_token == separator::scope_open || a_token == separator::group_open) {
                            return find_next_exclude::open;
                        } else if (a_token == separator::scope_close || a_token == separator::group_close) {
                            return find_next_exclude::close;
                        } else {
                            return find_next_exclude::none;
                        }
                    });
                };

                const auto find_assignment = [](const span<token> a_tokens) noexcept -> span<token>::iterator {
                    return find_next(a_tokens, separator::assignment, [](const token& a_token) -> find_next_exclude {
                        if (a_token == separator::scope_open || a_token == separator::selector_open || a_token == separator::group_open) {
                            return find_next_exclude::open;
                        } else if (a_token == separator::scope_close || a_token == separator::selector_close || a_token == separator::group_close) {
                            return find_next_exclude::close;
                        } else {
                            return find_next_exclude::none;
                        }
                    });
                };

                node::immediate_table tbl;

                span<token>::iterator last_entry = a_tokens.begin() + i + 1;
                span<token>::iterator entry_end = find_next_entry(span<token>(last_entry, end_scope_token));

                do {
                    span<token> entry_tokens(last_entry, entry_end);

                    if (!entry_tokens.empty()) {
                        span<token>::iterator assignment_token = find_assignment(entry_tokens);

                        span<token> value_tokens(assignment_token + 1, entry_tokens.end());
                        span<source_position> value_source_positions = a_source_positions.subspan(assignment_token + 1 - a_tokens.begin(), value_tokens.size());

                        if (std::distance(assignment_token, last_entry) == 1) {
                            tbl.m_entries.emplace_back(
                                node {
                                    span<token>(last_entry, last_entry + 1),
                                    a_source_positions.subspan(i + 1, 1),
                                    node::type::token,
                                    &*last_entry
                                },
                                parse_group(value_tokens, value_source_positions)
                            );
                        } else if (entry_tokens[0] == separator::selector_open) {
                            span<token>::iterator key_expression_end = find_next(entry_tokens, separator::selector_close, separator::selector_open, separator::selector_close);

                            span<token> key_tokens(entry_tokens.begin() + 1, key_expression_end);
                            span<source_position> key_source_positions = a_source_positions.subspan(key_tokens.begin() - a_tokens.begin(), key_tokens.size());

                            if (key_expression_end != entry_tokens.cend()) {
                                tbl.m_entries.emplace_back(
                                    node {
                                        key_tokens,
                                        key_source_positions,
                                        node::type::expression,
                                        parse_group(key_tokens, key_source_positions)
                                    },
                                parse_group(value_tokens, value_source_positions)
                                );
                            } else {
                                // Malformed key expression for immediate table.

                                // TODO: Throw malformed immediate table entry expression.
                            }
                        }
                    }

                    last_entry = entry_end + 1;

                    if (last_entry >= end_scope_token) {
                        break;
                    }

                    entry_end = find_next_entry(span<token>(last_entry, end_scope_token));
                } while (entry_end != last_entry);

                span<token> immediate_table_tokens(a_tokens.begin() + i, end_scope_token);
                span<source_position> immediate_table_source_positions = a_source_positions.subspan(i, immediate_table_tokens.size());

                nodes.emplace_back(
                    immediate_table_tokens,
                    immediate_table_source_positions,
                    node::type::immediate_table,
                    tbl
                );

                i += std::distance(end_scope_token, a_tokens.begin() + 1);
            } else {
                nodes.emplace_back(
                    a_tokens.subspan(i, 1),
                    a_source_positions.subspan(i, 1),
                    node::type::token,
                    &tok
                );
            }
        }

        return {
            parse_ast(nodes),
            a_tokens,
            a_source_positions
        };
    }

    // Routine to parse a block.
    [[nodiscard]] node::block parse_block(const span<token> a_tokens, const span<source_position> a_source_positions) noexcept {
        std::vector<node> nodes;

        bool flag_constant = false;
        bool flag_local = false;

        for (size_t i = 0; i < a_tokens.size(); ++i) {
            const token& tok = a_tokens[i];

            if (tok == keyword::if_statement && a_tokens[i + 1] == separator::group_open) {
                // "If" parsing routine.

                span<token>::iterator group_close_find = find_next(a_tokens.subspan(i + 2), separator::group_close, separator::group_open, separator::group_close);

                // Ensure conditional is closed.
                if (group_close_find != a_tokens.cend()) {
                    span<token> conditional_tokens(a_tokens.begin() + i + 2, group_close_find);
                    span<source_position> conditional_source_positions = a_source_positions.subspan(i + 2, conditional_tokens.size());

                    if (*(group_close_find + 1) == separator::scope_open) {
                        // Regular "if" with postceding block.

                        span<token>::iterator block_end_find = find_next(span<token>(group_close_find + 2, a_tokens.end()), separator::scope_close, separator::scope_open, separator::scope_close);

                        // Ensure block is closed.
                        if (block_end_find != a_tokens.cend()) {
                            // Block bounds located.

                            span<token> body_tokens(group_close_find + 2, block_end_find);
                            span<source_position> body_source_positions(conditional_source_positions.end() + 2, conditional_source_positions.end() + 2 + body_tokens.size());

                            span<token> if_statement_tokens(a_tokens.begin() + i, block_end_find + 1);
                            span<source_position> if_statement_source_positions = a_source_positions.subspan(i, if_statement_tokens.size());

                            nodes.emplace_back(
                                if_statement_tokens,
                                if_statement_source_positions,
                                node::type::if_declaration,
                                node::if_declaration(
                                    parse_group(conditional_tokens, conditional_source_positions),
                                    parse_block(body_tokens, body_source_positions)
                                )
                            );

                            i += std::distance(block_end_find, a_tokens.begin() + i);
                        } else {
                            // Malformed block postceding "if" statement.

                            // TODO: Throw incomplete block syntax error.
                        }
                    } else {
                        // Single statement "if" with postceding statement.

                        span<token>::iterator statement_end = find_next(span<token>(group_close_find + 2, a_tokens.end()), separator::end_statement, separator::scope_open, separator::scope_close);

                        // Ensure statement has an end.
                        if (statement_end != a_tokens.cend()) {
                            // Statement bounds located.

                            span<token> statement_tokens(group_close_find + 2, statement_end);
                            span<source_position> statement_source_positions(conditional_source_positions.end() + 2, conditional_source_positions.end() + 2 + statement_tokens.size());

                            span<token> if_statement_tokens(a_tokens.begin() + i, statement_end + 1);
                            span<source_position> if_statement_source_positions = a_source_positions.subspan(i, if_statement_tokens.size());

                            nodes.emplace_back(
                                if_statement_tokens,
                                if_statement_source_positions,
                                node::type::if_declaration,
                                node::if_declaration(
                                    parse_group(conditional_tokens, conditional_source_positions),
                                    {{
                                        statement_tokens,
                                        statement_source_positions,
                                        node::type::expression,
                                        parse_group(statement_tokens, statement_source_positions)
                                    }}
                                )
                            );

                            i += std::distance(statement_end, a_tokens.begin() + i);
                        } else {
                            // Malformed statement.

                            // TODO: Throw incomplete statement syntax error.
                        }
                    }
                } else {
                    // Malformed "if" conditional.

                    // TODO: Throw incomplete "if" syntax error.
                }
            } else if (tok == keyword::else_statement) {
                // Else statement parsing routine.

                if (a_tokens[i + 1] == keyword::if_statement) {
                    // "Else if" block.

                    // Ensure there's a conditional.
                    if (a_tokens[i + 2] == separator::group_open) {
                        span<token>::iterator group_close_find = find_next(a_tokens.subspan(i + 3), separator::group_close, separator::group_open, separator::group_close);

                        // Ensure the conditional is closed.
                        if (group_close_find != a_tokens.cend()) {
                            span<token> conditional_tokens(a_tokens.begin() + i + 3, group_close_find);
                            span<source_position> conditional_source_positions = a_source_positions.subspan(i + 3, conditional_tokens.size());

                            if (*(group_close_find + 1) == separator::scope_open) {
                                // Regular "else if" with postceding block.

                                span<token>::iterator block_end_find = find_next(span<token>(group_close_find + 2, a_tokens.end()), separator::scope_close, separator::scope_open, separator::scope_close);

                                // Ensure block is closed.
                                if (block_end_find != a_tokens.cend()) {
                                    // Block bounds located.

                                    span<token> body_tokens(group_close_find + 2, block_end_find);
                                    span<source_position> body_source_positions(conditional_source_positions.end() + 2, conditional_source_positions.end() + 2 + body_tokens.size());

                                    span<token> else_statement_tokens(a_tokens.begin() + i, block_end_find + 1);
                                    span<source_position> else_statement_source_positions = a_source_positions.subspan(i, else_statement_tokens.size());

                                    nodes.emplace_back(
                                        else_statement_tokens,
                                        else_statement_source_positions,
                                        node::type::else_if_declaration,
                                        node::else_if_declaration(
                                            parse_group(conditional_tokens, conditional_source_positions),
                                            parse_block(body_tokens, body_source_positions)
                                        )
                                    );

                                    i += std::distance(block_end_find, a_tokens.begin() + i);
                                } else {
                                    // Malformed block postceding "else if" statement.

                                    // TODO: Throw incomplete block syntax error.
                                }
                            } else {
                                // Single statement "else if" with postceding statement.

                                span<token>::iterator statement_end = find_next(span<token>(group_close_find + 2, a_tokens.end()), separator::end_statement, separator::scope_open, separator::scope_close);

                                // Ensure expression has an end.
                                if (statement_end != a_tokens.cend()) {
                                    // Statement bounds located.

                                    span<token> statement_tokens(group_close_find + 2, statement_end);
                                    span<source_position> statement_source_positions(conditional_source_positions.end() + 2, conditional_source_positions.end() + 2 + statement_tokens.size());

                                    span<token> else_statement_tokens(a_tokens.begin() + i, statement_end + 1);
                                    span<source_position> else_statement_source_positions = a_source_positions.subspan(i, else_statement_tokens.size());

                                    nodes.emplace_back(
                                        else_statement_tokens,
                                        else_statement_source_positions,
                                        node::type::else_if_declaration,
                                        node::else_if_declaration(
                                        parse_group(conditional_tokens, conditional_source_positions),
                                            {{
                                                statement_tokens,
                                                statement_source_positions,
                                                node::type::expression,
                                                parse_group(statement_tokens, statement_source_positions)
                                            }}
                                        )
                                    );

                                    i += std::distance(statement_end, a_tokens.begin() + i);
                                } else {
                                    // Malformed statement.

                                    // TODO: Throw incomplete expression syntax error.
                                }
                            }

                        } else {
                            // Malformed "else if" conditional.

                            // TODO: Throw incomplete "else if" syntax error.
                        }
                    } else {
                        // Malformed "else if" conditional.

                        // TODO: Throw incomplete "else if" syntax error.
                    }
                } else if (a_tokens[i + 1] == separator::scope_open) {
                    // Regular "else" block.

                    span<token>::iterator scope_close_find = find_next(a_tokens.subspan(i + 2), separator::scope_close, separator::scope_open, separator::scope_close);

                    span<token> body_tokens(a_tokens.begin() + i + 2, scope_close_find);
                    span<source_position> body_source_positions = a_source_positions.subspan(i + 2, body_tokens.size());

                    span<token> else_statement_tokens(a_tokens.begin() + i, scope_close_find + 1);
                    span<source_position> else_statement_source_positions = a_source_positions.subspan(i, else_statement_tokens.size());

                    if (scope_close_find != a_tokens.cend()) {
                        nodes.emplace_back(
                            else_statement_tokens,
                            else_statement_source_positions,
                            node::type::else_declaration,
                            parse_block(body_tokens, body_source_positions)
                        );

                        i += std::distance(scope_close_find, a_tokens.begin() + i);
                    } else {
                        // Malformed block postceding "else" statement.

                        // TODO: Throw incomplete "else" syntax error.
                    }
                } else {
                    // Single statement "else if" with postceding statement.

                    span<token>::iterator statement_end_find = find_next(a_tokens.subspan(i + 1), separator::end_statement, separator::scope_open, separator::scope_close);

                    if (statement_end_find != a_tokens.cend()) {
                        span<token> statement_tokens(a_tokens.begin() + i + 1, statement_end_find);
                        span<source_position> statement_source_positions = a_source_positions.subspan(i + 1, statement_tokens.size());

                        span<token> else_statement_tokens(a_tokens.begin() + i, statement_end_find + 1);
                        span<source_position> else_statement_source_positions = a_source_positions.subspan(i, else_statement_tokens.size());

                        nodes.emplace_back(
                            else_statement_tokens,
                            else_statement_source_positions,
                            node::type::else_declaration,
                            parse_block(statement_tokens, statement_source_positions)
                        );

                        i += std::distance(statement_end_find, a_tokens.begin() + i);
                    } else {
                        // Malformed statement postceding "else" statement.

                        // TODO: Throw incomplete "else" syntax error.
                    }
                }
            } else if (tok == keyword::for_loop && a_tokens[i + 1] == separator::group_open) {
                // For-loop parsing routine.

                span<token>::iterator group_end = find_next(a_tokens.subspan(i + 2), separator::group_close, separator::group_open, separator::group_close);
                span<token> group_tokens(a_tokens.begin() + i + 2, group_end);
                span<source_position> group_source_positions = a_source_positions.subspan(i + 2, group_tokens.size());

                span<token>::iterator initialization_end = find_next(group_tokens, separator::end_statement, separator::scope_open, separator::scope_close);
                span<token> initialization_tokens(group_tokens.begin(), initialization_end);
                span<source_position> initialization_source_positions = group_source_positions.subspan(0, initialization_tokens.size());

                span<token>::iterator condition_end = find_next(span<token>(initialization_end + 1, group_end), separator::end_statement, separator::scope_open, separator::scope_close);
                span<token> condition_tokens(initialization_end + 1, condition_end);
                span<source_position> condition_source_positions(initialization_source_positions.end(), initialization_source_positions.end() + condition_tokens.size());

                span<token> iteration_tokens(condition_end + 1, group_end);
                span<source_position> iteration_source_positions(condition_source_positions.end(), group_source_positions.end());

                if (*(group_end + 1) == separator::scope_open) {
                    // For-loop with postceding block.

                    span<token>::iterator block_end = find_next(span<token>(group_end + 2, a_tokens.end()), separator::scope_close, separator::scope_open, separator::scope_close);
                    span<token> body_tokens(group_end + 2, block_end);
                    span<source_position> body_source_positions(group_source_positions.end() + 2, group_source_positions.end() + 2 + body_tokens.size());

                    span<token> loop_tokens(a_tokens.begin() + i, block_end + 1);
                    span<source_position> loop_source_positions = a_source_positions.subspan(i, loop_tokens.size());

                    nodes.emplace_back(
                        loop_tokens,
                        loop_source_positions,
                        node::type::for_declaration,
                        node::for_declaration(
                            parse_group(initialization_tokens, initialization_source_positions),
                            parse_group(condition_tokens, condition_source_positions),
                            parse_group(iteration_tokens, iteration_source_positions),
                            parse_block(body_tokens, body_source_positions)
                        )
                    );

                    i += std::distance(block_end, a_tokens.begin() + i);
                } else {
                    // For-loop with postceding statement.

                    span<token>::iterator statement_end = find_next(span<token>(group_end + 1, a_tokens.end()), separator::end_statement, separator::scope_open, separator::scope_close);
                    span<token> body_tokens(group_end + 1, statement_end);
                    span<source_position> body_source_positions(group_source_positions.end() + 1, group_source_positions.end() + 1 + body_tokens.size());

                    span<token> loop_tokens(a_tokens.begin() + i, statement_end + 1);
                    span<source_position> loop_source_positions = a_source_positions.subspan(i, loop_tokens.size());

                    nodes.emplace_back(
                        loop_tokens,
                        loop_source_positions,
                        node::type::for_declaration,
                        node::for_declaration(
                            parse_group(initialization_tokens, initialization_source_positions),
                            parse_group(condition_tokens, condition_source_positions),
                            parse_group(iteration_tokens, iteration_source_positions),
                            parse_block(body_tokens, body_source_positions)
                        )
                    );

                    i += std::distance(statement_end, a_tokens.begin() + i);
                }
            } else if (tok == keyword::function) {
                // Function parsing routine.

                function_tags tags = (flag_local && flag_constant)
                        ? function_tags::constant : (flag_constant
                        ? function_tags::global_constant : (flag_local
                        ? function_tags::basic : function_tags::global));

                // Includes local/constant flags.
                size_t flag_correction_offset = static_cast<size_t>(flag_constant) + static_cast<size_t>(flag_local);
                flag_local = false;
                flag_constant = false;

                span<token>::iterator group_open_find = find_next(a_tokens.subspan(i + 1), separator::group_open);

                span<token> identifier_tokens(a_tokens.begin() + i - flag_correction_offset, group_open_find);
                span<source_position> identifier_source_positions = a_source_positions.subspan(i - flag_correction_offset, identifier_tokens.size());

                span<token>::iterator group_close_find = find_next(span<token>(group_open_find + 1, a_tokens.end()), separator::group_close, separator::group_open, separator::group_open);

                span<token> argument_tokens(group_open_find + 1, group_close_find);
                span<source_position> argument_source_positions(identifier_source_positions.end() + 1, identifier_source_positions.end() + 1 + argument_tokens.size());

                const auto func_identifier = parse_group(identifier_tokens, identifier_source_positions);

                // TODO: Explicit post-refactor integrity check.

                node::argument_list func_args;

                if (func_identifier.get_operation() == separator::dot) {
                    const static token this_token{ token::type::identifier, "this" };

                    func_args.emplace_back(
                        span<token> {},
                        span<source_position> {},
                        node::type::token,
                        &this_token
                    );
                }

                // TODO: Possible optimization opportunity.
                for (const auto& arg : parse_arguments(argument_tokens, argument_source_positions)) {
                    func_args.emplace_back(arg);
                }

                if (*(group_close_find + 1) == separator::scope_open) {
                    // Block postceding function.

                    span<token>::iterator scope_end_find = find_next(span<token>(group_close_find + 2, a_tokens.end()), separator::scope_close, separator::scope_open, separator::scope_close);

                    span<token> body_tokens(group_close_find + 2, scope_end_find);
                    span<source_position> body_source_positions(argument_source_positions.end() + 2, argument_source_positions.end() + 2 + body_tokens.size());

                    span<token> function_tokens(a_tokens.begin() + i, scope_end_find + 1);
                    span<source_position> function_source_positions = a_source_positions.subspan(i, function_tokens.size());

                    nodes.emplace_back(
                        function_tokens,
                        function_source_positions,
                        node::type::function_declaration,
                        node::function_declaration(
                            func_identifier,
                            tags,
                            func_args,
                            parse_block(body_tokens, body_source_positions)
                        )
                    );

                    i += std::distance(scope_end_find, a_tokens.begin() + i);
                } else {
                    // Statement/expression postceding function.

                    span<token>::iterator end_statement_find = find_next(span<token>(group_close_find + 1, a_tokens.end()), separator::end_statement, separator::scope_open, separator::scope_close);

                    span<token> body_tokens(group_close_find + 1, end_statement_find);
                    span<source_position> body_source_positions(argument_source_positions.end() + 1, argument_source_positions.end() + 1 + body_tokens.size());

                    span<token> function_tokens(a_tokens.begin() + i, end_statement_find + 1);
                    span<source_position> function_source_positions = a_source_positions.subspan(i, function_tokens.size());

                    nodes.emplace_back(
                        function_tokens,
                        function_source_positions,
                        node::type::function_declaration,
                        node::function_declaration(
                            func_identifier,
                            tags,
                            func_args,
                            parse_block(body_tokens, body_source_positions)
                        )
                    );

                    i += std::distance(end_statement_find, a_tokens.begin() + i);
                }
            } else if (tok == keyword::while_loop) {
                // While-loop parsing routine.

                span<token>::iterator group_close_find = find_next(a_tokens.subspan(i + 2), separator::group_close, separator::group_open, separator::group_close);

                // Ensure conditional is closed.
                if (group_close_find != a_tokens.cend()) {
                    span<token> conditional_tokens(a_tokens.begin() + i + 2, group_close_find);
                    span<source_position> conditional_source_positions = a_source_positions.subspan(i + 2, conditional_tokens.size());

                    if (*(group_close_find + 1) == separator::scope_open) {
                        // Regular while-loop with postceding block.

                        span<token>::iterator block_end_find = find_next(span<token>(group_close_find + 2, a_tokens.end()), separator::scope_close, separator::scope_open, separator::scope_close);

                        // Ensure block is closed.
                        if (block_end_find != a_tokens.cend()) {
                            // Block bounds located.

                            span<token> body_tokens(group_close_find + 2, block_end_find);
                            span<source_position> body_source_positions(conditional_source_positions.end() + 2, conditional_source_positions.end() + 2 + body_tokens.size());

                            span<token> loop_tokens(a_tokens.begin() + i, block_end_find + 1);
                            span<source_position> loop_source_positions = a_source_positions.subspan(i, loop_tokens.size());

                            nodes.emplace_back(
                                loop_tokens,
                                loop_source_positions,
                                node::type::while_declaration,
                                node::while_declaration(
                                    parse_group(conditional_tokens, conditional_source_positions),
                                    parse_block(body_tokens, body_source_positions)
                                )
                            );

                            i += std::distance(block_end_find, a_tokens.begin() + i);
                        } else {
                            // Malformed block postceding while-loop.

                            // TODO: Throw incomplete block syntax error.
                        }
                    } else {
                        // Single statement while-loop with postceding statement.

                        span<token>::iterator statement_end = find_next(span<token>(group_close_find + 1, a_tokens.end()), separator::end_statement, separator::scope_open, separator::scope_close);

                        span<token> body_tokens(group_close_find + 1, statement_end);
                        span<source_position> body_source_positions(conditional_source_positions.end() + 2, conditional_source_positions.end() + 2 + body_tokens.size());

                        span<token> loop_tokens(a_tokens.begin() + i, statement_end + 1);
                        span<source_position> loop_source_positions = a_source_positions.subspan(i, loop_tokens.size());

                        // Ensure statement has an end.
                        if (statement_end != a_tokens.cend()) {
                            // Statement bounds located.

                            nodes.emplace_back(
                                loop_tokens,
                                loop_source_positions,
                                node::type::while_declaration,
                                node::while_declaration(
                                    parse_group(conditional_tokens, conditional_source_positions),
                                    {{
                                        body_tokens,
                                        body_source_positions,
                                        node::type::expression,
                                        parse_group(body_tokens, body_source_positions)
                                    }}
                                )
                            );

                            i += std::distance(statement_end, a_tokens.begin() + i);
                        } else {
                            // Malformed statement.

                            // TODO: Throw incomplete statement syntax error.
                        }
                    }
                } else {
                    // Malformed while-loop conditional.

                    // TODO: Throw incomplete while-loop syntax error.
                }
            } else if (tok == keyword::do_loop) {
                // Do-while-loop parsing routine.


            } else if (tok == keyword::switch_statement) {
                // Switch statement parsing routine.


            } else if (tok == keyword::class_declaration) {
                // Class declaration parsing routine.

                class_tags tags = flag_local ? class_tags::basic : class_tags::global;


            } else if (tok == keyword::function_return) {
                // Function return statement parsing routine.

                span<token>::iterator end_statement_find = find_next(a_tokens.subspan(i + 1), separator::end_statement, separator::scope_open, separator::scope_close);

                span<token> expression_tokens(a_tokens.begin() + i + 1, end_statement_find);
                span<source_position> expression_source_positions = a_source_positions.subspan(i + 1, expression_tokens.size());

                span<token> statement_tokens(a_tokens.begin() + i, end_statement_find + 1);
                span<source_position> statement_source_positions = a_source_positions.subspan(i, expression_tokens.size());

                nodes.emplace_back(
                    statement_tokens,
                    statement_source_positions,
                    node::type::return_statement,
                    parse_group(expression_tokens, expression_source_positions)
                );

                i += std::distance(end_statement_find, a_tokens.begin() + i);
            } else if (tok == keyword::break_statement) {
                if (a_tokens[i + 1] == separator::end_statement) {
                    nodes.emplace_back(
                        a_tokens.subspan(i, 2),
                        a_source_positions.subspan(i, 2),
                        node::type::break_statement,
                        std::nullptr_t{}
                    );
                } else {
                    // Malformed break statement.
                    // TODO: Throw malformed break statement error.
                }

                ++i;
            } else if (tok == keyword::continue_statement) {
                if (a_tokens[i + 1] == separator::end_statement) {
                    nodes.emplace_back(
                        a_tokens.subspan(i, 2),
                        a_source_positions.subspan(i, 2),
                        node::type::continue_statement,
                        std::nullptr_t{}
                    );
                } else {
                    // Malformed break statement.
                    // TODO: Throw malformed break statement error.
                }

                ++i;
            } else if (tok == keyword::local) {
                // Local definition declared.

                flag_local = true;
            } else if (tok == keyword::constant) {
                // Constant definition declared.

                flag_constant = true;
            } else if (tok == separator::scope_open) {
                // Block parsing.

                span<token>::iterator block_end = find_next(a_tokens.subspan(i + 1), separator::scope_close, separator::scope_open, separator::scope_close);

                span<token> body_tokens(a_tokens.begin() + i + 1, block_end);
                span<source_position> body_source_positions = a_source_positions.subspan(i + 1, body_tokens.size());

                span<token> scope_tokens(a_tokens.begin() + i, block_end + 1);
                span<source_position> scope_source_positions = a_source_positions.subspan(i, body_tokens.size());

                if (block_end != a_tokens.cend()) {
                    nodes.emplace_back(
                        scope_tokens,
                        scope_source_positions,
                        node::type::block,
                        parse_block(body_tokens, body_source_positions)
                    );

                    i += std::distance(block_end, a_tokens.begin() + i);
                } else {
                    // Malformed block.
                    // TODO: Throw malformed block error.
                }
            } else {
                // Statement parsing routine.

                span<token>::iterator end_statement_find = find_next(a_tokens.subspan(i), separator::end_statement, separator::scope_open, separator::scope_close);

                // Includes local/constant flags.
                size_t flag_correction_offset = static_cast<size_t>(flag_constant) + static_cast<size_t>(flag_local);
                flag_local = false;
                flag_constant = false;

                span<token> body_tokens(a_tokens.begin() + i - flag_correction_offset, end_statement_find);
                span<source_position> body_source_positions = a_source_positions.subspan(i - flag_correction_offset, body_tokens.size());

                span<token> statement_tokens(a_tokens.begin() + i - flag_correction_offset, end_statement_find + 1);
                span<source_position> statement_source_positions = a_source_positions.subspan(i - flag_correction_offset, statement_tokens.size());

                nodes.emplace_back(
                    statement_tokens,
                    statement_source_positions,
                    node::type::expression,
                    parse_group(body_tokens, body_source_positions)
                );

                i += std::distance(end_statement_find, a_tokens.begin() + i);
            }
        }

        return nodes;
    }
}

#endif //REBAR_PARSER_HPP
