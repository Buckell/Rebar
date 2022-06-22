//
// Created by maxng on 6/21/2022.
//

#ifndef REBAR_TOKEN_HPP
#define REBAR_TOKEN_HPP

namespace rebar {
    struct token {
        enum class type : enum_base {
            separator,
            keyword,
            string_literal,
            identifier,
            integer_literal,
            number_literal
        };

        using data = std::variant<separator, keyword, std::string, integer, number>;

        type m_type;
        data m_data;

        token() = delete;
        token(const type a_token_type, data a_token_data) noexcept : m_type(a_token_type), m_data(std::move(a_token_data)) {}

        template <typename t_type, typename... t_args>
        constexpr token(const type a_token_type, std::in_place_type_t<t_type> a_in_place_type, t_args&&... a_args) noexcept : m_type(a_token_type), m_data(a_in_place_type, std::forward<t_args>(a_args)...) {}

        token(const token& a_token) = default;
        token(token&& a_token) noexcept : m_type(a_token.m_type), m_data(std::move(a_token.m_data)) {}

        token& operator=(const token& a_token) noexcept = default;
        token& operator=(token&& a_token) noexcept = default;

        [[nodiscard]] constexpr type token_type() const noexcept {
            return m_type;
        }

        [[nodiscard]] constexpr bool is_separator() const noexcept {
            return m_type == type::separator;
        }

        [[nodiscard]] constexpr bool is_keyword() const noexcept {
            return m_type == type::keyword;
        }

        [[nodiscard]] constexpr bool is_string_literal() const noexcept {
            return m_type == type::string_literal;
        }

        [[nodiscard]] constexpr bool is_identifier() const noexcept {
            return m_type == type::identifier;
        }

        [[nodiscard]] constexpr bool is_integer_literal() const noexcept {
            return m_type == type::integer_literal;
        }

        [[nodiscard]] constexpr bool is_number_literal() const noexcept {
            return m_type == type::number_literal;
        }

        [[nodiscard]] constexpr separator get_separator() const noexcept {
            return std::get<separator>(m_data);
        }

        [[nodiscard]] constexpr keyword get_keyword() const noexcept {
            return std::get<keyword>(m_data);
        }

        [[nodiscard]] std::string_view get_string_literal() const noexcept {
            return std::get<std::string>(m_data);
        }

        [[nodiscard]] std::string_view get_identifier() const noexcept {
            return std::get<std::string>(m_data);
        }

        [[nodiscard]] constexpr integer get_integer_literal() const noexcept {
            return std::get<integer>(m_data);
        }

        [[nodiscard]] constexpr number get_number_literal() const noexcept {
            return std::get<number>(m_data);
        }

        [[nodiscard]] std::string to_string() const noexcept {
            switch (m_type) {
                case token::type::separator:
                    return std::string("SEPARATOR: ") + std::string(separator_to_string(get_separator())) + "; ";
                case token::type::keyword:
                    return std::string("KEYWORD: ") + std::string(keyword_to_string(get_keyword())) + "; ";
                case token::type::string_literal:
                    return std::string("STRING LITERAL: \"") + std::string(get_string_literal()) + "\"; ";
                case token::type::identifier:
                    return std::string("IDENTIFIER: \"") + std::string(get_identifier()) + "\"; ";
                case token::type::integer_literal:
                    return std::string("INTEGER LITERAL: ") + std::to_string(get_integer_literal()) + "; ";
                case token::type::number_literal:
                    return std::string("NUMBER LITERAL: ") + std::to_string(get_number_literal()) + "; ";
            }
        }

        [[nodiscard]] constexpr bool operator==(const token& rhs) const noexcept {
            if (m_type != rhs.m_type) return false;

            switch (m_type) {
                case type::separator:
                    return get_separator() == rhs.get_separator();
                case type::keyword:
                    return get_keyword() == rhs.get_keyword();
                case type::string_literal:
                    return get_string_literal() == rhs.get_string_literal();
                case type::identifier:
                    return get_identifier() == rhs.get_identifier();
                case type::integer_literal:
                    return get_integer_literal() == rhs.get_integer_literal();
                case type::number_literal:
                    return get_number_literal() == rhs.get_number_literal();
                default:
                    return false;
            }
        }

        [[nodiscard]] constexpr bool operator!=(const token& rhs) const noexcept {
            return !(*this == rhs);
        }

        [[nodiscard]] constexpr bool operator==(const separator rhs) const noexcept {
            return is_separator() && get_separator() == rhs;
        }

        [[nodiscard]] constexpr bool operator!=(const separator rhs) const noexcept {
            return !(*this == rhs);
        }

        [[nodiscard]] constexpr bool operator==(const keyword rhs) const noexcept {
            return is_keyword() && get_keyword() == rhs;
        }

        [[nodiscard]] constexpr bool operator!=(const keyword rhs) const noexcept {
            return !(*this == rhs);
        }

        [[nodiscard]] constexpr bool operator==(const std::string_view rhs) const noexcept {
            return (is_string_literal() || is_identifier()) && get_string_literal() == rhs;
        }

        [[nodiscard]] constexpr bool operator!=(const std::string_view rhs) const noexcept {
            return !(*this == rhs);
        }

        [[nodiscard]] constexpr bool operator==(const integer rhs) const noexcept {
            return is_integer_literal() && get_integer_literal() == rhs;
        }

        [[nodiscard]] constexpr bool operator!=(const integer rhs) const noexcept {
            return !(*this == rhs);
        }

        [[nodiscard]] constexpr bool operator==(const number rhs) const noexcept {
            return is_number_literal() && get_number_literal() == rhs;
        }

        [[nodiscard]] constexpr bool operator!=(const number rhs) const noexcept {
            return !(*this == rhs);
        }
    };
}

#endif //REBAR_TOKEN_HPP
