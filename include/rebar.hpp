/*
Copyright (C) 2021 Max Goddard
All rights reserved.
*/

#ifndef INCLUDE_REBAR_H
#define INCLUDE_REBAR_H

#include <type_traits>
#include <utility>
#include <variant>
#include <map>
#include <string_view>
#include <vector>
#include <algorithm>

namespace rebar {
    using integer = std::conditional_t<sizeof(size_t) == 8, int64_t, int32_t>;
    using number = std::conditional_t<sizeof(size_t) == 8, double, float>;
    using enum_base = size_t;

    static_assert(sizeof(integer) == sizeof(size_t));
    static_assert(sizeof(number) == sizeof(size_t));

    enum class separator : enum_base {
        space,
        assignment,
        addition,
        addition_assignment,
        multiplication,
        multiplication_assignment,
        division,
        division_assignment,
        subtraction,
        subtraction_assignment,
        increment,
        decrement,
        group_open,
        group_close,
        selector_open,
        selector_close,
        scope_open,
        scope_close,
        equality,
        inverse_equality,
        greater,
        lesser,
        greater_equality,
        lesser_equality,
        logical_or,
        logical_and,
        logical_not,
        bitwise_or,
        bitwise_or_assignment,
        bitwise_xor,
        bitwise_xor_assignment,
        bitwise_and,
        bitwise_and_assignment,
        bitwise_not,
        shift_right,
        shift_right_assignment,
        shift_left,
        shift_left_assignment,
        exponent,
        exponent_assignment,
        modulus,
        modulus_assignment,
        seek,
        ternary,
        direct,
        dot,
        list,
        length,
        ellipsis,
        end_statement
    };

    constexpr std::string_view separator_to_string(const separator a_separator) noexcept {
        using namespace std::string_view_literals;

        constexpr std::string_view separator_strings[] {
                "SPACE"sv,
                "ASSIGNMENT"sv,
                "ADDITION"sv,
                "ADDITION ASSIGNMENT"sv,
                "MULTIPLICATION"sv,
                "MULTIPLICATION ASSIGNMENT"sv,
                "DIVISION"sv,
                "DIVISION ASSIGNMENT"sv,
                "SUBTRACTION"sv,
                "SUBTRACTION ASSIGNMENT"sv,
                "INCREMENT"sv,
                "DECREMENT"sv,
                "OPEN GROUP"sv,
                "CLOSE GROUP"sv,
                "OPEN SELECTOR"sv,
                "CLOSE SELECTOR"sv,
                "OPEN SCOPE"sv,
                "CLOSE SCOPE"sv,
                "EQUALITY"sv,
                "INVERSE EQUALITY"sv,
                "GREATER THAN"sv,
                "LESSER THAN"sv,
                "GREATER THAN OR EQUAL TO"sv,
                "LESSER THAN OR EQUAL TO"sv,
                "LOGICAL OR"sv,
                "LOGICAL AND"sv,
                "LOGICAL NOT"sv,
                "BITWISE OR"sv,
                "BITWISE OR ASSIGNMENT"sv,
                "BITWISE XOR"sv,
                "BITWISE XOR ASSIGNMENT"sv,
                "BITWISE AND"sv,
                "BITWISE AND ASSIGNMENT"sv,
                "BITWISE NOT"sv,
                "SHIFT RIGHT"sv,
                "SHIFT RIGHT ASSIGNMENT"sv,
                "SHIFT LEFT"sv,
                "SHIFT LEFT ASSIGNMENT"sv,
                "EXPONENT"sv,
                "EXPONENT ASSIGNMENT"sv,
                "MODULUS"sv,
                "MODULUS ASSIGNMENT"sv,
                "SEEK"sv,
                "TERNARY"sv,
                "DIRECT"sv,
                "DOT"sv,
                "LIST"sv,
                "LENGTH"sv,
                "ELLIPSIS"sv,
                "END STATEMENT"sv
        };

        return separator_strings[static_cast<size_t>(a_separator)];
    }

    enum class keyword : enum_base {
        global,
        for_loop,
        function,
        if_statement,
        else_statement,
        type_of,
        while_loop,
        do_loop,
        constant,
        switch_statement,
        case_branch,
        default_case,
        break_statement,
        continue_statement,
        class_declaration,
        new_object
    };

    constexpr std::string_view keyword_to_string(const keyword a_keyword) noexcept {
        using namespace std::string_view_literals;

        constexpr std::string_view keyword_strings[] {
                "GLOBAL"sv,
                "FOR"sv,
                "FUNCTION"sv,
                "IF"sv,
                "ELSE"sv,
                "TYPEOF"sv,
                "WHILE"sv,
                "DO"sv,
                "CONSTANT"sv,
                "SWITCH"sv,
                "CASE"sv,
                "DEFAULT"sv,
                "BREAK"sv,
                "CONTINUE"sv,
                "CLASS"sv,
                "NEW"sv
        };

        return keyword_strings[static_cast<size_t>(a_keyword)];
    }

    struct token {
        enum class type : enum_base {
            separator,
            keyword,
            string_literal,
            identifier,
            integer_literal,
            number_literal
        };

        using data = std::variant<separator, keyword, std::string_view, integer, number>;

        type m_type;
        data m_data;

        token() = delete;
        token(const type a_token_type, data token_data) noexcept : m_type(a_token_type), m_data(std::move(token_data)) {}

        template <typename t_type, typename... t_args>
        constexpr token(const type a_token_type, std::in_place_type_t<t_type> a_in_place_type, t_args&&... a_args) noexcept : m_type(a_token_type), m_data(a_in_place_type, std::forward<t_args>(a_args)...) {}

        token(const token& a_token) = default;
        token(token&& a_token) noexcept : m_type(a_token.m_type), m_data(std::move(a_token.m_data)) {}

        token& operator=(const token& a_token) noexcept = default;
        token& operator=(token&& a_token) noexcept = default;

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

        [[nodiscard]] constexpr std::string_view get_string_literal() const noexcept {
            return std::get<std::string_view>(m_data);
        }

        [[nodiscard]] constexpr std::string_view get_identifier() const noexcept {
            return std::get<std::string_view>(m_data);
        }

        [[nodiscard]] constexpr integer get_integer_literal() const noexcept {
            return std::get<integer>(m_data);
        }

        [[nodiscard]] constexpr number get_number_literal() const noexcept {
            return std::get<number>(m_data);
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
            }
        }

        [[nodiscard]] constexpr bool operator==(const separator rhs) const noexcept {
            return is_separator() && get_separator() == rhs;
        }

        [[nodiscard]] constexpr bool operator==(const keyword rhs) const noexcept {
            return is_keyword() && get_keyword() == rhs;
        }

        [[nodiscard]] constexpr bool operator==(const std::string_view rhs) const noexcept {
            return (is_string_literal() || is_identifier()) && get_string_literal() == rhs;
        }

        [[nodiscard]] constexpr bool operator==(const integer rhs) const noexcept {
            return is_integer_literal() && get_integer_literal() == rhs;
        }

        [[nodiscard]] constexpr bool operator==(const number rhs) const noexcept {
            return is_number_literal() && get_number_literal() == rhs;
        }
    };

    template <typename t_type>
    class optional {
        t_type* m_object;

    public:
        constexpr optional() noexcept : m_object(nullptr) {}
        constexpr explicit optional(const t_type& a_object) noexcept(noexcept(t_type(a_object))) : m_object(new t_type(a_object)) {}
        constexpr explicit optional(t_type&& a_object) noexcept(noexcept(t_type(std::move(a_object)))) : m_object(new t_type(std::move(a_object))) {}
        constexpr explicit optional(t_type* a_object) noexcept : m_object(*a_object) {}

        template <typename... t_args>
        constexpr explicit optional(std::in_place_t, t_args&&... a_args) noexcept(noexcept(t_type(std::forward<t_args>(a_args)...))) : m_object(new t_type(std::forward<t_args>(a_args)...)) {}

        constexpr optional(const optional& a_optional) noexcept(noexcept(t_type(*a_optional.m_object))) : m_object(new t_type(*a_optional.m_object)) {}
        constexpr optional(optional&& a_optional) noexcept {
                delete m_object;
                m_object = a_optional.m_object;
                a_optional.m_object = nullptr;
        }

        ~optional() {
            delete m_object;
        }

        constexpr optional& operator=(const optional& rhs) noexcept(noexcept(t_type(&rhs.m_object))) {
            delete m_object;
            m_object = rhs.m_object == nullptr ? nullptr : new t_type(*rhs.m_object);
            return *this;
        }

        constexpr optional& operator=(optional&& rhs) noexcept(noexcept(t_type(std::move(rhs.m_object)))) {
            delete m_object;
            m_object = new t_type(std::move(*rhs.m_object));
            rhs.m_object = nullptr;
            return *this;
        }

        constexpr optional& operator=(const t_type& rhs) noexcept(noexcept(t_type(rhs))) {
            delete m_object;
            m_object = new t_type(rhs);
            return *this;
        }

        constexpr optional& operator=(t_type&& rhs) noexcept(noexcept(t_type(std::move(rhs)))) {
            delete m_object;
            m_object = new t_type(std::move(rhs));
            return *this;
        }

        constexpr optional& operator=(t_type* rhs) noexcept {
            delete m_object;
            m_object = rhs;
            return *this;
        }

        [[nodiscard]] constexpr t_type* operator->() noexcept {
            return m_object;
        }

        [[nodiscard]] constexpr const t_type& operator->() const noexcept {
            return *m_object;
        }

        [[nodiscard]] constexpr t_type& operator*() noexcept {
            return *m_object;
        }

        [[nodiscard]] constexpr const t_type& operator*() const noexcept {
            return *m_object;
        }

        [[nodiscard]] constexpr t_type* raw() noexcept {
            return m_object;
        }

        [[nodiscard]] constexpr const t_type* raw() const noexcept {
            return m_object;
        }

        [[nodiscard]] constexpr t_type& get() noexcept {
            return *m_object;
        }

        [[nodiscard]] constexpr const t_type& get() const noexcept {
            return *m_object;
        }

        [[nodiscard]] constexpr bool has_value() const noexcept {
            return m_object != nullptr;
        }
    };

    template <typename t_type>
    class optional_view {
        t_type* m_object;

    public:
        constexpr optional_view() noexcept : m_object(nullptr) {}
        constexpr explicit optional_view(t_type& a_object) noexcept : m_object(&a_object) {}
        constexpr explicit optional_view(t_type* a_object) noexcept : m_object(a_object) {}

        constexpr optional_view(const optional_view& a_optional_view) noexcept = default;
        constexpr optional_view(optional_view&& a_optional_view) noexcept = default;

        constexpr optional_view& operator=(const optional_view& rhs) noexcept = default;
        constexpr optional_view& operator=(optional_view&& rhs) noexcept = default;

        constexpr optional_view& operator=(t_type& rhs) noexcept {
            m_object = &rhs;
            return *this;
        }

        constexpr optional_view& operator=(t_type* rhs) noexcept {
            m_object = rhs;
            return *this;
        }

        [[nodiscard]] constexpr t_type& operator->() noexcept {
            return *m_object;
        }

        [[nodiscard]] constexpr const t_type& operator->() const noexcept {
            return *m_object;
        }

        [[nodiscard]] constexpr t_type& operator*() noexcept {
            return *m_object;
        }

        [[nodiscard]] constexpr const t_type& operator*() const noexcept {
            return *m_object;
        }

        [[nodiscard]] constexpr t_type* raw() noexcept {
            return m_object;
        }

        [[nodiscard]] constexpr const t_type* raw() const noexcept {
            return m_object;
        }

        [[nodiscard]] constexpr t_type& get() noexcept {
            return *m_object;
        }

        [[nodiscard]] constexpr const t_type& get() const noexcept {
            return *m_object;
        }

        [[nodiscard]] constexpr bool has_value() const noexcept {
            return m_object != nullptr;
        }
    };

    template <typename t_type>
    class has_std_size_function {
        template <typename t_c, typename = decltype(std::declval<t_c>().size())>
        static std::true_type test(int) {}

        template <typename t_c>
        static std::false_type test(...) {}

    public:
        constexpr static bool value = decltype(test<t_type>(0))::value;
    };

    template <typename t_type>
    class has_std_data_function {
        template <typename t_c, typename = decltype(std::declval<t_c>().m_data())>
        static std::true_type test(int) {}

        template <typename t_c>
        static std::false_type test(...) {}

    public:
        constexpr static bool value = decltype(test<t_type>(0))::value;
    };

    template <typename t_type>
    struct span_convertible {
        static constexpr bool value = has_std_size_function<t_type>::value && has_std_data_function<t_type>::value;
    };

    template <typename t_type>
    struct span {
        struct iterator {
            using iterator_category = std::random_access_iterator_tag;
            using difference_type = std::ptrdiff_t;
            using value_type = t_type;
            using pointer = t_type*;
            using reference = t_type&;

        private:
            pointer m_ptr;

        public:
            constexpr iterator(pointer ptr) noexcept : m_ptr(ptr) {}

            constexpr iterator& operator++() noexcept {
                ++m_ptr;
                return *this;
            }

            constexpr iterator operator++(int) & noexcept {
                return iterator(m_ptr++);
            }

            constexpr iterator& operator--() noexcept {
                --m_ptr;
                return *this;
            }

            constexpr iterator operator--(int) & noexcept {
                return iterator(m_ptr--);
            }

            [[nodiscard]] constexpr reference operator*() noexcept {
                return *m_ptr;
            }

            constexpr pointer operator->() noexcept {
                return m_ptr;
            }

            constexpr pointer operator->() const noexcept {
                return m_ptr;
            }

            constexpr iterator& operator+=(difference_type rhs) noexcept {
                m_ptr += rhs;
                return *this;
            }

            constexpr iterator& operator-=(difference_type rhs) noexcept {
                m_ptr -= rhs;
                return *this;
            }

            [[nodiscard]] constexpr reference operator[](difference_type rhs) noexcept {
                return *(m_ptr + rhs);
            }

            [[nodiscard]] constexpr bool operator<(const iterator rhs) const noexcept {
                return m_ptr < rhs.m_ptr;
            }

            [[nodiscard]] constexpr bool operator>(const iterator rhs) const noexcept {
                return m_ptr > rhs.m_ptr;
            }

            [[nodiscard]] constexpr bool operator>=(const iterator rhs) const noexcept {
                return m_ptr >= rhs.m_ptr;
            }

            [[nodiscard]] constexpr bool operator<=(const iterator rhs) const noexcept {
                return m_ptr <= rhs.m_ptr;
            }

            [[nodiscard]] constexpr bool operator==(const iterator rhs) const noexcept {
                return m_ptr == rhs.m_ptr;
            }

            [[nodiscard]] constexpr bool operator!=(const iterator rhs) const noexcept {
                return m_ptr != rhs.m_ptr;
            }

            [[nodiscard]] friend constexpr iterator operator+(iterator lhs, iterator::difference_type rhs) noexcept {
                return iterator(lhs.m_ptr + rhs);
            }

            [[nodiscard]] friend constexpr iterator operator+(iterator::difference_type rhs, iterator lhs) noexcept {
                return iterator(lhs.m_ptr + rhs);
            }

            [[nodiscard]] friend constexpr iterator operator-(iterator lhs, iterator::difference_type rhs) noexcept {
                return iterator(lhs.m_ptr - rhs);
            }
        };

        using const_iterator = const iterator;

    private:
        t_type* m_data;
        size_t m_size;

    public:
        constexpr span(t_type* a_data, size_t a_size) noexcept : m_data(a_data), m_size(a_size) {}

        template <size_t c_size>
        constexpr span(t_type (&a_array)[c_size]) noexcept : m_data(a_array), m_size(c_size) {}

        template <size_t c_size>
        constexpr span(std::array<t_type, c_size> a_array) noexcept : m_data(a_array.data()), m_size(c_size) {}

        template <typename t_container, typename = std::enable_if<span_convertible<t_container>::value>>
        constexpr span(t_container& a_container) noexcept(noexcept(a_container.size() && a_container.data())) : m_data(a_container.data()), m_size(a_container.size()) {}

        constexpr span(const span& a_span) noexcept = default;
        constexpr span(span&& a_span) noexcept = default;

        [[nodiscard]] constexpr t_type* data() noexcept {
            return m_data;
        }

        [[nodiscard]] constexpr const t_type* data() const noexcept {
            return m_data;
        }

        [[nodiscard]] constexpr size_t size() const noexcept {
            return m_size;
        }

        [[nodiscard]] constexpr span<t_type> subspan(const size_t index, const size_t size = 18446744073709551615ULL) noexcept {
            return { m_data + index, std::min(size, m_size - index) };
        }

        [[nodiscard]] constexpr span<t_type> subspan(const size_t index, const size_t size = 18446744073709551615ULL) const noexcept {
            return { m_data + index, std::min(size, m_size - index) };
        }

        [[nodiscard]] constexpr iterator cbegin() const noexcept {
            return iterator(m_data);
        }

        [[nodiscard]] constexpr iterator begin() noexcept {
            return iterator(m_data);
        }

        [[nodiscard]] constexpr iterator begin() const noexcept {
            return iterator(m_data);
        }

        [[nodiscard]] constexpr iterator cend() const noexcept {
            return iterator(m_data + m_size);
        }

        [[nodiscard]] constexpr iterator end() noexcept {
            return iterator(m_data + m_size);
        }

        [[nodiscard]] constexpr iterator end() const noexcept {
            return iterator(m_data + m_size);
        }

        [[nodiscard]] t_type& operator[](const size_t index) noexcept {
            return m_data[index];
        }

        [[nodiscard]] t_type& operator[](const size_t index) const noexcept {
            return m_data[index];
        }
    };

    class symbol_map : public std::map<std::string_view, token> {
    public:
        using map_type = std::map<std::string_view, token>;
        using pair_type = std::pair<std::string_view, token>;

        symbol_map() noexcept = default;

        symbol_map(const symbol_map& a_symbol_map) noexcept = default;
        symbol_map(symbol_map&& a_symbol_map) noexcept = default;

        symbol_map& operator=(const symbol_map& a_symbol_map) noexcept = default;

        symbol_map& operator=(symbol_map&& a_symbol_map) noexcept = default;

        [[nodiscard]] optional_view<token> get(const std::string_view symbol) noexcept {
            map_type::iterator it{ find(symbol) };
            return (it != end()) ? optional_view<token>(it->second) : optional_view<token>();
        }

        [[nodiscard]] optional_view<const token> get(const std::string_view symbol) const noexcept {
            map_type::const_iterator it{ find(symbol) };
            return (it != end()) ? optional_view<const token>(it->second) : optional_view<const token>();
        }

        [[nodiscard]] optional<const std::pair<std::string_view, const token&>> next(const std::string_view text) const noexcept {
            auto selected_element_iterator = cend();
            size_t selected_symbol_length = 0;

            auto end_iterator = selected_element_iterator;
            for (auto current_iterator = begin(); current_iterator != end_iterator; ++current_iterator) {
                if (current_iterator->first == text.substr(0, current_iterator->first.size()) && current_iterator->first.size() > selected_symbol_length) {
                    selected_element_iterator = current_iterator;
                    selected_symbol_length = current_iterator->first.size();
                }
            }

            return selected_element_iterator == cend() ? optional<const std::pair<std::string_view, const token&>>()
                    : optional<const std::pair<std::string_view, const token&>>(*selected_element_iterator);
        }

        [[nodiscard]] static symbol_map get_default() noexcept {
            return symbol_map{{
                            { " ",   { token::type::separator, separator::space } },
                            { "\n",  { token::type::separator, separator::space } },
                            { "\t",  { token::type::separator, separator::space } },
                            { "=",   { token::type::separator, separator::assignment } },
                            { "*",   { token::type::separator, separator::multiplication } },
                            { "*=",  { token::type::separator, separator::multiplication_assignment } },
                            { "/",   { token::type::separator, separator::division } },
                            { "/=",  { token::type::separator, separator::division_assignment } },
                            { "+",   { token::type::separator, separator::addition } },
                            { "+=",  { token::type::separator, separator::addition_assignment } },
                            { "-",   { token::type::separator, separator::subtraction } },
                            { "-=",  { token::type::separator, separator::subtraction_assignment } },
                            { "++",  { token::type::separator, separator::increment } },
                            { "--",  { token::type::separator, separator::decrement } },
                            { "(",   { token::type::separator, separator::group_open } },
                            { ")",   { token::type::separator, separator::group_close } },
                            { "[",   { token::type::separator, separator::selector_open } },
                            { "]",   { token::type::separator, separator::selector_close } },
                            { "{",   { token::type::separator, separator::scope_open } },
                            { "}",   { token::type::separator, separator::scope_close } },
                            { "==",  { token::type::separator, separator::equality } },
                            { "!=",  { token::type::separator, separator::inverse_equality } },
                            { ">",   { token::type::separator, separator::greater } },
                            { "<",   { token::type::separator, separator::lesser } },
                            { ">=",  { token::type::separator, separator::greater_equality } },
                            { "<=",  { token::type::separator, separator::lesser_equality } },
                            { "||",  { token::type::separator, separator::logical_or } },
                            { "or",  { token::type::separator, separator::logical_or } },
                            { "&&",  { token::type::separator, separator::logical_and } },
                            { "and", { token::type::separator, separator::logical_and } },
                            { "!",   { token::type::separator, separator::logical_not } },
                            { "not", { token::type::separator, separator::logical_not } },
                            { "|",   { token::type::separator, separator::bitwise_or } },
                            { "|=",  { token::type::separator, separator::bitwise_or_assignment } },
                            { ">|",  { token::type::separator, separator::bitwise_xor } },
                            { ">|=", { token::type::separator, separator::bitwise_xor_assignment } },
                            { "&",   { token::type::separator, separator::bitwise_and } },
                            { "&=",  { token::type::separator, separator::bitwise_and_assignment } },
                            { "~",   { token::type::separator, separator::bitwise_not } },
                            { ">>",  { token::type::separator, separator::shift_right } },
                            { ">>=", { token::type::separator, separator::shift_right_assignment } },
                            { "<<",  { token::type::separator, separator::shift_left } },
                            { "<<=", { token::type::separator, separator::shift_left_assignment } },
                            { "^",   { token::type::separator, separator::exponent } },
                            { "^=",  { token::type::separator, separator::exponent_assignment } },
                            { "%",   { token::type::separator, separator::modulus } },
                            { "%=",  { token::type::separator, separator::modulus_assignment } },
                            { ":",   { token::type::separator, separator::seek } },
                            { "?",   { token::type::separator, separator::ternary } },
                            { ".",   { token::type::separator, separator::dot } },
                            { ",",   { token::type::separator, separator::list } },
                            { "->",  { token::type::separator, separator::direct } },
                            { "#",   { token::type::separator, separator::length } },
                            { "...", { token::type::separator, separator::ellipsis } },
                            { ";",   { token::type::separator, separator::end_statement } },

                            { "global",   { token::type::keyword, keyword::global } },
                            { "for",      { token::type::keyword, keyword::for_loop } },
                            { "function", { token::type::keyword, keyword::function } },
                            { "if",       { token::type::keyword, keyword::if_statement } },
                            { "else",     { token::type::keyword, keyword::else_statement } },
                            { "typeof",   { token::type::keyword, keyword::type_of } },
                            { "while",    { token::type::keyword, keyword::while_loop } },
                            { "do",       { token::type::keyword, keyword::do_loop } },
                            { "const",    { token::type::keyword, keyword::constant } },
                            { "switch",   { token::type::keyword, keyword::switch_statement } },
                            { "case",     { token::type::keyword, keyword::case_branch } },
                            { "default",  { token::type::keyword, keyword::default_case } },
                            { "break",    { token::type::keyword, keyword::break_statement } },
                            { "continue", { token::type::keyword, keyword::continue_statement } },
                            { "class",    { token::type::keyword, keyword::class_declaration } },
                            { "new",      { token::type::keyword, keyword::new_object } }
                    }};
        }
    };

    enum class function_tags : enum_base {
        basic,
        constant,
        global,
        global_constant
    };

    enum class class_tags : enum_base {
        basic,
        global
    };

    struct parse_unit {
        struct node {
            enum class type : enum_base {
                empty,
                token,
                statement,
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
                class_declaration
            };

            using statement = std::vector<node>;
            using block = std::vector<node>;
            using group = std::vector<node>;
            using selector = std::vector<node>;
            using ranged_selector = std::pair<selector, selector>;
            using argument_list = std::vector<node>;

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
                statement m_initialization;
                group m_conditional;
                statement m_iteration;
                block m_body;

                for_declaration(statement a_initialization, group a_conditional, statement a_iteration, block a_body) noexcept :
                        m_initialization(std::move(a_initialization)), m_conditional(std::move(a_conditional)),
                        m_iteration(std::move(a_iteration)), m_body(std::move(a_body)) {}

                for_declaration(const for_declaration& a_decl) = default;
                for_declaration(for_declaration&& a_decl) noexcept = default;
            };

            struct function_declaration {
                std::string_view m_identifier;
                function_tags m_tags;
                argument_list m_parameters;
                block m_body;

                function_declaration(const std::string_view a_identifier, const function_tags a_tags, argument_list a_parameters, block a_body) noexcept :
                        m_identifier(a_identifier), m_tags(a_tags), m_parameters(std::move(a_parameters)), m_body(std::move(a_body)) {}

                function_declaration(const function_declaration& a_decl) = default;
                function_declaration(function_declaration&& a_decl) noexcept = default;
            };

            using while_declaration = if_declaration;
            using do_declaration = if_declaration;

            struct switch_declaration {
                struct case_declaration {
                    bool m_ranged;
                    std::pair<group, group> m_data;

                    explicit case_declaration(group a_group) noexcept : m_ranged(false), m_data(std::move(a_group), group()) {}
                    case_declaration(group a_begin, group a_end) noexcept : m_ranged(true), m_data(std::move(a_begin), std::move(a_end)) {}

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
                class_declaration(class_declaration& a_decl) noexcept = default;
            };

            using data_type = std::variant<std::nullptr_t, const token*, std::vector<node>, ranged_selector, if_declaration, for_declaration, function_declaration, switch_declaration, class_declaration>;

            type m_type;
            data_type m_data;

            node() noexcept : m_type(type::empty), m_data(nullptr) {}
            node(const type a_type, data_type a_data) noexcept : m_type(a_type), m_data(std::move(a_data)) {}

            template <typename t_type, typename... t_args>
            node(const type a_type, std::in_place_type_t<t_type> a_in_place_type, t_args... a_args) noexcept : m_type(a_type), m_data(a_in_place_type, std::forward<t_args>(a_args)...) {}

            node(const node& a_node) = default;
            node(node&& a_node) noexcept = default;

            [[nodiscard]] bool is_empty() const noexcept {
                return m_type == type::empty;
            }

            [[nodiscard]] bool is_token() const noexcept {
                return m_type == type::token;
            }

            [[nodiscard]] bool is_statement() const noexcept {
                return m_type == type::statement;
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

            [[nodiscard]] const token& get_token() const noexcept {
                return *std::get<const token*>(m_data);
            }

            [[nodiscard]] statement& get_statement() noexcept {
                return std::get<statement>(m_data);
            }

            [[nodiscard]] const statement& get_statement() const noexcept {
                return std::get<statement>(m_data);
            }

            [[nodiscard]] block& get_block() noexcept {
                return std::get<block>(m_data);
            }

            [[nodiscard]] const block& get_block() const noexcept {
                return std::get<block>(m_data);
            }

            [[nodiscard]] group& get_group() noexcept {
                return std::get<group>(m_data);
            }

            [[nodiscard]] const group& get_group() const noexcept {
                return std::get<group>(m_data);
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

            [[nodiscard]] std::string to_string() const noexcept {
                switch (m_type) {
                case type::empty:
                    return "EMPTY";
                    break;
                case type::token: {
                    const auto& tok = get_token();

                    switch (tok.m_type) {
                    case token::type::separator:
                        return std::string("SEPARATOR: ") + std::string(separator_to_string(tok.get_separator()));
                        break;
                    case token::type::keyword:
                        return std::string("KEYWORD: ") + std::string(keyword_to_string(tok.get_keyword()));
                        break;
                    case token::type::string_literal:
                        return std::string("STRING LITERAL: ") + std::string(tok.get_string_literal());
                        break;
                    case token::type::identifier:
                        return std::string("IDENTIFIER: ") + std::string(tok.get_identifier());
                        break;
                    case token::type::integer_literal:
                        return std::string("INTEGER LITERAL: ") + std::to_string(tok.get_integer_literal());
                        break;
                    case token::type::number_literal:
                        return std::string("NUMBER LITERAL: ") + std::to_string(tok.get_number_literal());
                        break;
                    }

                    break;
                }
                case type::statement:
                    break;
                case type::block:
                    break;
                case type::group:
                    break;
                case type::selector:
                    break;
                case type::ranged_selector:
                    break;
                case type::argument_list:
                    break;
                case type::if_declaration:
                    break;
                case type::else_if_declaration:
                    break;
                case type::else_declaration:
                    break;
                case type::for_declaration:
                    break;
                case type::function_declaration:
                    break;
                case type::while_declaration:
                    break;
                case type::do_declaration:
                    break;
                case type::switch_declaration:
                    break;
                case type::class_declaration:
                    break;
                }

                return "";
            }
        };

        std::string m_plaintext;
        std::vector<token> m_tokens;
        node::block m_block;

        [[nodiscard]] std::string string_representation() const noexcept {
            std::string string;

            for (const auto& n : m_block) {
                string += n.to_string() + ",\n";
            }

            return string;
        }
    };

    class parser {
        symbol_map m_map;

    public:
        parser() noexcept : m_map(symbol_map::get_default()) {}
        explicit parser(symbol_map a_map) noexcept : m_map(std::move(a_map)) {}

        void set_symbol_map(symbol_map a_map) noexcept {
            m_map = std::move(a_map);
        }

        [[nodiscard]] std::vector<token> lex(const std::string_view a_string) {
            std::vector<token> tokens;

            bool string_mode = false;
            bool identifier_mode = true;
            bool escape_mode = false;

            size_t string_start_index = 0;
            size_t identifier_start_index = 0;

            size_t scan_index = 0;
            while (scan_index < a_string.size()) {
                char character = a_string[scan_index];

                if (string_mode) {
                    if (escape_mode) {
                        escape_mode = false;
                    } else if (character == '"') {
                        string_mode = false;
                        tokens.emplace_back(token::type::string_literal, std::in_place_type<std::string_view>, a_string.substr(string_start_index, scan_index - string_start_index));
                    }

                    escape_mode = character == '\\';
                    ++scan_index;
                } else if (character == '"') {
                    if (identifier_mode) {
                        identifier_mode = false;
                        tokens.emplace_back(token::type::identifier, std::in_place_type<std::string_view>, a_string.substr(identifier_start_index, scan_index - identifier_start_index));
                    }

                    string_mode = true;
                    string_start_index = ++scan_index;
                } else {
                    optional<const std::pair<std::string_view, const token&>> next_token = m_map.next(a_string.substr(scan_index));

                    if (next_token.has_value()) {
                        if (identifier_mode) {
                            identifier_mode = false;
                            tokens.emplace_back(token::type::identifier, std::in_place_type<std::string_view>, a_string.substr(identifier_start_index, scan_index - identifier_start_index));
                        }

                        tokens.push_back(next_token->second);
                        scan_index += next_token->first.size();
                    } else {
                        identifier_start_index = identifier_mode ? identifier_start_index : scan_index;
                        identifier_mode = true;
                        ++scan_index;
                    }
                }
            }

            if (identifier_mode) {
                tokens.emplace_back(token::type::identifier, std::in_place_type<std::string_view>, a_string.substr(identifier_start_index, scan_index - identifier_start_index));
            }

            tokens.erase(std::remove_if(tokens.begin(), tokens.end(), [](const token& a_token) noexcept -> bool {
                    return a_token == separator::space;
            }), tokens.end());

            return tokens;
        }

        [[nodiscard]] parse_unit parse(std::string a_string) noexcept {
            parse_unit unit;
            unit.m_plaintext = std::move(a_string);
            unit.m_tokens = lex(unit.m_plaintext);
            unit.m_block = parse_block(span<token>(unit.m_tokens));

            return unit;
        }

        void print_tokens(const span<token> tokens) noexcept {
            for (const auto& tok : tokens) {
                switch (tok.m_type) {
                case token::type::separator:
                    std::cout << "SEPARATOR: " << std::find_if(m_map.begin(), m_map.end(), [&tok](const symbol_map::pair_type& pair) noexcept -> bool {
                        return tok == pair.second;
                    })->first << '\n';
                    break;
                case token::type::keyword:
                    std::cout << "KEYWORD: " << std::find_if(m_map.begin(), m_map.end(), [&tok](const symbol_map::pair_type& pair) noexcept -> bool {
                        return tok == pair.second;
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

    private:
        [[nodiscard]] static parse_unit::node::ranged_selector parse_ranged_selector(const span<token> a_tokens) noexcept {
            size_t selector_increment = 0;

            for (size_t i = 0; i < a_tokens.size(); ++i) {
                const token& tok = a_tokens[i];

                if (tok == separator::selector_open) {
                    ++selector_increment;
                } else if (tok == separator::selector_close) {
                    --selector_increment;
                } else if (selector_increment == 0 && tok == separator::seek) {
                    return parse_unit::node::ranged_selector(parse_group(a_tokens.subspan(0, i - 1)), parse_group(a_tokens.subspan(i + 1)));
                }
            }
        }

        [[nodiscard]] static parse_unit::node::argument_list parse_arguments(const span<token> a_tokens) noexcept {
            std::vector<parse_unit::node> nodes;

            size_t group_increment = 0;
            size_t last_index = 0;

            for (size_t i = 0; i < a_tokens.size(); ++i) {
                const token& tok = a_tokens[i];

                if (tok == separator::group_open) {
                    ++group_increment;
                } else if (tok == separator::group_close) {
                    --group_increment;
                } else if (group_increment == 0 && tok == separator::list) {
                    if (i - last_index == 2) {
                        nodes.emplace_back(parse_unit::node::type::token, a_tokens.data() + i - 1);
                    }
                }
            }

            return nodes;
        }

        [[nodiscard]] static parse_unit::node::group parse_group(const span<token> a_tokens) noexcept {
            std::vector<parse_unit::node> nodes;

            bool group_parse = false;
            bool argument_parse = false;
            size_t group_index = 0;
            size_t group_increment = 0;

            bool selector_parse = false;
            bool ranged_selector_parse = false;
            size_t selector_index = 0;
            size_t selector_increment = 0;

            bool block_parse = false;
            size_t block_index = 0;
            size_t block_increment = 0;

            for (size_t i = 0; i < a_tokens.size(); ++i) {
                const token& tok = a_tokens[i];

                if (group_parse) {
                    if (tok == separator::group_open) {
                        ++group_increment;
                    } else if (tok == separator::group_close) {
                        --group_increment;
                    } else if (tok == separator::list && group_increment == 1) {
                        argument_parse = true;
                    }

                    if (group_increment == 0) {
                        if (argument_parse) {
                            nodes.emplace_back(parse_unit::node::type::argument_list, parse_arguments(a_tokens.subspan(group_index, i - group_index)));
                            argument_parse = false;
                        } else {
                            nodes.emplace_back(parse_unit::node::type::group, parse_group(a_tokens.subspan(group_index, i - group_index)));
                        }

                        group_parse = false;
                    }
                } else if (selector_parse) {
                    if (tok == separator::selector_open) {
                        ++selector_increment;
                    } else if (tok == separator::selector_close) {
                        --selector_increment;
                    } else if (tok == separator::seek && selector_increment == 1) {
                        ranged_selector_parse = true;
                    }

                    if (selector_increment == 0) {
                        if (ranged_selector_parse) {
                            nodes.emplace_back(parse_unit::node::type::ranged_selector, parse_ranged_selector(a_tokens.subspan(selector_index, i - selector_index)));
                            ranged_selector_parse = false;
                        } else {
                            nodes.emplace_back(parse_unit::node::type::selector, parse_group(a_tokens.subspan(selector_index, i - selector_index)));
                        }

                        selector_parse = false;
                    }
                } else if (block_parse) {
                    if (tok == separator::scope_open) {
                        ++block_increment;
                    } else if (tok == separator::scope_close) {
                        --block_increment;
                    }

                    if (block_increment == 0) {
                        nodes.emplace_back(parse_unit::node::type::block, parse_block(a_tokens.subspan(block_index, i - block_index)));
                        block_parse = false;
                    }
                } else {
                    if (tok == separator::group_open) {
                        group_parse = true;
                        group_index = i + 1;
                        ++group_increment;
                    } else if (tok == separator::selector_open) {
                        selector_parse = true;
                        selector_index = i + 1;
                        ++selector_increment;
                    } else if (tok == separator::scope_open) {
                        block_parse = true;
                        block_index = i + 1;
                        ++block_increment;
                    } else {
                        nodes.emplace_back(parse_unit::node::type::token, &tok);
                    }
                }
            }

            return nodes;
        }

        [[nodiscard]] static parse_unit::node::block parse_block(const span<token> a_tokens) noexcept {
            std::vector<parse_unit::node> nodes;

            size_t statement_begin_index = 0;

            for (size_t i = 0; i < a_tokens.size(); ++i) {
                const token& tok = a_tokens[i];

                if (tok == separator::end_statement) {
                    nodes.emplace_back(parse_unit::node::type::statement, parse_group(a_tokens.subspan(statement_begin_index, i - statement_begin_index)));
                }
            }

            return nodes;
        }
    };
}

#endif // #ifndef INCLUDE_REBAR_H
