//
// Created by maxng on 6/21/2022.
//

#ifndef REBAR_LEXER_HPP
#define REBAR_LEXER_HPP

#include <vector>
#include <variant>
#include <string>
#include <optional>
#include <map>
#include <iostream>

#include "definitions.hpp"
#include "utility.hpp"
#include "span.hpp"
#include "token.hpp"

namespace rebar {
    struct symbol_mapping {
        bool interrupter;
        token replaced;
    };

    class symbol_map : public std::map<std::string_view, symbol_mapping> {
    public:
        using map_type = std::map<std::string_view, symbol_mapping>;

        symbol_map() noexcept = default;

        [[nodiscard]] std::optional<symbol_mapping> get(const std::string_view symbol) const noexcept {
            map_type::const_iterator it{ find(symbol) };
            return (it != cend()) ? std::optional<symbol_mapping>(it->second) : std::nullopt;
        }

        [[nodiscard]] std::optional<std::pair<std::string_view, symbol_mapping>> next(const std::string_view text) const noexcept {
            auto selected_element_iterator = cend();
            size_t selected_symbol_length = 0;

            auto end_iterator = selected_element_iterator;
            for (auto current_iterator = begin(); current_iterator != end_iterator; ++current_iterator) {
                if (current_iterator->first == text.substr(0, current_iterator->first.size()) && current_iterator->first.size() > selected_symbol_length) {
                    selected_element_iterator = current_iterator;
                    selected_symbol_length = current_iterator->first.size();
                }
            }

            // TODO: Proper way to return empty optional?
            return selected_element_iterator != cend() ? std::optional<std::pair<std::string_view, symbol_mapping>>(*selected_element_iterator)
                                                       : std::nullopt;
        }

        [[maybe_unused]] void print_tokens(const span<token> tokens) noexcept {
            for (const auto& tok : tokens) {
                switch (tok.m_type) {
                    case token::type::separator:
                        std::cout << "SEPARATOR: " << std::find_if(begin(), end(), [&tok](const auto& a_pair) noexcept -> bool {
                            return tok == a_pair.second.replaced;
                        })->first << '\n';
                        break;
                    case token::type::keyword:
                        std::cout << "KEYWORD: " << std::find_if(begin(), end(), [&tok](const auto& a_pair) noexcept -> bool {
                            return tok == a_pair.second.replaced;
                        })->first << '\n';
                        break;
                    case token::type::string_literal:
                        std::cout << "STRING LITERAL: " << tok.get_string_literal() << '\n';
                        break;
                    case token::type::identifier:
                        std::cout << "IDENTIFIER: " << tok.get_identifier() << '\n';
                        break;
                    case token::type::integer_literal:
                        std::cout << "INTEGER LITERAL: " << tok.get_integer_literal() << '\n';
                        break;
                    case token::type::number_literal:
                        std::cout << "NUMBER LITERAL: " << tok.get_number_literal() << '\n';
                        break;
                }
            }
        }

        [[nodiscard]] static symbol_map get_default() noexcept {
            return symbol_map{{
                { " ",    { true,  { token::type::separator, separator::space } } },
                { "\n",   { true,  { token::type::separator, separator::space } } },
                { "\t",   { true,  { token::type::separator, separator::space } } },
                { "\r\n", { true,  { token::type::separator, separator::space } } },
                { "=",    { true,  { token::type::separator, separator::assignment } } },
                { "*",    { true,  { token::type::separator, separator::multiplication } } },
                { "*=",   { true,  { token::type::separator, separator::multiplication_assignment } } },
                { "/",    { true,  { token::type::separator, separator::division } } },
                { "/=",   { true,  { token::type::separator, separator::division_assignment } } },
                { "+",    { true,  { token::type::separator, separator::addition } } },
                { "+=",   { true,  { token::type::separator, separator::addition_assignment } } },
                { "-",    { true,  { token::type::separator, separator::subtraction } } },
                { "-=",   { true,  { token::type::separator, separator::subtraction_assignment } } },
                { "++",   { true,  { token::type::separator, separator::increment } } },
                { "--",   { true,  { token::type::separator, separator::decrement } } },
                { "(",    { true,  { token::type::separator, separator::group_open } } },
                { ")",    { true,  { token::type::separator, separator::group_close } } },
                { "[",    { true,  { token::type::separator, separator::selector_open } } },
                { "]",    { true,  { token::type::separator, separator::selector_close } } },
                { "{",    { true,  { token::type::separator, separator::scope_open } } },
                { "}",    { true,  { token::type::separator, separator::scope_close } } },
                { "==",   { true,  { token::type::separator, separator::equality } } },
                { "!=",   { true,  { token::type::separator, separator::inverse_equality } } },
                { ">",    { true,  { token::type::separator, separator::greater } } },
                { "<",    { true,  { token::type::separator, separator::lesser } } },
                { ">=",   { true,  { token::type::separator, separator::greater_equality } } },
                { "<=",   { true,  { token::type::separator, separator::lesser_equality } } },
                { "||",   { true,  { token::type::separator, separator::logical_or } } },
                { "or",   { false, { token::type::separator, separator::logical_or } } },
                { "&&",   { true,  { token::type::separator, separator::logical_and } } },
                { "and",  { false, { token::type::separator, separator::logical_and } } },
                { "!",    { true,  { token::type::separator, separator::logical_not } } },
                { "not",  { false, { token::type::separator, separator::logical_not } } },
                { "|",    { true,  { token::type::separator, separator::bitwise_or } } },
                { "|=",   { true,  { token::type::separator, separator::bitwise_or_assignment } } },
                { ">|",   { true,  { token::type::separator, separator::bitwise_xor } } },
                { ">|=",  { true,  { token::type::separator, separator::bitwise_xor_assignment } } },
                { "&",    { true,  { token::type::separator, separator::bitwise_and } } },
                { "&=",   { true,  { token::type::separator, separator::bitwise_and_assignment } } },
                { "~",    { true,  { token::type::separator, separator::bitwise_not } } },
                { ">>",   { true,  { token::type::separator, separator::shift_right } } },
                { ">>=",  { true,  { token::type::separator, separator::shift_right_assignment } } },
                { "<<",   { true,  { token::type::separator, separator::shift_left } } },
                { "<<=",  { true,  { token::type::separator, separator::shift_left_assignment } } },
                { "^",    { true,  { token::type::separator, separator::exponent } } },
                { "^=",   { true,  { token::type::separator, separator::exponent_assignment } } },
                { "%",    { true,  { token::type::separator, separator::modulus } } },
                { "%=",   { true,  { token::type::separator, separator::modulus_assignment } } },
                { ":",    { true,  { token::type::separator, separator::seek } } },
                { "?",    { true,  { token::type::separator, separator::ternary } } },
                { ".",    { true,  { token::type::separator, separator::dot } } },
                { ",",    { true,  { token::type::separator, separator::list } } },
                { "->",   { true,  { token::type::separator, separator::direct } } },
                { "#",    { true,  { token::type::separator, separator::length } } },
                { "...",  { true,  { token::type::separator, separator::ellipsis } } },
                { ";",    { true,  { token::type::separator, separator::end_statement } } },
                { "new",  { false, { token::type::separator, separator::new_object } } },
                { "::",   { true,  { token::type::separator, separator::namespace_index } } },

                { "local",    { false, { token::type::keyword, keyword::local } } },
                { "for",      { false, { token::type::keyword, keyword::for_loop } } },
                { "function", { false, { token::type::keyword, keyword::function } } },
                { "if",       { false, { token::type::keyword, keyword::if_statement } } },
                { "else",     { false, { token::type::keyword, keyword::else_statement } } },
                { "typeof",   { false, { token::type::keyword, keyword::type_of } } },
                { "while",    { false, { token::type::keyword, keyword::while_loop } } },
                { "do",       { false, { token::type::keyword, keyword::do_loop } } },
                { "const",    { false, { token::type::keyword, keyword::constant } } },
                { "switch",   { false, { token::type::keyword, keyword::switch_statement } } },
                { "case",     { false, { token::type::keyword, keyword::case_branch } } },
                { "default",  { false, { token::type::keyword, keyword::default_case } } },
                { "break",    { false, { token::type::keyword, keyword::break_statement } } },
                { "continue", { false, { token::type::keyword, keyword::continue_statement } } },
                { "class",    { false, { token::type::keyword, keyword::class_declaration } } },
                { "return",   { false, { token::type::keyword, keyword::function_return }  }},
                { "true",     { false, { token::type::keyword, keyword::literal_true }  }},
                { "false",    { false, { token::type::keyword, keyword::literal_false }  }},
                { "null",     { false, { token::type::keyword, keyword::literal_null }  }}
            }};
        }
    };

    class source_position {
        size_t m_row;
        size_t m_column;

    public:
        constexpr source_position() noexcept : m_row(0), m_column(0) {}
        constexpr source_position(const size_t a_row, const size_t a_column) : m_row(a_row), m_column(a_column) {}

        [[nodiscard]] constexpr size_t row() const noexcept {
            return m_row;
        }

        [[nodiscard]] constexpr size_t column() const noexcept {
            return m_column;
        }
    };

    class lex_unit {
        std::vector<token> m_tokens;
        std::vector<source_position> m_source_positions;

    public:
        template <typename... t_args>
        void add_token(const source_position a_source_position, const token::type a_type, t_args&&... a_args) {
            m_source_positions.push_back(a_source_position);
            m_tokens.emplace_back(a_type, std::forward<t_args>(a_args)...);
        }

        void add_token(const source_position a_source_position, const token& a_token) {
            m_source_positions.push_back(a_source_position);
            m_tokens.push_back(a_token);
        }

        void add_token(const source_position a_source_position, token&& a_token) {
            m_source_positions.push_back(a_source_position);
            m_tokens.push_back(std::move(a_token));
        }

        [[nodiscard]] size_t token_count() const noexcept {
            return m_tokens.size();
        }

        [[nodiscard]] std::vector<token>& tokens() noexcept {
            return m_tokens;
        }

        [[nodiscard]] std::vector<source_position>& source_positions() noexcept {
            return m_source_positions;
        }
    };

    class lexer {
        symbol_map m_map;

    public:
        lexer() noexcept : m_map(symbol_map::get_default()) {}
        explicit lexer(symbol_map a_map) noexcept : m_map(std::move(a_map)) {}

        [[maybe_unused]] void set_symbol_map(symbol_map a_map) noexcept {
            m_map = std::move(a_map);
        }

        [[maybe_unused]] [[nodiscard]] symbol_map& token_symbol_map() noexcept {
            return m_map;
        }

        // Lexical analysis / tokenizer.
        // TODO: Add row/column indices.
        [[nodiscard]] lex_unit lex(const std::string_view a_string) {
            lex_unit unit;

            bool string_mode = false;
            bool identifier_mode = true;
            bool escape_mode = false;
            bool line_comment_mode = false;
            bool block_comment_mode = false;

            size_t string_start_index = 0;
            size_t identifier_start_index = 0;

            // Scan through each character.
            size_t scan_index = 0;
            while (scan_index < a_string.size()) {
                char character = a_string[scan_index];

                if (line_comment_mode) {
                    if (character == '\n') {
                        line_comment_mode = false;
                    }

                    ++scan_index;
                    continue;
                } else if (block_comment_mode) {
                    if (character == '*' && a_string[scan_index + 1] == '/') {
                        block_comment_mode = false;
                        ++scan_index;
                    }

                    ++scan_index;
                    continue;
                } else if (string_mode) {
                    // Actively parsing a string. Handle escape characters, etc.

                    if (escape_mode) {
                        escape_mode = false;
                    } else if (character == '"') {
                        string_mode = false;
                        unit.add_token({ 0, 0 }, token::type::string_literal, std::in_place_type<std::string>, a_string.substr(string_start_index, scan_index - string_start_index));
                    }

                    escape_mode = character == '\\';
                    ++scan_index;
                } else if (character == '/') {
                    if (a_string[scan_index + 1] == '/') {
                        line_comment_mode = true;
                        scan_index += 2;
                    } else if (a_string[scan_index + 1] == '*') {
                        block_comment_mode = true;
                        scan_index += 2;
                    }

                    if (line_comment_mode || block_comment_mode) {
                        // Check if identifier is being parsed.
                        if (identifier_mode) {
                            std::string_view identifier_string = a_string.substr(identifier_start_index, scan_index - identifier_start_index - 2);

                            if (identifier_string.empty()) {
                                identifier_mode = false;
                                continue;
                            }

                            // Check if the "identifier" is a number.
                            if (is_number_string(identifier_string)) {
                                if (is_integer_string(identifier_string)) {
                                    unit.add_token({ 0, 0 }, token::type::integer_literal, std::stoll(std::string(identifier_string)));
                                } else {
                                    unit.add_token({ 0, 0 }, token::type::number_literal, std::stod(std::string(identifier_string)));
                                }
                            } else {
                                unit.add_token({ 0, 0 }, token::type::identifier, std::in_place_type<std::string>, identifier_string);
                            }

                            identifier_mode = false;
                        }

                        continue;
                    }
                } else if (character == '"') {
                    // Begin parsing string.

                    // Check if identifier is being parsed.
                    if (identifier_mode) {
                        std::string_view identifier_string = a_string.substr(identifier_start_index, scan_index - identifier_start_index);

                        // Check if the "identifier" is a number.
                        if (is_number_string(identifier_string)) {
                            if (is_integer_string(identifier_string)) {
                                unit.add_token({ 0, 0 }, token::type::integer_literal, std::stoll(std::string(identifier_string)));
                            } else {
                                unit.add_token({ 0, 0 }, token::type::number_literal, std::stod(std::string(identifier_string)));
                            }
                        } else {
                            unit.add_token({ 0, 0 }, token::type::identifier, std::in_place_type<std::string>, identifier_string);
                        }

                        identifier_mode = false;
                    }

                    string_mode = true;
                    string_start_index = ++scan_index;
                } else {
                    // Check if any separators (operators) are present.
                    std::optional<std::pair<std::string_view, symbol_mapping>> next_token = m_map.next(a_string.substr(scan_index));

                    if (next_token.has_value()) {
                        if (next_token->first == "-" && is_number_string(a_string.substr(scan_index + 1, 1))) {
                            identifier_start_index = scan_index;
                            identifier_mode = true;
                            ++scan_index;
                            continue;
                        }

                        // Check if identifier is being parsed.
                        if (identifier_mode) {
                            if (!next_token->second.interrupter) {
                                scan_index += next_token->first.size();
                                continue;
                            }

                            std::string_view identifier_string = a_string.substr(identifier_start_index, scan_index - identifier_start_index);

                            // Check if the "identifier" is a number.
                            if (is_number_string(identifier_string)) {
                                if (next_token->second.replaced != separator::dot) {
                                    if (is_integer_string(identifier_string)) {
                                        unit.add_token({ 0, 0 }, token::type::integer_literal, std::stoll(std::string(identifier_string)));
                                    } else {
                                        unit.add_token({ 0, 0 }, token::type::number_literal, std::stod(std::string(identifier_string)));
                                    }

                                    identifier_mode = false;
                                } else {
                                    scan_index += next_token->first.size();
                                    continue;
                                }
                            } else {
                                identifier_mode = false;

                                if (!identifier_string.empty()) {
                                    unit.add_token({ 0, 0 }, token::type::identifier, std::in_place_type<std::string>, identifier_string);
                                }
                            }
                        }

                        unit.add_token({ 0, 0 }, next_token->second.replaced);
                        scan_index += next_token->first.size();
                    } else {
                        identifier_start_index = identifier_mode ? identifier_start_index : scan_index;
                        identifier_mode = true;
                        ++scan_index;
                    }
                }
            }

            // Check for an identifier at the end of the string.
            if (identifier_mode) {
                std::string_view identifier_string = a_string.substr(identifier_start_index, scan_index - identifier_start_index);

                // Check if the "identifier" is a string.
                if (is_number_string(identifier_string)) {
                    if (is_integer_string(identifier_string)) {
                        unit.add_token({ 0, 0 }, token::type::integer_literal, std::stoll(std::string(identifier_string)));
                    } else {
                        unit.add_token({ 0, 0 }, token::type::number_literal, std::stod(std::string(identifier_string)));
                    }
                } else {
                    unit.add_token({ 0, 0 }, token::type::identifier, std::in_place_type<std::string>, identifier_string);
                }
            }

            std::vector<token>& tokens = unit.tokens();
            std::vector<source_position>& source_positions = unit.source_positions();

            // There is a better, more optimized way to do this.
            // TODO: Optimize this.
            for (int i = 0; i < tokens.size(); i++) {
                if (tokens[i] == separator::space) {
                    tokens.erase(tokens.begin() + i);
                    source_positions.erase(source_positions.begin() + i);

                    --i;
                }
            }

            return unit;
        }
    };
}

#endif //REBAR_LEXER_HPP
