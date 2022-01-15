//
// Copyright (C) 2021, Max Goddard.
// All rights reserved.
//

#ifndef INCLUDE_REBAR_H
#define INCLUDE_REBAR_H

#pragma once

#include <type_traits>
#include <utility>
#include <variant>
#include <map>
#include <string_view>
#include <string>
#include <vector>
#include <iostream>
#include <algorithm>
#include <limits>
#include <cstring>
#include <memory>
#include <cstdlib>
#include <optional>
#include <fstream>
#include <sstream>

#include <skarupke_map.hpp>
#include <xxhash.hpp>

namespace rebar {
    // TODO: Replace reinterpret_cast usage with std::bitcast in C++20.

    using integer = std::conditional_t<sizeof(void*) == 8, int64_t, int32_t>;
    using number = std::conditional_t<sizeof(void*) == 8, double, float>;
    using enum_base = size_t;

    static_assert(sizeof(integer) == sizeof(void*));
    static_assert(sizeof(number) == sizeof(void*));

    // Operators.
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
        end_statement,
        new_object,
        namespace_index,

        // Meta separators:
        operation_prefix_increment,
        operation_postfix_increment,
        operation_prefix_decrement,
        operation_postfix_decrement,
        operation_index,
        operation_call
    };

    [[nodiscard]] constexpr std::string_view separator_to_string(const separator a_separator) noexcept {
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
                "END STATEMENT"sv,
                "NEW OBJECT"sv,
                "NAMESPACE INDEX"sv,

                // Meta separators:
                "OPERATION PREFIX INCREMENT"sv,
                "OPERATION POSTFIX INCREMENT"sv,
                "OPERATION PREFIX DECREMENT"sv,
                "OPERATION POSTFIX INCREMENT"sv,
                "OPERATION INDEX"sv,
                "OPERATION CALL"sv
        };

        return separator_strings[static_cast<size_t>(a_separator)];
    }

    struct separator_info {
        size_t m_precedence;
        bool m_single_operand;

        constexpr separator_info(const size_t a_precedence, const bool a_single_operand) noexcept : m_precedence(a_precedence), m_single_operand(a_single_operand) {}

        [[nodiscard]] constexpr size_t precedence() const noexcept {
            return m_precedence;
        }

        [[nodiscard]] constexpr bool has_single_operand() const noexcept {
            return m_single_operand;
        }
    };

    [[nodiscard]] constexpr separator_info get_separator_info(const separator a_separator) noexcept {
        constexpr separator_info separator_infos[] {
                separator_info(0,  false), // "SPACE"sv,
                separator_info(1,  false), // "ASSIGNMENT"sv,
                separator_info(5,  false), // "ADDITION"sv,
                separator_info(2,  false), // "ADDITION ASSIGNMENT"sv,
                separator_info(6,  false), // "MULTIPLICATION"sv,
                separator_info(1,  false), // "MULTIPLICATION ASSIGNMENT"sv,
                separator_info(6,  false), // "DIVISION"sv,
                separator_info(2,  false), // "DIVISION ASSIGNMENT"sv,
                separator_info(5,  false), // "SUBTRACTION"sv,
                separator_info(2,  false), // "SUBTRACTION ASSIGNMENT"sv,
                separator_info(9,  true),  // "INCREMENT"sv,
                separator_info(9,  true),  // "DECREMENT"sv,
                separator_info(10, false), // "OPEN GROUP"sv,
                separator_info(10, false), // "CLOSE GROUP"sv,
                separator_info(10, false), // "OPEN SELECTOR"sv,
                separator_info(10, false), // "CLOSE SELECTOR"sv,
                separator_info(0,  false), // "OPEN SCOPE"sv,
                separator_info(0,  false), // "CLOSE SCOPE"sv,
                separator_info(4,  false), // "EQUALITY"sv,
                separator_info(4,  false), // "INVERSE EQUALITY"sv,
                separator_info(4,  false), // "GREATER THAN"sv,
                separator_info(4,  false), // "LESSER THAN"sv,
                separator_info(4,  false), // "GREATER THAN OR EQUAL TO"sv,
                separator_info(4,  false), // "LESSER THAN OR EQUAL TO"sv,
                separator_info(3,  false), // "LOGICAL OR"sv,
                separator_info(3,  false), // "LOGICAL AND"sv,
                separator_info(9,  true),  // "LOGICAL NOT"sv,
                separator_info(7,  false), // "BITWISE OR"sv,
                separator_info(1,  false), // "BITWISE OR ASSIGNMENT"sv,
                separator_info(7,  false), // "BITWISE XOR"sv,
                separator_info(1,  false), // "BITWISE XOR ASSIGNMENT"sv,
                separator_info(7,  false), // "BITWISE AND"sv,
                separator_info(1,  false), // "BITWISE AND ASSIGNMENT"sv,
                separator_info(9,  true),  // "BITWISE NOT"sv,
                separator_info(7,  false), // "SHIFT RIGHT"sv,
                separator_info(1,  false), // "SHIFT RIGHT ASSIGNMENT"sv,
                separator_info(7,  false), // "SHIFT LEFT"sv,
                separator_info(1,  false), // "SHIFT LEFT ASSIGNMENT"sv,
                separator_info(8,  false), // "EXPONENT"sv,
                separator_info(1,  false), // "EXPONENT ASSIGNMENT"sv,
                separator_info(6,  false), // "MODULUS"sv,
                separator_info(1,  false), // "MODULUS ASSIGNMENT"sv,
                separator_info(10, false), // "SEEK"sv,
                separator_info(2,  false), // "TERNARY"sv,
                separator_info(10, false), // "DIRECT"sv,
                separator_info(10, false), // "DOT"sv,
                separator_info(0,  false), // "LIST"sv,
                separator_info(9,  true),  // "LENGTH"sv,
                separator_info(0,  false), // "ELLIPSIS"sv,
                separator_info(0,  false), // "END STATEMENT"sv,
                separator_info(9,  true), // "NEW OBJECT"sv,
                separator_info(10, false) // "NAMESPACE INDEX"sv,
        };

        return separator_infos[static_cast<size_t>(a_separator)];
    }

    enum class keyword : enum_base {
        local,
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
        function_return,
        literal_true,
        literal_false,
        literal_null
    };

    [[nodiscard]] constexpr std::string_view keyword_to_string(const keyword a_keyword) noexcept {
        using namespace std::string_view_literals;

        constexpr std::string_view keyword_strings[] {
                "LOCAL"sv,
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
                "NEW"sv,
                "FUNCTION RETURN"sv,
                "TRUE"sv,
                "FALSE"sv,
                "NULL"sv
        };

        return keyword_strings[static_cast<size_t>(a_keyword)];
    }

    // Defines attributes of a Rebar function.
    enum class function_tags : enum_base {
        basic,          // Standard local function. Accessible only within the same scope and nested scopes.
        constant,       // Constant local function. Acts like a "constexpr" function.
        global,         // Standard global function. Accessible anywhere within the same environment.
        global_constant // Constant global function.
    };

    [[nodiscard]] constexpr std::string_view function_tags_to_string(const function_tags a_function_tag) noexcept {
        using namespace std::string_view_literals;

        constexpr std::string_view function_tags_strings[] {
                "BASIC"sv,
                "CONSTANT"sv,
                "GLOBAL"sv,
                "GLOBAL CONSTANT"sv
        };

        return function_tags_strings[static_cast<size_t>(a_function_tag)];
    }

    // Defines attributes of a Rebar class.
    enum class class_tags : enum_base {
        basic, // Standard local class. Accessible only within the same scope and nested scopes.
        global // Standard global class. Accessible anywhere within the same environment.
    };

    [[nodiscard]] constexpr std::string_view class_tags_to_string(const class_tags a_class_tag) noexcept {
        using namespace std::string_view_literals;

        constexpr std::string_view class_tags_strings[] {
                "BASIC"sv,
                "GLOBAL"sv
        };

        return class_tags_strings[static_cast<size_t>(a_class_tag)];
    }

    // UTILITY FUNCTIONS

    [[nodiscard]] constexpr bool is_integer_string(const std::string_view a_string) noexcept {
        return !a_string.empty() && a_string.find_first_not_of("0123456789-+") == std::string::npos;
    }

    [[nodiscard]] constexpr bool is_number_string(const std::string_view a_string) noexcept {
        return !a_string.empty() && a_string.find_first_not_of("0123456789.-+") == std::string::npos;
    }

    [[nodiscard]] constexpr bool is_integer_string(const std::wstring_view a_string) noexcept {
        return !a_string.empty() && a_string.find_first_not_of(L"0123456789-+") == std::string::npos;
    }

    [[nodiscard]] constexpr bool is_number_string(const std::wstring_view a_string) noexcept {
        return !a_string.empty() && a_string.find_first_not_of(L"0123456789.-+") == std::string::npos;
    }

    [[nodiscard]] constexpr size_t cpu_bit_architecture() noexcept {
        if constexpr (sizeof(void*) == 8) {
            return 64;
        } else {
            return 32;
        }
    }

    // Finds the next object that is equal to a_search in an iterable container and
    // returns an iterator to that object. The last two arguments are for excluding finding
    // within those two objects. For example, if you have a vector { 'a', '(', 'b', ')', 'b' }
    // and are searching for the character 'b' but with a_open_exclude and a_close_exclude as
    // '(' and ')', respectively, then it will return an iterator to the last character 'b'
    // because the first character is surrounded by the characters '(' and ')'. Useful for
    // finding characters on the "same level" in recursive/enclosed sequences.

    // A function may be used for a_search of signature:
    // bool (const t_container::value_type&)

    // A function may be used for a_open_exclude of signature:
    // find_next_exclude (const t_container::value_type&)

    // * If a function is specified for a_open_exclude, a_close_exclude
    //   is not used and can be left unspecified.

    enum class find_next_exclude : enum_base {
        none,
        open,
        close
    };

    template <typename t_container, typename t_search, typename t_exclude_open = std::nullptr_t, typename t_exclude_close = std::nullptr_t>
    [[nodiscard]] static typename t_container::iterator find_next(const t_container& a_container, const t_search& a_search, const t_exclude_open& a_open_exclude = nullptr, const t_exclude_close& a_close_exclude = nullptr) noexcept {
        typename t_container::iterator it = a_container.begin();
        typename t_container::iterator end_it = a_container.end();

        // A "search" function has been specified.
        constexpr bool using_search_function = std::is_convertible_v<t_search, const std::function<bool (const typename t_container::value_type&)>>;

        // An "open/close exclude" function has been specified.
        constexpr bool using_exclude_function = std::is_convertible_v<t_search, const std::function<find_next_exclude (const typename t_container::value_type&)>>;

        // Standard "open/close exclude" objects have been specified.
        constexpr bool using_exclude = !using_exclude_function && !std::is_same_v<t_exclude_open, std::nullptr_t> && !std::is_same_v<t_exclude_close, std::nullptr_t>;

        size_t exclude_increment = 0;

        for (; it != end_it; ++it) {
            if constexpr (using_search_function) {
                const std::function<bool (const typename t_container::value_type&)> search_function = a_search;

                if (search_function(*it) && exclude_increment == 0) {
                    return it;
                }
            } else {
                if (*it == a_search && exclude_increment == 0) {
                    return it;
                }
            }

            if constexpr (using_exclude) {
                exclude_increment += *it == a_open_exclude;
                exclude_increment -= *it == a_close_exclude;
            } else if constexpr (using_exclude_function) {
                const std::function<find_next_exclude (const typename t_container::value_type&)> exclude_function = a_open_exclude;

                switch (exclude_function(*it)) {
                    case find_next_exclude::open:
                        exclude_increment++;
                        break;
                    case find_next_exclude::close:
                        exclude_increment--;
                        break;
                    default:
                        break;
                }
            }
        }

        return end_it;
    }

    inline void* aligned_alloc(const size_t a_alignment, const size_t a_size) {
#if defined(_MSC_VER)
        return _aligned_malloc(a_size, a_alignment);
#else
        return std::aligned_alloc(a_alignment, a_size);
#endif
        return nullptr;
    }

    [[maybe_unused]] [[nodiscard]] std::string read_file(const std::string_view a_file) {
        std::ifstream file_stream(std::string(a_file), std::ifstream::in);
        std::stringstream in_string_stream;
        in_string_stream << file_stream.rdbuf();
        return in_string_stream.str();
    }

    template <typename t_pointer>
    constexpr inline t_pointer* pointer_byte_offset(void* a_ptr, ptrdiff_t a_offset) noexcept {
        return reinterpret_cast<t_pointer*>(reinterpret_cast<uint8_t*>(a_ptr) + a_offset);
    }

    template <typename t_pointer>
    constexpr inline const t_pointer* pointer_byte_offset(const void* a_ptr, ptrdiff_t a_offset) noexcept {
        return reinterpret_cast<const t_pointer*>(reinterpret_cast<const uint8_t*>(a_ptr) + a_offset);
    }

    // END UTILITY FUNCTIONS

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
        template <typename t_c, typename = decltype(std::declval<t_c>().data())>
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
            friend struct span<t_type>;

            using iterator_category = std::random_access_iterator_tag;
            using difference_type = std::ptrdiff_t;
            using value_type = t_type;
            using pointer = const t_type*;
            using reference = const t_type&;

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

            [[nodiscard]] friend constexpr iterator::difference_type operator-(iterator lhs, iterator rhs) {
                return rhs.m_ptr - lhs.m_ptr;
            }
        };

        using value_type = t_type;
        using const_iterator = const iterator;

    private:
        const t_type* m_data;
        const size_t m_size;

    public:
        constexpr span(const t_type* a_data, const size_t a_size) noexcept : m_data(a_data), m_size(a_size) {}

        template <size_t c_size>
        constexpr span(const t_type (&a_array)[c_size]) noexcept : m_data(a_array), m_size(c_size) {}

        template <size_t c_size>
        constexpr span(const std::array<t_type, c_size> a_array) noexcept : m_data(a_array.data()), m_size(c_size) {}

        template <typename t_container, typename = std::enable_if<span_convertible<t_container>::value>>
        constexpr span(const t_container& a_container) noexcept(noexcept(a_container.size() && a_container.data())) : m_data(a_container.data()), m_size(a_container.size()) {}

        constexpr span(const iterator a_begin, const iterator a_end) : m_data(a_begin.m_ptr), m_size(std::distance(a_end, a_begin)) {}

        constexpr span(const span& a_span) noexcept = default;
        constexpr span(span&& a_span) noexcept = default;

        [[nodiscard]] constexpr const t_type* data() const noexcept {
            return m_data;
        }

        [[nodiscard]] constexpr size_t size() const noexcept {
            return m_size;
        }

        [[nodiscard]] constexpr bool empty() const noexcept {
            return m_size == 0;
        }

        [[nodiscard]] constexpr span<t_type> subspan(const size_t index, const size_t size = std::numeric_limits<size_t>::max()) const noexcept {
            return { m_data + index, std::min(size, m_size - index) };
        }

        [[nodiscard]] constexpr iterator cbegin() const noexcept {
            return iterator(m_data);
        }

        [[nodiscard]] constexpr iterator begin() const noexcept {
            return iterator(m_data);
        }

        [[nodiscard]] constexpr iterator cend() const noexcept {
            return iterator(m_data + m_size);
        }

        [[nodiscard]] constexpr iterator end() const noexcept {
            return iterator(m_data + m_size);
        }

        [[nodiscard]] constexpr iterator find(const t_type& a_object) const noexcept {
            return std::find(cbegin(), cend(), a_object);
        }

        [[nodiscard]] constexpr bool contains(const t_type& a_object) const noexcept {
            return find(a_object) != cend();
        }

        [[nodiscard]] const t_type& operator[](const size_t a_index) const noexcept {
            return m_data[a_index];
        }
    };

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

    template <typename t_comparable>
    class comparable_range {
        const t_comparable& m_lower;
        const t_comparable& m_upper;

    public:
        constexpr comparable_range(const t_comparable& a_lower, const t_comparable& a_upper) noexcept : m_lower(a_lower), m_upper(a_upper) {}
        explicit constexpr comparable_range(const t_comparable& a_single) noexcept : m_lower(a_single), m_upper(a_single) {}

        [[nodiscard]] constexpr bool contains(const comparable_range& a_range) const noexcept {
            return a_range.m_lower >= m_lower && a_range.m_upper <= m_upper;
        }

        [[nodiscard]] constexpr bool operator == (const comparable_range& rhs) const noexcept {
            return contains(rhs) || rhs.contains(*this);
        }

        [[nodiscard]] constexpr bool operator != (const comparable_range& rhs) const noexcept {
            return !(*this == rhs);
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

    struct parse_unit {
        struct node {
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
            using group = expression;
            using selector = expression;
            using ranged_selector = std::pair<expression, expression>;
            using argument_list = std::vector<expression>;
            using return_statement = expression;
            using immediate_array = std::vector<node>;

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

                for_declaration(expression a_initialization, group a_conditional, expression a_iteration, block a_body) noexcept :
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

            using data_type = std::variant<std::nullptr_t, const token*, expression, std::vector<node>, argument_list, ranged_selector, if_declaration, for_declaration, function_declaration, switch_declaration, class_declaration, immediate_table>;

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
                        return std::string{ "RANGED SELECTOR { " } + range.first.to_string() + range.second.to_string() + "}; ";
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

        std::string m_plaintext;
        lex_unit m_lex_unit;
        node::block m_block;

        parse_unit() noexcept = default;

        parse_unit(const parse_unit&) = delete;
        parse_unit(parse_unit&& a_unit) noexcept :
                m_plaintext(std::move(a_unit.m_plaintext)),
                m_lex_unit(std::move(a_unit.m_lex_unit)),
                m_block(std::move(a_unit.m_block)) {}

        parse_unit& operator=(const parse_unit&) = delete;
        parse_unit& operator=(parse_unit&& a_unit) noexcept {
            m_plaintext = std::move(a_unit.m_plaintext);
            m_lex_unit = std::move(a_unit.m_lex_unit);
            m_block = std::move(a_unit.m_block);
        }

        [[maybe_unused]] [[nodiscard]] std::string string_representation() const noexcept {
            std::string string;

            for (const auto& n : m_block) {
                string += n.to_string();
            }

            return string;
        }
    };

    class parser {
        symbol_map m_map;

    public:
        parser() noexcept : m_map(symbol_map::get_default()) {}
        explicit parser(symbol_map a_map) noexcept : m_map(std::move(a_map)) {}

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

        [[nodiscard]] parse_unit parse(std::string a_string) noexcept {
            parse_unit unit;
            unit.m_plaintext = std::move(a_string);
            unit.m_lex_unit = std::move(lex(unit.m_plaintext));
            unit.m_block = parse_block(span<token>(unit.m_lex_unit.tokens()));

            return std::move(unit);
        }

        [[maybe_unused]] void print_tokens(const span<token> tokens) noexcept {
            for (const auto& tok : tokens) {
                switch (tok.m_type) {
                    case token::type::separator:
                        std::cout << "SEPARATOR: " << std::find_if(m_map.begin(), m_map.end(), [&tok](const auto& a_pair) noexcept -> bool {
                            return tok == a_pair.second.replaced;
                        })->first << '\n';
                        break;
                    case token::type::keyword:
                        std::cout << "KEYWORD: " << std::find_if(m_map.begin(), m_map.end(), [&tok](const auto& a_pair) noexcept -> bool {
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

    private:
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

        [[nodiscard]] parse_unit::node::argument_list parse_arguments(const span<token> a_tokens) noexcept {
            std::vector<parse_unit::node::group> groups;

            span<token>::iterator last_token = a_tokens.begin();

            for (size_t i = 0; i < a_tokens.size(); ++i) {
                span<token>::iterator next_token = find_next(a_tokens.subspan(i), separator::list, separator::group_open, separator::group_close);

                if (next_token == a_tokens.end()) {
                    groups.emplace_back(parse_group(a_tokens.subspan(i)));
                    break;
                }

                span<token>::iterator::difference_type difference = std::distance(next_token, last_token);

                groups.emplace_back(parse_group(a_tokens.subspan(i, difference)));

                i += difference;

                last_token = next_token + 1;
            }

            return groups;
        }

        [[nodiscard]] parse_unit::node::abstract_syntax_tree parse_ast(const span<parse_unit::node>& a_nodes) {
            if (a_nodes.empty()) {
                return {};
            } else if (a_nodes.size() == 1) {
                return parse_unit::node::abstract_syntax_tree{ { a_nodes[0] } };
            } else if (a_nodes.size() == 2) {
                if (a_nodes[0].is_token()) {
                    const auto &first_operand = a_nodes[0].get_token();

                    if (first_operand.is_separator()) {
                        switch (first_operand.get_separator()) {
                            case separator::increment: {
                                parse_unit::node::abstract_syntax_tree ast{ separator::operation_prefix_increment };
                                ast.add_operand(a_nodes[1]);
                                return ast;
                            }
                            case separator::decrement: {
                                parse_unit::node::abstract_syntax_tree ast{ separator::operation_prefix_decrement };
                                ast.add_operand(a_nodes[1]);
                                return ast;
                            }
                            case separator::logical_not: {
                                parse_unit::node::abstract_syntax_tree ast{ separator::logical_not };
                                ast.add_operand(a_nodes[1]);
                                return ast;
                            }
                            case separator::bitwise_not: {
                                parse_unit::node::abstract_syntax_tree ast{ separator::bitwise_not };
                                ast.add_operand(a_nodes[1]);
                                return ast;
                            }
                            case separator::new_object: {
                                parse_unit::node::abstract_syntax_tree ast{ separator::new_object };
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
                                parse_unit::node::abstract_syntax_tree ast{ separator::operation_postfix_increment };
                                ast.add_operand(a_nodes[0]);
                                return ast;
                            }
                            case separator::decrement: {
                                parse_unit::node::abstract_syntax_tree ast{ separator::operation_postfix_decrement };
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
                    parse_unit::node::abstract_syntax_tree ast{ separator::operation_call };
                    ast.add_operand(a_nodes[0]);

                    for (const auto& arg : a_nodes[1].get_argument_list()) {
                        ast.add_operand({ parse_unit::node::type::expression, arg });
                    }

                    return ast;
                } else if (a_nodes[1].is_selector()) {
                    return {
                        separator::operation_index,
                        a_nodes[0],
                        { parse_unit::node::type::expression, a_nodes[1].get_selector() }
                    };
                } else if (a_nodes[1].is_ranged_selector()) {
                    parse_unit::node::abstract_syntax_tree ast{ separator::operation_index };
                    ast.add_operand(a_nodes[0]);

                    const auto& ranged = a_nodes[1].get_ranged_selector();

                    ast.add_operand({ parse_unit::node::type::expression, ranged.first });
                    ast.add_operand({ parse_unit::node::type::expression, ranged.second });

                    return ast;
                }
            }

            const auto found = std::find_if(a_nodes.begin(), a_nodes.end(), [](const parse_unit::node& a_node) noexcept -> bool {
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
                    parse_unit::node::abstract_syntax_tree ast{ separator::space };

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
                parse_unit::node::abstract_syntax_tree ast{ sep };

                if (begin_node.is_token() && begin_node.get_token() == sep) {
                    ast.add_operand(a_nodes.size() == 2 ? end_node : parse_unit::node{ parse_unit::node::type::expression, parse_ast(a_nodes.subspan(1)) });
                } else if (end_node.is_token() && end_node.get_token() == sep) {
                    ast.add_operand(a_nodes.size() == 2 ? begin_node : parse_unit::node{ parse_unit::node::type::expression, parse_ast(a_nodes.subspan(0, a_nodes.size() - 1)) });
                }

                return ast;
            }

            if ((end_node.is_group() || end_node.is_selector() || end_node.is_ranged_selector() || end_node.is_expression() || end_node.is_argument_list()) && !((a_nodes.end() - 2)->is_token() && (a_nodes.end() - 2)->get_token().is_separator())) {
                if (min_precedence >= get_separator_info(separator::group_open).precedence()) {
                    span<parse_unit::node> lhs_nodes(a_nodes.begin(), a_nodes.end() - 1);
                    parse_unit::node lhs = (lhs_nodes.size() == 1) ? lhs_nodes[0] : parse_unit::node(parse_unit::node::type::expression, parse_ast(lhs_nodes));

                    if (end_node.is_group()) {
                        return {
                            separator::operation_call,
                            lhs,
                            end_node
                        };
                    } else if (end_node.is_argument_list()) {
                        const auto& args = end_node.get_argument_list();

                        parse_unit::node::abstract_syntax_tree ast{ separator::operation_call };

                        ast.add_operand(lhs);

                        for (const auto& arg : args) {
                            ast.add_operand({ parse_unit::node::type::expression, arg });
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

                        parse_unit::node::abstract_syntax_tree ast{ separator::operation_index };

                        ast.add_operand(lhs);
                        ast.add_operand({ parse_unit::node::type::expression, ranged.first });
                        ast.add_operand({ parse_unit::node::type::expression, ranged.second });

                        return ast;
                    }
                }
            }

            span<parse_unit::node> lhs_nodes(a_nodes.begin(), min_separator_it);
            span<parse_unit::node> rhs_nodes(min_separator_it + 1, a_nodes.end());

            parse_unit::node lhs = lhs_nodes.size() == 1 ? lhs_nodes[0] : parse_unit::node{ parse_unit::node::type::expression, parse_ast(lhs_nodes) };

            if (sep == separator::ternary) {
                parse_unit::node::abstract_syntax_tree ast{ sep };

                ast.add_operand(lhs);

                span<parse_unit::node>::iterator ternary_break_find = find_next(rhs_nodes, [](const parse_unit::node& a_node) noexcept -> bool {
                    return a_node.is_token() && a_node.get_token() == separator::seek;
                }, [](const parse_unit::node& a_node) noexcept -> find_next_exclude {
                    if (a_node.is_token()) {
                        const auto& tok = a_node.get_token();

                        if (tok == separator::ternary) {
                            return find_next_exclude::open;
                        } else if (tok == separator::seek) {
                            return find_next_exclude::close;
                        }
                    }
                });

                span<parse_unit::node> ternary_lhs_nodes(min_separator_it + 1, ternary_break_find);
                span<parse_unit::node> ternary_rhs_nodes(ternary_break_find + 1, rhs_nodes.end());

                parse_unit::node lhs = ternary_lhs_nodes.size() == 1 ? ternary_lhs_nodes[0] : parse_unit::node{ parse_unit::node::type::expression, parse_ast(ternary_lhs_nodes) };
                parse_unit::node rhs = ternary_rhs_nodes.size() == 1 ? ternary_rhs_nodes[0] : parse_unit::node{ parse_unit::node::type::expression, parse_ast(ternary_rhs_nodes) };

                ast.add_operand(lhs);
                ast.add_operand(rhs);

                return ast;
            }

            parse_unit::node rhs = rhs_nodes.size() == 1 ? rhs_nodes[0] : parse_unit::node{ parse_unit::node::type::expression, parse_ast(rhs_nodes) };

            return { sep, lhs, rhs };
        }

        // Routine to parse groups.
        [[nodiscard]] parse_unit::node::group parse_group(const span<token> a_tokens) noexcept {
            std::vector<parse_unit::node> nodes;

            for (size_t i = 0; i < a_tokens.size(); ++i) {
                const token& tok = a_tokens[i];

                if (tok == separator::group_open) {
                    // Parsing group.

                    span<token>::iterator end_group = find_next(a_tokens.subspan(i + 1), separator::group_close, separator::group_open, separator::group_close);

                    span<token> captured_tokens(a_tokens.begin() + i + 1, end_group);

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
                        nodes.emplace_back(parse_unit::node::type::argument_list, parse_arguments(captured_tokens));
                    } else {
                        nodes.emplace_back(parse_unit::node::type::group, parse_group(captured_tokens));
                    }

                    i += std::distance(end_group, a_tokens.begin() + i);
                } else if (tok == separator::selector_open) {
                    // Parsing selector.

                    span<token>::iterator end_selector_token = find_next(a_tokens.subspan(i + 1), separator::selector_close, separator::selector_open, separator::selector_close);

                    span<token> captured_tokens(a_tokens.begin() + i + 1, end_selector_token);

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

                        std::vector<parse_unit::node> array;

                        span<token>::iterator last_entry = a_tokens.begin() + i + 1;
                        span<token>::iterator entry_end = find_next_entry(span<token>(last_entry, end_selector_token));

                        do {
                            span<token> entry_tokens = span<token>(last_entry, entry_end);

                            if (!entry_tokens.empty()) {
                                if (entry_tokens.size() == 1) {
                                    array.emplace_back(parse_unit::node::type::token, &entry_tokens[0]);
                                } else {
                                    array.emplace_back(parse_unit::node::type::expression, parse_group(entry_tokens));
                                }
                            }

                            last_entry = entry_end + 1;

                            if (last_entry >= end_selector_token) {
                                break;
                            }

                            entry_end = find_next_entry(span<token>(last_entry, end_selector_token));
                        } while (entry_end != last_entry);

                        nodes.emplace_back(parse_unit::node::type::immediate_array, std::move(array));
                    } else {
                        // Selector.

                        span<token>::iterator selector_seek = find_next(captured_tokens, separator::seek, separator::selector_open, separator::selector_close);

                        if (selector_seek != captured_tokens.cend()) {
                            nodes.emplace_back(parse_unit::node::type::ranged_selector, parse_unit::node::ranged_selector(
                                parse_group(span<token>(a_tokens.begin() + i + 1, selector_seek)),
                                parse_group(span<token>(selector_seek + 1, end_selector_token))
                            ));
                        } else {
                            nodes.emplace_back(parse_unit::node::type::selector, parse_group(captured_tokens));
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

                    parse_unit::node::immediate_table tbl;

                    span<token>::iterator last_entry = a_tokens.begin() + i + 1;
                    span<token>::iterator entry_end = find_next_entry(span<token>(last_entry, end_scope_token));

                    do {
                        span<token> entry_tokens = span<token>(last_entry, entry_end);

                        if (!entry_tokens.empty()) {
                            span<token>::iterator assignment_token = find_assignment(entry_tokens);

                            if (std::distance(assignment_token, last_entry) == 1) {
                                tbl.m_entries.emplace_back(
                                        parse_unit::node(parse_unit::node::type::token, &*last_entry),
                                        parse_group(span<token>(assignment_token + 1, entry_tokens.end()))
                                );
                            } else if (entry_tokens[0] == separator::selector_open) {
                                span<token>::iterator key_expression_end = find_next(entry_tokens, separator::selector_close, separator::selector_open, separator::selector_close);

                                if (key_expression_end != entry_tokens.cend()) {
                                    tbl.m_entries.emplace_back(
                                            parse_unit::node(parse_unit::node::type::expression, parse_group(span<token>(assignment_token + 1, entry_tokens.end()))),
                                            parse_group(span<token>(assignment_token + 1, entry_tokens.end()))
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

                    nodes.emplace_back(parse_unit::node::type::immediate_table, tbl);
                    i += std::distance(end_scope_token, a_tokens.begin() + 1);
                } else {
                    nodes.emplace_back(parse_unit::node::type::token, &tok);
                }
            }

            return parse_ast(nodes);
        }

        // Routine to parse a block.
        [[nodiscard]] parse_unit::node::block parse_block(const span<token> a_tokens) noexcept {
            std::vector<parse_unit::node> nodes;

            bool flag_constant = false;
            bool flag_local = false;

            for (size_t i = 0; i < a_tokens.size(); ++i) {
                const token& tok = a_tokens[i];

                if (tok == keyword::if_statement && a_tokens[i + 1] == separator::group_open) {
                    // "If" parsing routine.

                    span<token>::iterator group_close_find = find_next(a_tokens.subspan(i + 2), separator::group_close, separator::group_open, separator::group_close);

                    // Ensure conditional is closed.
                    if (group_close_find != a_tokens.cend()) {
                        span<token> conditional_tokens{ a_tokens.begin() + i + 2, group_close_find };

                        if (*(group_close_find + 1) == separator::scope_open) {
                            // Regular "if" with postceding block.

                            span<token>::iterator block_end_find = find_next(span<token>(group_close_find + 2, a_tokens.end()), separator::scope_close, separator::scope_open, separator::scope_close);

                            // Ensure block is closed.
                            if (block_end_find != a_tokens.cend()) {
                                // Block bounds located.

                                nodes.emplace_back(parse_unit::node::type::if_declaration, parse_unit::node::if_declaration(parse_group(conditional_tokens), parse_block(span<token>(group_close_find + 2, block_end_find))));

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

                                nodes.emplace_back(parse_unit::node::type::if_declaration, parse_unit::node::if_declaration(parse_group(conditional_tokens), { { parse_unit::node::type::expression, parse_group(span<token>(group_close_find + 2, statement_end)) } }));

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
                                span<token> conditional_tokens{ a_tokens.begin() + i + 3, group_close_find };

                                if (*(group_close_find + 1) == separator::scope_open) {
                                    // Regular "else if" with postceding block.

                                    span<token>::iterator block_end_find = find_next(span<token>(group_close_find + 2, a_tokens.end()), separator::scope_close, separator::scope_open, separator::scope_close);

                                    // Ensure block is closed.
                                    if (block_end_find != a_tokens.cend()) {
                                        // Block bounds located.

                                        nodes.emplace_back(parse_unit::node::type::else_if_declaration, parse_unit::node::else_if_declaration(parse_group(conditional_tokens), parse_block(span<token>(group_close_find + 2, block_end_find))));

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

                                        nodes.emplace_back(parse_unit::node::type::else_if_declaration, parse_unit::node::else_if_declaration(parse_group(conditional_tokens), { { parse_unit::node::type::expression, parse_group(span<token>(group_close_find + 2, statement_end)) } }));

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

                        if (scope_close_find != a_tokens.cend()) {
                            nodes.emplace_back(parse_unit::node::type::else_declaration, parse_block(span<token>(a_tokens.begin() + i + 2, scope_close_find)));

                            i += std::distance(scope_close_find, a_tokens.begin() + i);
                        } else {
                            // Malformed block postceding "else" statement.

                            // TODO: Throw incomplete "else" syntax error.
                        }
                    } else {
                        span<token>::iterator statement_end_find = find_next(a_tokens.subspan(i + 1), separator::end_statement, separator::scope_open, separator::scope_close);

                        if (statement_end_find != a_tokens.cend()) {
                            nodes.emplace_back(parse_unit::node::type::else_declaration, parse_block(span<token>(a_tokens.begin() + i, statement_end_find)));

                            i += std::distance(statement_end_find, a_tokens.begin() + i);
                        } else {
                            // Malformed block postceding "else" statement.

                            // TODO: Throw incomplete "else" syntax error.
                        }
                    }
                } else if (tok == keyword::for_loop && a_tokens[i + 1] == separator::group_open) {
                    // For-loop parsing routine.

                    span<token>::iterator group_end = find_next(a_tokens.subspan(i + 2), separator::group_close, separator::group_open, separator::group_close);
                    span<token> group_tokens(a_tokens.begin() + i + 2, group_end);

                    span<token>::iterator initialization_end = find_next(group_tokens, separator::end_statement, separator::scope_open, separator::scope_close);
                    span<token> initialization_tokens(group_tokens.begin(), initialization_end);

                    span<token>::iterator condition_end = find_next(span<token>(initialization_end + 1, group_end), separator::end_statement, separator::scope_open, separator::scope_close);
                    span<token> condition_tokens(initialization_end + 1, condition_end);

                    span<token> iteration_tokens(condition_end + 1, group_end);

                    if (*(group_end + 1) == separator::scope_open) {
                        // For-loop with postceding block.

                        span<token>::iterator block_end = find_next(span<token>(group_end + 2, a_tokens.end()), separator::scope_close, separator::scope_open, separator::scope_close);
                        span<token> body_tokens(group_end + 2, block_end);

                        nodes.emplace_back(parse_unit::node::type::for_declaration, parse_unit::node::for_declaration(
                            parse_group(initialization_tokens),
                            parse_group(condition_tokens),
                            parse_group(iteration_tokens),
                            parse_block(body_tokens)
                        ));

                        i += std::distance(block_end, a_tokens.begin() + i);
                    } else {
                        // For-loop with postceding statement.

                        span<token>::iterator statement_end = find_next(span<token>(group_end + 1, a_tokens.end()), separator::end_statement, separator::scope_open, separator::scope_close);
                        span<token> body_tokens(group_end + 1, statement_end);

                        nodes.emplace_back(parse_unit::node::type::for_declaration, parse_unit::node::for_declaration(
                            parse_group(initialization_tokens),
                            parse_group(condition_tokens),
                            parse_group(iteration_tokens),
                            parse_block(body_tokens)
                        ));

                        i += std::distance(statement_end, a_tokens.begin() + i);
                    }
                } else if (tok == keyword::function) {
                    // Function parsing routine.

                    function_tags tags = (flag_local && flag_constant) ? function_tags::constant
                                        : (flag_constant ? function_tags::global_constant
                                        : (flag_local ? function_tags::basic
                                        : function_tags::global));

                    // Includes local/constant flags.
                    size_t flag_correction_offset = static_cast<size_t>(flag_constant) + static_cast<size_t>(flag_local);
                    flag_local = false;
                    flag_constant = false;

                    span<token>::iterator group_open_find = find_next(a_tokens.subspan(i + 1), separator::group_open);

                    span<token> identifier_tokens(a_tokens.begin() + i - flag_correction_offset, group_open_find);

                    span<token>::iterator group_close_find = find_next(span<token>(group_open_find + 1, a_tokens.end()), separator::group_close, separator::group_open, separator::group_open);

                    span<token> argument_tokens(group_open_find + 1, group_close_find);

                    const auto func_identifier = parse_group(identifier_tokens);
                    auto func_args = parse_arguments(argument_tokens);

                    if (func_identifier.get_operation() == separator::dot) {
                        const static token this_token{ token::type::identifier, "this" };

                        parse_unit::node::abstract_syntax_tree ast{ separator::space };

                        ast.add_operand({ parse_unit::node::type::token, &this_token });

                        func_args.insert(func_args.begin(), ast);
                    }

                    if (*(group_close_find + 1) == separator::scope_open) {
                        // Block postceding function.

                        span<token>::iterator scope_end_find = find_next(span<token>(group_close_find + 2, a_tokens.end()), separator::scope_close, separator::scope_open, separator::scope_close);

                        nodes.emplace_back(parse_unit::node::type::function_declaration, parse_unit::node::function_declaration(func_identifier, tags, func_args, parse_block(span<token>(group_close_find + 2, scope_end_find))));

                        i += std::distance(scope_end_find, a_tokens.begin() + i);
                    } else {
                        // Statement/expression postceding function.

                        span<token>::iterator end_statement_find = find_next(span<token>(group_close_find + 1, a_tokens.end()), separator::end_statement, separator::scope_open, separator::scope_close);

                        nodes.emplace_back(parse_unit::node::type::function_declaration, parse_unit::node::function_declaration(func_identifier, tags, func_args, parse_block(span<token>(group_close_find + 2, end_statement_find))));

                        i += std::distance(end_statement_find, a_tokens.begin() + i);
                    }
                } else if (tok == keyword::while_loop) {
                    // While-loop parsing routine.

                    span<token>::iterator group_close_find = find_next(a_tokens.subspan(i + 2), separator::group_close, separator::group_open, separator::group_close);

                    // Ensure conditional is closed.
                    if (group_close_find != a_tokens.cend()) {
                        span<token> conditional_tokens{ a_tokens.begin() + i + 2, group_close_find };

                        if (*(group_close_find + 1) == separator::scope_open) {
                            // Regular while-loop with postceding block.

                            span<token>::iterator block_end_find = find_next(span<token>(group_close_find + 2, a_tokens.end()), separator::scope_close, separator::scope_open, separator::scope_close);

                            // Ensure block is closed.
                            if (block_end_find != a_tokens.cend()) {
                                // Block bounds located.

                                nodes.emplace_back(parse_unit::node::type::while_declaration, parse_unit::node::while_declaration(parse_group(conditional_tokens), parse_block(span<token>(group_close_find + 2, block_end_find))));

                                i += std::distance(block_end_find, a_tokens.begin() + i);
                            } else {
                                // Malformed block postceding while-loop.

                                // TODO: Throw incomplete block syntax error.
                            }
                        } else {
                            // Single statement while-loop with postceding statement.

                            span<token>::iterator statement_end = find_next(span<token>(group_close_find + 2, a_tokens.end()), separator::end_statement, separator::scope_open, separator::scope_close);

                            // Ensure statement has an end.
                            if (statement_end != a_tokens.cend()) {
                                // Statement bounds located.

                                nodes.emplace_back(parse_unit::node::type::while_declaration, parse_unit::node::while_declaration(parse_group(conditional_tokens), { { parse_unit::node::type::expression, parse_group(span<token>(group_close_find + 2, statement_end)) } }));

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

                    nodes.emplace_back(parse_unit::node::type::return_statement, parse_group(span<token>(a_tokens.begin() + i + 1, end_statement_find)));

                    i += std::distance(end_statement_find, a_tokens.begin() + i);
                } else if (tok == keyword::break_statement) {
                    if (a_tokens[i + 1] == separator::end_statement) {
                        nodes.emplace_back(parse_unit::node::type::break_statement, std::nullptr_t{});
                    } else {
                        // Malformed break statement.
                        // TODO: Throw malformed break statement error.
                    }
                } else if (tok == keyword::continue_statement) {
                    if (a_tokens[i + 1] == separator::end_statement) {
                        nodes.emplace_back(parse_unit::node::type::continue_statement, std::nullptr_t{});
                    } else {
                        // Malformed break statement.
                        // TODO: Throw malformed break statement error.
                    }
                } else if (tok == keyword::local) {
                    // Local definition declared.

                    flag_local = true;
                } else if (tok == keyword::constant) {
                    // Constant definition declared.

                    flag_constant = true;
                } else if (tok == separator::scope_open) {
                    // Block parsing.

                    span<token>::iterator block_end = find_next(a_tokens.subspan(i + 1), separator::scope_close, separator::scope_open, separator::scope_close);

                    if (block_end != a_tokens.cend()) {
                        nodes.emplace_back(parse_unit::node::type::block, parse_block(span<token>(a_tokens.begin() + i + 1, block_end)));
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

                    nodes.emplace_back(parse_unit::node::type::expression, parse_group(span<token>(a_tokens.begin() + i - flag_correction_offset, end_statement_find)));

                    i += std::distance(end_statement_find, a_tokens.begin() + i);
                }
            }

            return nodes;
        }
    };

    class environment;

    class string {
        // Stores size (size_t), reference count (size_t),
        // and string contents (char[]) in contiguous block of memory.
        //
        // Similar to Pascal strings. Optimized for Rebar object storage.

        // [size_t size][size_t reference count][char[] data]

        void* m_root_pointer;

    public:
        explicit string(void* a_root_pointer) noexcept : m_root_pointer(a_root_pointer) {
            reference();
        }

        string(const string& a_string) noexcept : m_root_pointer(a_string.m_root_pointer) {
            reference();
        }

        string(environment& a_env, const std::string_view a_string);

        ~string() noexcept {
            dereference();
        }

        [[nodiscard]] const char* c_str() const noexcept {
            return reinterpret_cast<char*>(m_root_pointer) + (sizeof(size_t) * 2);
        }

        [[nodiscard]] size_t length() const noexcept {
            return reinterpret_cast<size_t*>(m_root_pointer)[0];
        }

        [[nodiscard]] size_t reference_count() const noexcept {
            return reinterpret_cast<size_t*>(m_root_pointer)[1];
        }

        [[nodiscard]] size_t size() const noexcept {
            return length() + sizeof(size_t);
        }

        [[nodiscard]] const void* data() const noexcept {
            return m_root_pointer;
        }

        [[nodiscard]] std::string_view to_string_view() const noexcept {
            return { c_str(), length() };
        }

        [[nodiscard]] bool operator==(const string& rhs) const noexcept {
            return m_root_pointer == rhs.m_root_pointer;
        }

        [[nodiscard]] bool operator!=(const string& rhs) const noexcept {
            return m_root_pointer != rhs.m_root_pointer;
        }

        friend std::ostream& operator<<(std::ostream& lhs, const string rhs) {
            return (lhs << rhs.to_string_view());
        }

    private:
        explicit string(const std::string_view a_string) noexcept : m_root_pointer(reinterpret_cast<size_t*>(std::malloc((sizeof(size_t) * 2) + a_string.size() + 1))) {
            auto* root_pointer = reinterpret_cast<size_t*>(m_root_pointer);

            // String size/length.
            root_pointer[0] = a_string.length();

            // String reference counter.
            root_pointer[1] = 1;

            // String data.
            memcpy(reinterpret_cast<char*>(m_root_pointer) + (sizeof(size_t) * 2), a_string.data(), a_string.size());
            *(reinterpret_cast<char*>(m_root_pointer) + a_string.size() + (sizeof(size_t) * 2)) = 0;
        }

        void deallocate() {
            std::free(m_root_pointer);
            m_root_pointer = nullptr;
        }

        void set_reference_count(const size_t a_count) noexcept {
            reinterpret_cast<size_t*>(m_root_pointer)[1] = a_count;
        }

        size_t reference(const size_t a_increment = 1) noexcept {
            return reinterpret_cast<size_t*>(m_root_pointer)[1] += a_increment;
        }

        size_t dereference(const size_t a_decrement = 1) noexcept {
            size_t ref_count = (reinterpret_cast<size_t*>(m_root_pointer)[1] -= a_decrement);

            if (ref_count == 0) {
                deallocate();
            }

            return ref_count;
        }

        friend class environment;

        friend struct object;
    };

    struct table;

    class function {
        friend class object;

        environment& m_environment;
        const void* m_data;

    public:
        function(environment& a_environment, const void* a_data) noexcept : m_environment(a_environment), m_data(a_data) {}

        template <typename... t_objects>
        object call(t_objects&&... a_objects);
        object call(const span<object> a_objects);

        template <typename... t_objects>
        object operator () (t_objects&&... a_objects) {
            if constexpr (sizeof...(a_objects) == 0) {
                return call();
            } else {
                return call(std::forward<t_objects>(a_objects)...);
            }
        }
    };

    struct virtual_table;

    template <typename t_object>
    struct structured_native_object {
        using destructor_function = void (*)(void*);

        size_t m_reference_count;
        virtual_table* m_virtual_table;
        destructor_function m_destructor;
        size_t m_data_size;
        t_object m_data;
    };

    class native_object {
        using destructor_function = void (*)(void*);

        void* m_root_pointer;

        // 0 00  size_t m_ref_count;
        // 1 08 virtual_table* m_v_table;
        // 2 16 destructor_function m_destructor;
        // 3 24 size_t m_data_size;
        // 4 32 uint8_t m_data[m_data_size];

    public:
        native_object(void* a_data) noexcept : m_root_pointer(a_data) {
            reference();
        }

        native_object(const native_object& a_native_object) noexcept : m_root_pointer(a_native_object.m_root_pointer) {
            reference();
        }

        native_object(native_object&& a_native_object) noexcept : m_root_pointer(a_native_object.m_root_pointer) {
            reference();
        }

        ~native_object() noexcept {
            if (m_root_pointer != nullptr) {
                dereference();
            }
        }

        native_object& operator = (const native_object& a_native_object) noexcept {
            dereference();
            m_root_pointer = a_native_object.m_root_pointer;
            reference();

            return *this;
        }

        native_object& operator = (native_object&& a_native_object) noexcept {
            dereference();

            m_root_pointer = a_native_object.m_root_pointer;
            a_native_object.m_root_pointer = nullptr;

            return *this;
        }

        template <typename t_object>
        static native_object create(virtual_table& a_v_table, t_object a_object) {
            destructor_function destructor = nullptr;

            if constexpr (!std::is_trivially_destructible_v<t_object>) {
                destructor = [](void* a_obj) noexcept(std::is_nothrow_destructible_v<t_object>) {
                    reinterpret_cast<t_object*>(a_obj)->~t_object();
                };
            }

            structured_native_object<t_object>* obj = reinterpret_cast<structured_native_object<t_object>*>(std::malloc(sizeof(structured_native_object<t_object>)));

            obj->m_reference_count = 0;          // Reference count. Initialize to 0. "Real" lifetime begins when native_object is constructed and returned.
            obj->m_virtual_table = &a_v_table;   // Virtual table.
            obj->m_destructor = destructor;      // Destructor for garbage collection.
            obj->m_data_size = sizeof(t_object); // Data size (size of object in bytes).

            new (&obj->m_data) t_object(std::move(a_object)); // Data (object).

            return { reinterpret_cast<void*>(obj) };
        }

        template <typename t_object, typename... t_args>
        static native_object create(virtual_table& a_v_table, std::in_place_type_t<t_object>, t_args... a_args) {
            destructor_function destructor = nullptr;

            if constexpr (!std::is_trivially_destructible_v<t_object>) {
                destructor = [](void* a_obj){
                    reinterpret_cast<t_object*>(a_obj)->~t_object();
                };
            }

            structured_native_object<t_object>* obj = reinterpret_cast<structured_native_object<t_object>*>(std::malloc(sizeof(structured_native_object<t_object>)));

            obj->m_reference_count = 0;          // Reference count. Initialize to 0. "Real" lifetime begins when native_object is constructed and returned.
            obj->m_virtual_table = &a_v_table;   // Virtual table.
            obj->m_destructor = destructor;      // Destructor for garbage collection.
            obj->m_data_size = sizeof(t_object); // Data size (size of object in bytes).

            new (&obj->m_data) t_object(std::forward<t_args>(a_args)...); // Data (object).

            return { reinterpret_cast<void*>(obj) };
        }

        [[nodiscard]] constexpr void* data() noexcept {
            return m_root_pointer;
        }

        [[nodiscard]] constexpr const void* data() const noexcept {
            return m_root_pointer;
        }

        [[nodiscard]] inline size_t get_reference_count() const noexcept {
            return reinterpret_cast<structured_native_object<std::nullptr_t>*>(m_root_pointer)->m_reference_count;
        }

        [[nodiscard]] inline virtual_table& get_virtual_table() noexcept {
            return *reinterpret_cast<structured_native_object<std::nullptr_t>*>(m_root_pointer)->m_virtual_table;
        }

        [[nodiscard]] inline destructor_function get_destructor() const noexcept {
            return reinterpret_cast<structured_native_object<std::nullptr_t>*>(m_root_pointer)->m_destructor;
        }

        [[nodiscard]] inline size_t get_object_size() const noexcept {
            return reinterpret_cast<structured_native_object<std::nullptr_t>*>(m_root_pointer)->m_data_size;
        }

        template <typename t_object>
        [[nodiscard]] inline t_object& get_object() const noexcept {
            return reinterpret_cast<structured_native_object<t_object>*>(m_root_pointer)->m_data;
        }

                      inline void    overload_assignment                  (environment& a_env, const object rhs);
        [[nodiscard]] inline object  overload_addition                    (environment& a_env, const object rhs);
                      inline void    overload_addition_assignment         (environment& a_env, const object rhs);
        [[nodiscard]] inline object  overload_multiplication              (environment& a_env, const object rhs);
                      inline void    overload_multiplication_assignment   (environment& a_env, const object rhs);
        [[nodiscard]] inline object  overload_division                    (environment& a_env, const object rhs);
                      inline void    overload_division_assignment         (environment& a_env, const object rhs);
        [[nodiscard]] inline object  overload_subtraction                 (environment& a_env, const object rhs);
                      inline void    overload_subtraction_assignment      (environment& a_env, const object rhs);
        [[nodiscard]] inline object  overload_equality                    (environment& a_env, const object rhs);
        [[nodiscard]] inline object  overload_inverse_equality            (environment& a_env, const object rhs);
        [[nodiscard]] inline object  overload_greater                     (environment& a_env, const object rhs);
        [[nodiscard]] inline object  overload_lesser                      (environment& a_env, const object rhs);
        [[nodiscard]] inline object  overload_greater_equality            (environment& a_env, const object rhs);
        [[nodiscard]] inline object  overload_lesser_equality             (environment& a_env, const object rhs);
        [[nodiscard]] inline object  overload_logical_or                  (environment& a_env, const object rhs);
        [[nodiscard]] inline object  overload_logical_and                 (environment& a_env, const object rhs);
        [[nodiscard]] inline object  overload_logical_not                 (environment& a_env);
        [[nodiscard]] inline object  overload_bitwise_or                  (environment& a_env, const object rhs);
                      inline void    overload_bitwise_or_assignment       (environment& a_env, const object rhs);
        [[nodiscard]] inline object  overload_bitwise_xor                 (environment& a_env, const object rhs);
                      inline void    overload_bitwise_xor_assignment      (environment& a_env, const object rhs);
        [[nodiscard]] inline object  overload_bitwise_and                 (environment& a_env, const object rhs);
                      inline void    overload_bitwise_and_assignment      (environment& a_env, const object rhs);
        [[nodiscard]] inline object  overload_bitwise_not                 (environment& a_env);
        [[nodiscard]] inline object  overload_shift_right                 (environment& a_env, const object rhs);
                      inline void    overload_shift_right_assignment      (environment& a_env, const object rhs);
        [[nodiscard]] inline object  overload_shift_left                  (environment& a_env, const object rhs);
                      inline void    overload_shift_left_assignment       (environment& a_env, const object rhs);
        [[nodiscard]] inline object  overload_exponent                    (environment& a_env, const object rhs);
                      inline void    overload_exponent_assignment         (environment& a_env, const object rhs);
        [[nodiscard]] inline object  overload_modulus                     (environment& a_env, const object rhs);
                      inline void    overload_modulus_assignment          (environment& a_env, const object rhs);
                      inline object  overload_length                      (environment& a_env);
                      inline void    overload_prefix_increment            (environment& a_env);
                      inline object  overload_postfix_increment           (environment& a_env);
                      inline void    overload_prefix_decrement            (environment& a_env);
                      inline object  overload_postfix_decrement           (environment& a_env);
        [[nodiscard]] inline object& overload_index                       (environment& a_env, const object rhs);
        [[nodiscard]] inline object  overload_select                      (environment& a_env, const object rhs);
        [[nodiscard]] inline object  overload_ranged_select               (environment& a_env, const object lhs, const object rhs);
        [[nodiscard]] inline object  overload_call                        (environment& a_env);
        [[nodiscard]] inline object  overload_new                         (environment& a_env);

        void reference() noexcept {
            ++reinterpret_cast<structured_native_object<std::nullptr_t>*>(m_root_pointer)->m_reference_count;
        }

        void dereference() noexcept {
            if (--reinterpret_cast<structured_native_object<std::nullptr_t>*>(m_root_pointer)->m_reference_count == 0) {
                destructor_function destructor = get_destructor();

                if (destructor != nullptr) {
                    destructor(&reinterpret_cast<structured_native_object<std::nullptr_t>*>(m_root_pointer)->m_data);
                }

                std::free(m_root_pointer);
            }
        }
    };

    struct array {
        enum class type : enum_base {
            managed,
            view
        };

    private:
        void* m_root_pointer;

        // type = managed (array_type)
        // reference_count (size_t)
        // objects (vector<object>)
        //
        // type = view (array_type)
        // reference_count (size_t)
        // reference_managed_array (array)
        // size (size_t)
        // index (size_t)

        constexpr size_t structure_size(type a_type) {
            switch (a_type) {
                case type::managed:
                    return (2 * sizeof(size_t)) + sizeof(std::vector<object>);
                case type::view:
                    return 5 * sizeof(size_t);
                default:
                    return 0;
            }
        }

        [[nodiscard]] inline type& type_reference() noexcept {
            return *reinterpret_cast<type*>(m_root_pointer);
        }

        [[nodiscard]] inline size_t& reference_count_reference() noexcept {
            return *(reinterpret_cast<size_t*>(m_root_pointer) + 1);
        }

        [[nodiscard]] inline std::vector<object>& vector_reference() noexcept {
            switch (get_type()) {
                case type::managed:
                    return *(reinterpret_cast<std::vector<object>*>(reinterpret_cast<size_t*>(m_root_pointer) + 2));
                case type::view:
                    return (reinterpret_cast<array*>(m_root_pointer) + 2)->vector_reference();
            }
        }

        [[nodiscard]] const std::vector<object>& vector_reference() const noexcept {
            switch (get_type()) {
                case type::managed:
                    return *(reinterpret_cast<std::vector<object>*>(reinterpret_cast<size_t*>(m_root_pointer) + 2));
                case type::view:
                    return (reinterpret_cast<array*>(m_root_pointer) + 2)->vector_reference();
            }
        }

        [[nodiscard]] inline array& view_array() noexcept {
            return *(reinterpret_cast<array*>(m_root_pointer) + 2);
        }

        [[nodiscard]] inline size_t& view_offset() noexcept {
            return *(reinterpret_cast<size_t*>(m_root_pointer) + 4);
        }

        [[nodiscard]] inline const size_t view_offset() const noexcept {
            return *(reinterpret_cast<size_t*>(m_root_pointer) + 4);
        }

    public:
        static constexpr size_t max_display_elements = 100;

        array() noexcept : m_root_pointer(nullptr) {}

        array(const size_t a_size) {
            initialize(type::managed, a_size);
        }

        array(array& a_reference, const size_t a_offset, const size_t a_length) noexcept {
            initialize(type::view);

            view_array() = a_reference;
            view_offset() = a_offset;
        }

        array(const array& a_array) noexcept : m_root_pointer(a_array.m_root_pointer) {
            reference();
        }

        array(array&& a_array) noexcept : m_root_pointer(a_array.m_root_pointer) {
            reference();
        }

        ~array() noexcept {
            if (m_root_pointer != nullptr) {
                dereference();
            }
        }

        array& operator = (const array& rhs) {
            dereference();
            m_root_pointer = rhs.m_root_pointer;
            reference();

            return *this;
        }

        array& operator = (array&& rhs) {
            dereference();

            m_root_pointer = rhs.m_root_pointer;
            rhs.m_root_pointer = nullptr;

            return *this;
        }

        void initialize(type a_type, size_t a_capacity = 4) noexcept {
            m_root_pointer = std::malloc(structure_size(a_type));
            std::memset(m_root_pointer, 0, structure_size(a_type));
            type_reference() = a_type;

            if (a_type == type::managed) {
                new (reinterpret_cast<std::vector<object>*>(reinterpret_cast<size_t*>(m_root_pointer) + 2)) std::vector<object>();
                vector_reference().reserve(a_capacity);
            }

            reference();
        }

        [[nodiscard]] inline type get_type() const noexcept {
            return *reinterpret_cast<type*>(m_root_pointer);
        }

        [[nodiscard]] inline size_t size() const noexcept {
            switch (get_type()) {
                case type::managed:
                    return vector_reference().size();
                case type::view:
                    return *(reinterpret_cast<size_t*>(m_root_pointer) + 3);
            }
        }

        [[nodiscard]] inline size_t capacity() const noexcept {
            return vector_reference().capacity();
        }

        [[nodiscard]] inline object* data() noexcept {
            return vector_reference().data();
        }

        [[nodiscard]] inline const object* data() const noexcept {
            return vector_reference().data();
        }

        [[nodiscard]] inline std::vector<object>::iterator begin() noexcept {
            return get_type() == type::view ? vector_reference().begin() + view_offset() : vector_reference().begin();
        }

        [[nodiscard]] inline std::vector<object>::const_iterator begin() const noexcept {
            return get_type() == type::view ? vector_reference().begin() + view_offset() : vector_reference().begin();
        }

        [[nodiscard]] inline std::vector<object>::const_iterator cbegin() const noexcept {
            return get_type() == type::view ? vector_reference().begin() + view_offset() : vector_reference().begin();
        }

        [[nodiscard]] inline std::vector<object>::iterator end() noexcept {
            return get_type() == type::view ? vector_reference().begin() + view_offset() + size() : vector_reference().end();
        }

        [[nodiscard]] inline std::vector<object>::const_iterator end() const noexcept {
            return get_type() == type::view ? vector_reference().begin() + view_offset() + size() : vector_reference().end();
        }

        [[nodiscard]] inline std::vector<object>::const_iterator cend() const noexcept {
            return get_type() == type::view ? vector_reference().begin() + view_offset() + size() : vector_reference().end();
        }

        [[nodiscard]] object& operator [] (const size_t index) noexcept {
            // TODO: Bounds checking.

            switch (get_type()) {
                case type::managed:
                    return vector_reference()[index];
                case type::view:
                    return vector_reference()[view_offset() + index];
            }
        }

        void push_back(const object a_object);

        [[nodiscard]] array sub_array(const size_t a_offset, const size_t a_length) {
            // TODO: Bounds checking.

            switch (get_type()) {
                case type::managed:
                    return { *this, a_offset, a_length };
                case type::view:
                    return { view_array(), view_offset() + a_offset, a_length };
            }
        }

        [[nodiscard]] std::string to_string() noexcept;

        void reference() {
            ++reference_count_reference();

            if (get_type() == type::view) {
                ++view_array().reference_count_reference();
            }
        }

        void dereference() {
            type arr_type = get_type();

            if (arr_type == type::view) {
                view_array().dereference();
            }

            if (--reference_count_reference() == 0) {
                if (arr_type == type::managed) {
                    vector_reference().~vector();
                }

                std::free(m_root_pointer);
            }
        }
    };

    struct alignas(sizeof(size_t) * 2) object {
        enum class type : enum_base {
            // Simple Types / Simply Comparable:
            null = 0,
            boolean = 1,
            integer = 2,
            number = 3,
            function = 4,

            // Complex Types:
            string = 5,

            // Complexly Comparable:
            table = 6,
            array = 7,
            native_object = 8
        };

        static constexpr type simple_type_end_boundary = type::number;
        static constexpr type simply_comparable_end_boundary = type::string;

    private:
        type m_type;
        size_t m_data;

    public:
        constexpr object(const type a_type, const size_t a_data) noexcept : m_type(a_type), m_data(a_data) {}

        constexpr object() noexcept : m_type(type::null), m_data(0) {}
        object(const integer a_integer) noexcept : m_type(type::integer), m_data(*reinterpret_cast<const size_t*>(&a_integer)) {}
        object(const int a_integer) noexcept : m_type(type::integer), m_data(0) {
            reinterpret_cast<integer&>(m_data) = static_cast<integer>(a_integer);
        }
        explicit object(const bool a_boolean) noexcept : m_type(type::boolean), m_data(*reinterpret_cast<const size_t*>(&a_boolean)) {}
        object(const function a_function) noexcept : m_type(type::function), m_data(reinterpret_cast<size_t>(a_function.m_data)) {}
        object(const number a_number) noexcept : m_type(type::number), m_data(*reinterpret_cast<const size_t*>(&a_number)) {}
        object(string a_string) noexcept : m_type(type::string), m_data(reinterpret_cast<size_t>(a_string.data())) {
            a_string.reference();
        }

        object(array a_array) noexcept : m_type(type::array), m_data(*reinterpret_cast<size_t*>(&a_array)) {
            a_array.reference();
        }

        object(table* a_table) noexcept : m_type(type::table), m_data(reinterpret_cast<size_t>(a_table)) {
            reference(*this);
        }

        object(native_object a_object) noexcept : m_type(type::native_object), m_data(reinterpret_cast<size_t>(a_object.data())) {
            a_object.reference();
        }

        // TODO: Generate casts/constructors for other types.

        object(const object& a_object) noexcept : m_type(a_object.m_type), m_data(a_object.m_data) {
            reference(*this);
        };

        object(object&& a_object) noexcept : m_type(a_object.m_type), m_data(a_object.m_data) {
            reference(*this);
        };

        ~object() noexcept {
            dereference(*this);
        }

        object& operator = (const object& a_object) noexcept {
            dereference(*this);

            m_type = a_object.m_type;
            m_data = a_object.m_data;

            reference(*this);

            return *this;
        }

        object& operator = (object&& a_object) noexcept {
            dereference(*this);

            m_type = a_object.m_type;
            m_data = a_object.m_data;

            reference(*this);

            return *this;
        }

        [[nodiscard]] constexpr type object_type() const noexcept {
            return m_type;
        }

        [[nodiscard]] constexpr bool is_null() const noexcept {
            return m_type == type::null;
        }

        [[nodiscard]] constexpr bool is_boolean() const noexcept {
            return m_type == type::boolean;
        }

        [[nodiscard]] constexpr bool is_integer() const noexcept {
            return m_type == type::integer;
        }

        [[nodiscard]] constexpr bool is_function() const noexcept {
            return m_type == type::function;
        }

        [[nodiscard]] constexpr bool is_number() const noexcept {
            return m_type == type::number;
        }

        [[nodiscard]] constexpr bool is_string() const noexcept {
            return m_type == type::string;
        }

        [[nodiscard]] constexpr bool is_table() const noexcept {
            return m_type == type::table;
        }

        [[nodiscard]] constexpr bool is_array() const noexcept {
            return m_type == type::table;
        }

        [[nodiscard]] constexpr bool is_native_object() const noexcept {
            return m_type == type::native_object;
        }

        [[nodiscard]] constexpr bool is_simple_type() const noexcept {
            return static_cast<size_t>(m_type) <= static_cast<size_t>(simple_type_end_boundary);
        }

        [[nodiscard]] constexpr bool is_complex_type() const noexcept {
            return static_cast<size_t>(m_type) > static_cast<size_t>(simple_type_end_boundary);
        }

        [[nodiscard]] constexpr bool is_simply_comparable() const noexcept {
            return static_cast<size_t>(m_type) <= static_cast<size_t>(simply_comparable_end_boundary);
        }

        [[nodiscard]] constexpr bool is_complexly_comparable() const noexcept {
            return static_cast<size_t>(m_type) > static_cast<size_t>(simply_comparable_end_boundary);
        }

        [[nodiscard]] constexpr size_t data() const noexcept {
            return m_data;
        }

        [[nodiscard]] constexpr bool get_boolean() const noexcept {
            return m_data != 0;
        }

        [[nodiscard]] inline integer get_integer() const noexcept {
            return *reinterpret_cast<const integer*>(&m_data);
        }

        [[nodiscard]] function get_function(environment& a_environment) const noexcept {
            return { a_environment, reinterpret_cast<const void*>(m_data) };
        }

        [[nodiscard]] number get_number() const noexcept {
            return *reinterpret_cast<const number*>(&m_data);
        }

        [[nodiscard]] string get_string() const noexcept {
            return string(reinterpret_cast<void*>(m_data));
        }

        [[nodiscard]] table& get_table() const noexcept {
            return *reinterpret_cast<table*>(m_data);
        }

        [[nodiscard]] native_object get_native_object() noexcept {
            return native_object{ reinterpret_cast<void*>(m_data) };
        }

        [[nodiscard]] array get_array() noexcept {
            return *reinterpret_cast<array*>(&m_data);
        }

        // TODO: Add functions for getting arrays.

        // Determines whether the object is "truthy."
        // Implementation may differ from get_boolean in the future;
        // make sure to use the proper version.
        [[nodiscard]] constexpr bool boolean_evaluate() const noexcept {
            return m_data != 0;
        }

        [[nodiscard]] object length(environment& a_environment) noexcept;

        template <typename... t_objects>
        object call(environment& a_environment, t_objects&&... a_objects);
        object call(environment& a_environment, const span<object> a_objects);

        template <typename... t_objects>
        object new_object(environment& a_environment, t_objects&&... a_objects);
        object new_object(environment& a_environment, const span<object> a_objects);

        [[nodiscard]] object& index(environment& a_environment, const object rhs);
        [[nodiscard]] object select(environment& a_environment, const object rhs);
        [[nodiscard]] object select(environment& a_environment, const object rhs1, const object rhs2);

        [[nodiscard]] std::string to_string() noexcept;

        bool operator == (const type rhs) const noexcept {
            return m_type == rhs;
        }

        // TODO: Implement addition operations for remaining types.
        static object add(environment& a_environment, object lhs, const object rhs);

        // TODO: Implement subtraction operations for remaining types.
        static object subtract(environment& a_environment, object lhs, const object rhs);

        // TODO: Implement multiplication operations for remaining types.
        static object multiply(environment& a_environment, object lhs, const object rhs);

        // TODO: Implement division operations for remaining types.
        static object divide(environment& a_environment, object lhs, const object rhs);

        // TODO: Implement modulus operations for remaining types.
        static object modulus(environment& a_environment, object lhs, const object rhs);

        // TODO: Implement exponentiation operations for remaining types.
        static object exponentiate(environment& a_environment, object lhs, const object rhs);

        // TODO: Implement equality operations for remaining types.
        static object equals(environment& a_environment, object lhs, const object rhs);

        // TODO: Implement inverse-equality operations for remaining types.
        static object not_equals(environment& a_environment, object lhs, const object rhs);

        // TODO: Implement shift-left operations for remaining types.
        static object shift_left(environment& a_environment, object lhs, const object rhs);

        // TODO: Implement shift-right operations for remaining types.
        static object shift_right(environment& a_environment, object lhs, const object rhs);

        // TODO: Implement bitwise XOR operations for remaining types.
        static object bitwise_xor(environment& a_environment, object lhs, const object rhs);

        // TODO: Implement bitwise OR operations for remaining types.
        static object bitwise_or(environment& a_environment, object lhs, const object rhs);

        // TODO: Implement bitwise AND operations for remaining types.
        static object bitwise_and(environment& a_environment, object lhs, const object rhs);

        // TODO: Implement bitwise NOT operations for remaining types.
        static object bitwise_not(environment& a_environment, object lhs);

        // TODO: Implement logical NOT operations for remaining types.
        static object logical_not(environment& a_environment, object lhs);

        // TODO: Implement logical OR operations for remaining types.
        static object logical_or(environment& a_environment, object lhs, const object rhs);

        // TODO: Implement logical AND operations for remaining types.
        static object logical_and(environment& a_environment, object lhs, const object rhs);

        // TODO: Implement greater-than operations for remaining types.
        static object greater_than(environment& a_environment, object lhs, const object rhs);

        // TODO: Implement lesser-than operations for remaining types.
        static object lesser_than(environment& a_environment, object lhs, const object rhs);

        // TODO: Implement greater-than-equal-to operations for remaining types.
        static object greater_than_equal_to(environment& a_environment, object lhs, const object rhs);

        // TODO: Implement lesser-than-equal-to operations for remaining types.
        static object lesser_than_equal_to(environment& a_environment, object lhs, const object rhs);

        friend std::ostream& operator << (std::ostream& lhs, object rhs) noexcept {
            switch (rhs.m_type) {
                case type::null:
                    return (lhs << "null");
                case type::boolean:
                    return (lhs << (rhs.get_boolean() ? "true" : "false"));
                case type::integer:
                    return (lhs << rhs.get_integer());
                case type::number:
                    return (lhs << rhs.get_number());
                case type::string:
                    return (lhs << rhs.get_string());
                case type::array:
                    return (lhs << rhs.get_array().to_string());
                case type::native_object:
                    return (lhs << "NATIVE_OBJECT");
                default:
                    return (lhs << rhs.data());
            }
        }

        [[nodiscard]] operator bool() const noexcept {
            return boolean_evaluate();
        }

        // TODO: Complete dereference function.
        void dereference(object& a_object);
        void reference(object& a_object);

        [[nodiscard]] bool operator == (const object rhs) const noexcept {
            if (m_type != rhs.m_type) {
                return false;
            }

            if (is_simply_comparable()) {
                return m_data == rhs.m_data;
            }

            return false;
        }
    };

    using type = object::type;

    inline object null { type::null, 0 };

    void array::push_back(const object a_object) {
        // TODO: Block views.
        vector_reference().push_back(a_object);
    }

    std::string array::to_string() noexcept {
        size_t max_elements = std::min(size(), max_display_elements);

        if (max_elements == 0) {
            return "[]";
        }

        std::string representation = "[ ";

        for (size_t i = 0; i < max_elements; ++i) {
            representation += (*this)[i].to_string();
            representation += (i == max_elements - 1)
                    ? ((size() > max_display_elements) ? ", ... ]" : " ]")
                    : ", ";
        }

        return representation;
    }
}

template <>
struct std::hash <rebar::object> {
    size_t operator()(const rebar::object s) const noexcept {
        return s.data();
    }
};

namespace rebar {
    struct table : public ska::detailv3::sherwood_v3_table<
            std::pair<object, object>,
            object,
            std::hash<object>,
            ska::detailv3::KeyOrValueHasher<object, std::pair<object, object>, std::hash<object>>,
            std::equal_to<object>,
            ska::detailv3::KeyOrValueEquality<object, std::pair<object, object>, std::equal_to<object>>,
            std::allocator<std::pair<object, object>>,
            typename std::allocator_traits<std::allocator<std::pair<object, object>>>::template rebind_alloc<ska::detailv3::sherwood_v3_entry<std::pair<object, object>>>
    > {
        size_t m_reference_count = 0;

        [[nodiscard]] inline object& operator[](const object a_key) {
            return emplace(a_key, object()).first->second;
        }

        [[nodiscard]] object& at(const object a_key) {
            auto found = this->find(a_key);

            if (found == this->cend()) {
                throw std::out_of_range("Argument passed to at() was not in the map.");
            }

            return found->second;
        }

        [[nodiscard]] const object& at(const object a_key) const {
            auto found = this->find(a_key);

            if (found == this->cend()) {
                throw std::out_of_range("Argument passed to at() was not in the map.");
            }

            return found->second;
        }

        [[nodiscard]] object index(const object a_key) const {
            auto found = this->find(a_key);

            if (found == this->cend()) {
                return {};
            }

            return found->second;
        }

        void add(const table& a_table) {
            for (const std::pair<object, object>& pair : a_table) {
                emplace(pair);
            }
        }

    public:

        [[nodiscard]] friend bool operator==(const table& lhs, const table& rhs) {
            if (lhs.size() != rhs.size()) {
                return false;
            }

            for (const std::pair<object, object>& value : lhs) {
                auto found = rhs.find(value.first);

                if (found == rhs.end() || value.second != found->second){
                    return false;
                }
            }

            return true;
        }

        [[nodiscard]] friend bool operator!=(const table& lhs, const table& rhs) {
            return !(lhs == rhs);
        }

        // TODO: Add "add" and "add assignment" operator.

        [[nodiscard]] friend object index(const table* a_table, const object& a_key) {
            auto found = a_table->find(a_key);

            if (found == a_table->end()) {
                return object();
            }

            return found->second;
        }

        friend void emplace(table* a_table, const object a_key, const object a_value) {
            a_table->emplace(a_key, a_value);
        }
    };

    struct virtual_table : public table {
        template <size_t v_arguments>
        using assignment_operation_function = std::conditional_t<v_arguments == 0, void (*)(environment*, native_object),
                                              std::conditional_t<v_arguments == 1, void (*)(environment*, native_object, object),
                                              std::conditional_t<v_arguments == 2, void (*)(environment*, native_object, object, object),
                                              void>>>;

        template <size_t v_arguments>
        using operation_function = std::conditional_t<v_arguments == 0, object (*)(environment*, native_object),
                                   std::conditional_t<v_arguments == 1, object (*)(environment*, native_object, object),
                                   std::conditional_t<v_arguments == 2, object (*)(environment*, native_object, object, object),
                                   void>>>;

        using index_function = object& (*)(environment*, native_object, object);

        // TODO: Throw exceptions in null functions.
        constexpr static const assignment_operation_function<0> null_aof_0 = [](environment*, native_object) {};
        constexpr static const assignment_operation_function<1> null_aof_1 = [](environment*, native_object, object) {};
        constexpr static const assignment_operation_function<2> null_aof_2 = [](environment*, native_object, object, object) {};

        constexpr static const operation_function<0> null_of_0 = [](environment*, native_object) -> object { return null; };
        constexpr static const operation_function<1> null_of_1 = [](environment*, native_object, object) -> object { return null; };
        constexpr static const operation_function<2> null_of_2 = [](environment*, native_object, object, object) -> object { return null; };

        constexpr static const index_function null_idx = [](environment*, native_object, object) -> object& { static object temp; return temp; };

        assignment_operation_function<1> overload_assignment                  = null_aof_1;
        operation_function<1>            overload_addition                    = null_of_1;
        assignment_operation_function<1> overload_addition_assignment         = null_aof_1;
        operation_function<1>            overload_multiplication              = null_of_1;
        assignment_operation_function<1> overload_multiplication_assignment   = null_aof_1;
        operation_function<1>            overload_division                    = null_of_1;
        assignment_operation_function<1> overload_division_assignment         = null_aof_1;
        operation_function<1>            overload_subtraction                 = null_of_1;
        assignment_operation_function<1> overload_subtraction_assignment      = null_aof_1;
        operation_function<1>            overload_equality                    = null_of_1;
        operation_function<1>            overload_inverse_equality            = null_of_1;
        operation_function<1>            overload_greater                     = null_of_1;
        operation_function<1>            overload_lesser                      = null_of_1;
        operation_function<1>            overload_greater_equality            = null_of_1;
        operation_function<1>            overload_lesser_equality             = null_of_1;
        operation_function<1>            overload_logical_or                  = null_of_1;
        operation_function<1>            overload_logical_and                 = null_of_1;
        operation_function<0>            overload_logical_not                 = null_of_0;
        operation_function<1>            overload_bitwise_or                  = null_of_1;
        assignment_operation_function<1> overload_bitwise_or_assignment       = null_aof_1;
        operation_function<1>            overload_bitwise_xor                 = null_of_1;
        assignment_operation_function<1> overload_bitwise_xor_assignment      = null_aof_1;
        operation_function<1>            overload_bitwise_and                 = null_of_1;
        assignment_operation_function<1> overload_bitwise_and_assignment      = null_aof_1;
        operation_function<0>            overload_bitwise_not                 = null_of_0;
        operation_function<1>            overload_shift_right                 = null_of_1;
        assignment_operation_function<1> overload_shift_right_assignment      = null_aof_1;
        operation_function<1>            overload_shift_left                  = null_of_1;
        assignment_operation_function<1> overload_shift_left_assignment       = null_aof_1;
        operation_function<1>            overload_exponent                    = null_of_1;
        assignment_operation_function<1> overload_exponent_assignment         = null_aof_1;
        operation_function<1>            overload_modulus                     = null_of_1;
        assignment_operation_function<1> overload_modulus_assignment          = null_aof_1;
        operation_function<0>            overload_length                      = null_of_0;
        assignment_operation_function<0> overload_operation_prefix_increment  = null_aof_0;
        operation_function<0>            overload_operation_postfix_increment = null_of_0;
        assignment_operation_function<0> overload_operation_prefix_decrement  = null_aof_0;
        operation_function<0>            overload_operation_postfix_decrement = null_of_0;
        index_function                   overload_operation_index             = null_idx;
        operation_function<1>            overload_operation_select            = null_of_1;
        operation_function<2>            overload_operation_ranged_select     = null_of_2;
        operation_function<0>            overload_operation_call              = null_of_0;
        operation_function<0>            overload_new                         = null_of_0;
    };

    // NATIVE OBJECT OVERLOAD FUNCTIONS

    inline void native_object::overload_assignment(environment& a_env, const object rhs) {
        this->get_virtual_table().overload_assignment(&a_env, *this, rhs);
    }

    [[nodiscard]] inline object native_object::overload_addition(environment& a_env, const object rhs) {
        return this->get_virtual_table().overload_addition(&a_env, *this, rhs);
    }

    inline void native_object::overload_addition_assignment(environment& a_env, const object rhs) {
        this->get_virtual_table().overload_addition_assignment(&a_env, *this, rhs);
    }

    [[nodiscard]] inline object native_object::overload_multiplication(environment& a_env, const object rhs) {
        return this->get_virtual_table().overload_multiplication(&a_env, *this, rhs);
    }

    inline void native_object::overload_multiplication_assignment(environment& a_env, const object rhs) {
        this->get_virtual_table().overload_multiplication_assignment(&a_env, *this, rhs);
    }

    [[nodiscard]] inline object native_object::overload_division(environment& a_env, const object rhs) {
        return this->get_virtual_table().overload_division(&a_env, *this, rhs);
    }

    inline void native_object::overload_division_assignment(environment& a_env, const object rhs) {
        this->get_virtual_table().overload_division_assignment(&a_env, *this, rhs);
    }

    [[nodiscard]] inline object native_object::overload_subtraction(environment& a_env, const object rhs) {
        return this->get_virtual_table().overload_subtraction(&a_env, *this, rhs);
    }

    inline void native_object::overload_subtraction_assignment(environment& a_env, const object rhs) {
        this->get_virtual_table().overload_subtraction_assignment(&a_env, *this, rhs);
    }

    [[nodiscard]] inline object native_object::overload_equality(environment& a_env, const object rhs) {
        return this->get_virtual_table().overload_equality(&a_env, *this, rhs);
    }

    [[nodiscard]] inline object native_object::overload_inverse_equality(environment& a_env, const object rhs) {
        return this->get_virtual_table().overload_inverse_equality(&a_env, *this, rhs);
    }

    [[nodiscard]] inline object native_object::overload_greater(environment& a_env, const object rhs) {
        return this->get_virtual_table().overload_greater(&a_env, *this, rhs);
    }

    [[nodiscard]] inline object native_object::overload_lesser(environment& a_env, const object rhs) {
        return this->get_virtual_table().overload_lesser(&a_env, *this, rhs);
    }

    [[nodiscard]] inline object native_object::overload_greater_equality(environment& a_env, const object rhs) {
        return this->get_virtual_table().overload_greater_equality(&a_env, *this, rhs);
    }

    [[nodiscard]] inline object native_object::overload_lesser_equality(environment& a_env, const object rhs) {
        return this->get_virtual_table().overload_lesser_equality(&a_env, *this, rhs);
    }

    [[nodiscard]] inline object native_object::overload_logical_or(environment& a_env, const object rhs) {
        return this->get_virtual_table().overload_logical_or(&a_env, *this, rhs);
    }

    [[nodiscard]] inline object native_object::overload_logical_and(environment& a_env, const object rhs) {
        return this->get_virtual_table().overload_logical_and(&a_env, *this, rhs);
    }

    [[nodiscard]] inline object native_object::overload_logical_not(environment& a_env) {
        return this->get_virtual_table().overload_logical_not(&a_env, *this);
    }

    [[nodiscard]] inline object native_object::overload_bitwise_or(environment& a_env, const object rhs) {
        return this->get_virtual_table().overload_bitwise_or(&a_env, *this, rhs);
    }

    inline void native_object::overload_bitwise_or_assignment(environment& a_env, const object rhs) {
        this->get_virtual_table().overload_bitwise_or_assignment(&a_env, *this, rhs);
    }

    [[nodiscard]] inline object native_object::overload_bitwise_xor(environment& a_env, const object rhs) {
        return this->get_virtual_table().overload_bitwise_xor(&a_env, *this, rhs);
    }

    inline void native_object::overload_bitwise_xor_assignment(environment& a_env, const object rhs) {
        this->get_virtual_table().overload_bitwise_xor_assignment(&a_env, *this, rhs);
    }

    [[nodiscard]] inline object native_object::overload_bitwise_and(environment& a_env, const object rhs) {
        return this->get_virtual_table().overload_bitwise_and(&a_env, *this, rhs);
    }

    inline void native_object::overload_bitwise_and_assignment(environment& a_env, const object rhs) {
        this->get_virtual_table().overload_bitwise_and_assignment(&a_env, *this, rhs);
    }

    [[nodiscard]] inline object native_object::overload_bitwise_not(environment& a_env) {
        return this->get_virtual_table().overload_bitwise_not(&a_env, *this);
    }

    [[nodiscard]] inline object native_object::overload_shift_right(environment& a_env, const object rhs) {
        return this->get_virtual_table().overload_shift_right(&a_env, *this, rhs);
    }

    inline void native_object::overload_shift_right_assignment(environment& a_env, const object rhs) {
        this->get_virtual_table().overload_shift_right_assignment(&a_env, *this, rhs);
    }

    [[nodiscard]] inline object native_object::overload_shift_left(environment& a_env, const object rhs) {
        return this->get_virtual_table().overload_shift_left(&a_env, *this, rhs);
    }

    inline void native_object::overload_shift_left_assignment(environment& a_env, const object rhs) {
        this->get_virtual_table().overload_shift_left_assignment(&a_env, *this, rhs);
    }

    [[nodiscard]] inline object native_object::overload_exponent(environment& a_env, const object rhs) {
        return this->get_virtual_table().overload_exponent(&a_env, *this, rhs);
    }

    inline void native_object::overload_exponent_assignment(environment& a_env, const object rhs) {
        this->get_virtual_table().overload_exponent_assignment(&a_env, *this, rhs);
    }

    [[nodiscard]] inline object native_object::overload_modulus(environment& a_env, const object rhs) {
        return this->get_virtual_table().overload_modulus(&a_env, *this, rhs);
    }

    inline void native_object::overload_modulus_assignment(environment& a_env, const object rhs) {
        this->get_virtual_table().overload_modulus_assignment(&a_env, *this, rhs);
    }

    inline object native_object::overload_length(environment& a_env) {
        return this->get_virtual_table().overload_length(&a_env, *this);
    }

    inline void native_object::overload_prefix_increment(environment& a_env) {
        this->get_virtual_table().overload_operation_prefix_increment(&a_env, *this);
    }

    inline object native_object::overload_postfix_increment(environment& a_env) {
        return this->get_virtual_table().overload_operation_postfix_increment(&a_env, *this);
    }

    inline void native_object::overload_prefix_decrement(environment& a_env) {
        this->get_virtual_table().overload_operation_prefix_decrement(&a_env, *this);
    }

    inline object native_object::overload_postfix_decrement(environment& a_env) {
        return this->get_virtual_table().overload_operation_postfix_decrement(&a_env, *this);
    }

    [[nodiscard]] inline object& native_object::overload_index(environment& a_env, const object rhs) {
        return this->get_virtual_table().overload_operation_index(&a_env, *this, rhs);
    }

    [[nodiscard]] inline object native_object::overload_select(environment& a_env, const object rhs) {
        return this->get_virtual_table().overload_operation_select(&a_env, *this, rhs);
    }

    [[nodiscard]] inline object native_object::overload_ranged_select(environment& a_env, const object lhs, const object rhs) {
        return this->get_virtual_table().overload_operation_ranged_select(&a_env, *this, lhs, rhs);
    }

    [[nodiscard]] inline object native_object::overload_call(environment& a_env) {
        return this->get_virtual_table().overload_operation_call(&a_env, *this);
    }

    [[nodiscard]] inline object native_object::overload_new(environment& a_env) {
        return this->get_virtual_table().overload_new(&a_env, *this);
    }

    // ~ NATIVE OBJECT OVERLOAD FUNCTIONS
}

template <>
struct std::hash <rebar::string> {
    size_t operator()(const rebar::string a_string) const noexcept {
        return xxh::xxhash3<rebar::cpu_bit_architecture()>(a_string.to_string_view());
    }
};

namespace rebar {
    struct xxh_string_view_hash {
        size_t operator()(const std::string_view a_string) const noexcept {
            return xxh::xxhash3<rebar::cpu_bit_architecture()>(a_string);
        }
    };

    /*
        Base Function:

         - sizeof(function) = sizeof(size_t)

        class function {

        public:
            template <typename... t_object>
            object call(t_object&&... a_objects) const {

            }

            template <typename... t_object>
            object operator()(t_object&&... a_objects) const {
                call(std::forward<t_object>(a_objects)...);
            }
        };

    */

    class environment;

    using callable = object (*)(environment*);

    struct provider {
        [[nodiscard]] virtual function compile(parse_unit a_unit) = 0;
        [[nodiscard]] virtual function bind(callable a_function) = 0;
        [[nodiscard]] virtual object call(const void* a_data) = 0;
    };

    struct interpreter : public provider {
        class function_source {
        protected:
            environment& m_environment;

        public:
            explicit function_source(environment& a_environment) noexcept : m_environment(a_environment) {}

            [[nodiscard]] environment& env() noexcept {
                return m_environment;
            }

            virtual object internal_call() = 0;
        };

        class native_function_source : public function_source {
            callable m_function;

        public:
            native_function_source(environment& a_environment, callable a_callable) noexcept :
                    function_source(a_environment),
                    m_function(a_callable) {}

        protected:
            object internal_call() override {
                return m_function(&m_environment);
            }
        };

        class interpreted_function_source : public function_source {
            parse_unit::node::argument_list m_arguments;
            const parse_unit::node::block& m_body;

        public:
            interpreted_function_source(environment& a_environment, parse_unit::node::argument_list a_arguments, const parse_unit::node::block& a_body) noexcept :
                    function_source(a_environment),
                    m_arguments(std::move(a_arguments)),
                    m_body(a_body) {}

        protected:
            object internal_call() override;
        };

        explicit interpreter(environment& a_environment) noexcept : m_environment(a_environment), m_arguments(1) {}

        [[nodiscard]] function compile(parse_unit a_unit) override {
            m_parse_units.push_back(std::make_unique<parse_unit>(std::move(a_unit)));
            m_function_sources.emplace_back(dynamic_cast<function_source*>(new interpreted_function_source(m_environment, parse_unit::node::argument_list(), m_parse_units.back()->m_block)));
            return { m_environment, m_function_sources.back().get() };
        }

        [[nodiscard]] function bind(callable a_function) override {
            m_function_sources.emplace_back(dynamic_cast<function_source*>(new native_function_source(m_environment, a_function)));
            return { m_environment, m_function_sources.back().get() };
        }

        [[nodiscard]] object call(const void* a_data) override {
            // I know, I know. It should be relatively safe.
            auto* func = const_cast<function_source*>(reinterpret_cast<const function_source*>(a_data));

            return func->internal_call();
        }

    private:
        environment& m_environment;
        size_t m_argument_stack_position = 0;
        std::vector<std::vector<object>> m_arguments;
        std::vector<std::unique_ptr<parse_unit>> m_parse_units;
        std::vector<std::unique_ptr<function_source>> m_function_sources;
    };

    template <typename t_provider>
    struct use_provider_t {};

    template <typename t_provider>
    inline constexpr use_provider_t<t_provider> use_provider{};

    using default_provider = interpreter;

    class environment {
        friend class function;

        ska::detailv3::sherwood_v3_table<
            std::pair<std::string_view, string>,
            std::string_view,
            xxh_string_view_hash,
            ska::detailv3::KeyOrValueHasher<std::string_view, std::pair<std::string_view, string>, xxh_string_view_hash>,
            std::equal_to<std::string_view>,
            ska::detailv3::KeyOrValueEquality<std::string_view, std::pair<std::string_view, string>, std::equal_to<std::string_view>>,
            std::allocator<std::pair<std::string_view, string>>,
            typename std::allocator_traits<std::allocator<std::pair<std::string_view, string>>>::template rebind_alloc<ska::detailv3::sherwood_v3_entry<std::pair<std::string_view, string>>>
        > m_string_table; // I don't like it any more than you do.

        table m_string_virtual_table;

        ska::detailv3::sherwood_v3_table<
                std::pair<object, virtual_table>,
                object,
                std::hash<object>,
                ska::detailv3::KeyOrValueHasher<object, std::pair<object, virtual_table>, std::hash<object>>,
                std::equal_to<object>,
                ska::detailv3::KeyOrValueEquality<object, std::pair<object, virtual_table>, std::equal_to<object>>,
                std::allocator<std::pair<object, virtual_table>>,
                typename std::allocator_traits<std::allocator<std::pair<object, virtual_table>>>::template rebind_alloc<ska::detailv3::sherwood_v3_entry<std::pair<object, virtual_table>>>
        > m_native_class_table; // Ditto.

        parser m_parser;
        std::vector<function> m_functions;
        table m_global_table;
        std::unique_ptr<provider> m_provider;
        size_t m_argument_stack_position = 0;
        std::vector<std::vector<object>> m_arguments;


    public:
        environment() noexcept : m_provider(std::make_unique<default_provider>(*this)), m_arguments(1) {}

        template <typename t_provider>
        explicit environment(const use_provider_t<t_provider>) noexcept : m_provider(std::make_unique<t_provider>(*this)), m_arguments(1) {};

        environment(const environment&) = delete;
        environment(environment&&) = delete;

        [[nodiscard]] string str(const std::string_view a_string) {
            auto found = m_string_table.find(a_string);

            if (found == m_string_table.cend()) {
                string created_string(a_string);

                m_string_table.emplace(created_string.to_string_view(), std::move(created_string));

                return created_string;
            }

            return found->second;
        }

        [[nodiscard]] table& get_string_virtual_table() noexcept {
            return m_string_virtual_table;
        }

        virtual_table& register_native_class(const object a_identifier, virtual_table a_table = {}) {
            auto iterator_pair = m_native_class_table.emplace(a_identifier, std::move(a_table));
            return iterator_pair.first->second;
        }

        virtual_table& register_native_class(const std::string_view a_identifier, virtual_table a_table = {}) {
            auto iterator_pair = m_native_class_table.emplace(str(a_identifier), std::move(a_table));
            return iterator_pair.first->second;
        }

        template <class t_class>
        virtual_table& register_native_class(const object a_identifier, t_class& a_class) {
            // TODO: Implement.
        }

        template <class t_class>
        virtual_table& register_native_class(const std::string_view a_identifier, t_class& a_class) {
            // TODO: Implement.
        }

        [[nodiscard]] virtual_table& get_native_class(const object a_identifier) {
            auto found = m_native_class_table.find(a_identifier);

            if (found == m_native_class_table.cend()) {
                throw std::out_of_range("Identifier passed to get_native_class() was not registered.");
            }

            return found->second;
        }

        [[nodiscard]] virtual_table& get_native_class(const std::string_view a_identifier) {
            auto found = m_native_class_table.find(str(a_identifier));

            if (found == m_native_class_table.cend()) {
                throw std::out_of_range("Identifier passed to get_native_class() was not registered.");
            }

            return found->second;
        }

        template <typename t_object>
        [[maybe_unused]] [[nodiscard]] native_object create_native_object(virtual_table& a_virtual_table) {
            return native_object::create<t_object>(a_virtual_table, std::in_place_type<t_object>);
        }

        template <typename t_object>
        [[maybe_unused]] [[nodiscard]] native_object create_native_object(const object a_identifier) {
            return native_object::create<t_object>(get_native_class(a_identifier), std::in_place_type<t_object>);
        }

        template <typename t_object>
        [[maybe_unused]] [[nodiscard]] native_object create_native_object(const std::string_view a_identifier) {
            return native_object::create<t_object>(get_native_class(a_identifier), std::in_place_type<t_object>);
        }

        template <typename t_object>
        [[maybe_unused]] [[nodiscard]] native_object create_native_object(virtual_table& a_virtual_table, t_object a_object) {
            return native_object::create<t_object>(a_virtual_table, std::move(a_object));
        }

        template <typename t_object>
        [[maybe_unused]] [[nodiscard]] native_object create_native_object(const object a_identifier, t_object a_object) {
            return native_object::create<t_object>(get_native_class(a_identifier), std::move(a_object));
        }

        template <typename t_object>
        [[maybe_unused]] [[nodiscard]] native_object create_native_object(const std::string_view a_identifier, t_object a_object) {
            return native_object::create<t_object>(get_native_class(a_identifier), std::move(a_object));
        }

        template <typename t_object, typename... t_args>
        [[maybe_unused]] [[nodiscard]] native_object create_native_object(virtual_table& a_virtual_table, std::in_place_type_t<t_object> a_in_place, t_args... a_args) {
            return native_object::create<t_object>(a_virtual_table, a_in_place, std::forward<t_args>(a_args)...);
        }

        template <typename t_object, typename... t_args>
        [[maybe_unused]] [[nodiscard]] native_object create_native_object(const object a_identifier, std::in_place_type_t<t_object> a_in_place, t_args... a_args) {
            return native_object::create<t_object>(get_native_class(a_identifier), a_in_place, std::forward<t_args>(a_args)...);
        }

        template <typename t_object, typename... t_args>
        [[maybe_unused]] [[nodiscard]] native_object create_native_object(const std::string_view a_identifier, std::in_place_type_t<t_object> a_in_place, t_args... a_args) {
            return native_object::create<t_object>(get_native_class(a_identifier), a_in_place, std::forward<t_args>(a_args)...);
        }

        [[nodiscard]] function compile_string(std::string a_string) {
            return m_provider->compile(m_parser.parse(std::move(a_string)));
        }

        [[nodiscard]] object bind(callable a_function) {
            return m_provider->bind(a_function);
        }

        // FUNCTION PARAMETERS

        [[nodiscard]] size_t arg_count() const noexcept {
            return m_arguments[m_argument_stack_position].size();
        }

        [[nodiscard]] object arg(const size_t a_index) const noexcept {
            return (a_index < arg_count()) ? m_arguments[m_argument_stack_position][a_index] : null;
        }

        void set_arg(const size_t a_index, const object a_object) {
            if (m_arguments[m_argument_stack_position].size() > a_index) {
                m_arguments[m_argument_stack_position][a_index] = a_object;
            } else {
                m_arguments[m_argument_stack_position].reserve(a_index + 1);
                m_arguments[m_argument_stack_position].emplace(m_arguments[m_argument_stack_position].end() + a_index, a_object);
            }
        }

        void set_args(const span<object> a_objects) {
            std::copy(a_objects.begin(), a_objects.end(), std::back_inserter(m_arguments[m_argument_stack_position]));
        }

        void clear_args() noexcept {
            m_arguments[m_argument_stack_position].clear();
        }

        void inc_arg_stack() noexcept {
            m_argument_stack_position++;

            if (m_arguments.size() <= m_argument_stack_position) {
                m_arguments.emplace_back();
            }
        }

        void dec_arg_stack() noexcept {
            m_arguments[m_argument_stack_position].clear();
            m_argument_stack_position--;
        }

        // - FUNCTION PARAMETERS

        /*
        [[nodiscard]] table& get_metadata(function a_function) noexcept {

        }

        [[nodiscard]] source_position plaintext_source(function a_function) const noexcept {

        }

        [[nodiscard]] parse_unit& function_source_unit(function a_function) noexcept {

        }
        */

        [[nodiscard]] table& global_table() noexcept {
            return m_global_table;
        }

        [[nodiscard]] parser& code_parser() noexcept {
            return m_parser;
        }

        [[nodiscard]] provider& execution_provider() noexcept {
            return *m_provider;
        }
    };

    string::string(environment& a_env, const std::string_view a_string) : m_root_pointer(a_env.str(a_string).m_root_pointer) {}

    template <typename... t_objects>
    object object::call(environment& a_environment, t_objects&&... a_objects) {
        switch (m_type) {
            case type::function:
                return get_function(a_environment).call(std::forward<t_objects>(a_objects)...);
            case type::native_object: {
                a_environment.inc_arg_stack();

                if constexpr (sizeof...(a_objects) > 0) {
                    std::vector<object> args{ { std::forward<t_objects>(a_objects)... } };
                    a_environment.set_args(args);
                }

                object result = get_native_object().overload_call(a_environment);

                a_environment.dec_arg_stack();

                return result;
            }
            default:
                return {};
        }
    }

    object object::call(environment& a_environment, const span<object> a_objects) {
        switch (m_type) {
            case type::function:
                return get_function(a_environment).call(a_objects);
            case type::native_object: {
                a_environment.inc_arg_stack();
                a_environment.set_args(a_objects);

                object result = get_native_object().overload_call(a_environment);

                a_environment.dec_arg_stack();

                return result;
            }
            default:
                return {};
        }
    }

    template <typename... t_objects>
    object object::new_object(environment& a_environment, t_objects&&... a_objects) {
        switch (m_type) {
            case type::native_object: {
                a_environment.inc_arg_stack();

                if constexpr (sizeof...(a_objects) > 0) {
                    std::vector<object> args{ { std::forward<t_objects>(a_objects)... } };
                    a_environment.set_args(args);
                }

                object result = get_native_object().overload_new(a_environment);

                a_environment.dec_arg_stack();

                return result;
            }
            default:
                return {};
        }
    }

    object object::new_object(environment& a_environment, const span<object> a_objects) {
        switch (m_type) {
            case type::native_object: {
                a_environment.inc_arg_stack();
                a_environment.set_args(a_objects);

                object result = get_native_object().overload_new(a_environment);

                a_environment.dec_arg_stack();

                return result;
            }
            default:
                return {};
        }
    }

    std::string object::to_string() noexcept {
        switch (m_type) {
            case type::null:
                return "null";
            case type::boolean:
                return m_data != 0 ? "true" : "false";
            case type::integer:
                return std::to_string(static_cast<integer>(m_data));
            case type::number:
                return std::to_string(get_number());
            case type::string:
                return "\"" + std::string(get_string().to_string_view()) + "\"";
            case type::array:
                return get_array().to_string();
            case type::native_object:
                return "NATIVE_OBJECT";
            default:
                return std::to_string(m_data);
        }
    }

    object object::add(environment& a_environment, object lhs, object rhs) {
        switch (lhs.m_type) {
            case type::null:
                if (rhs.is_string()) {
                    std::string result{ "null" };
                    result += rhs.get_string().to_string_view();
                    return a_environment.str(result);
                } else {
                    return {};
                }
            case type::boolean:
                if (rhs.is_string()) {
                    std::string result{ lhs.get_boolean() ? "true" : "false" };
                    result += rhs.get_string().to_string_view();
                    return a_environment.str(result);
                } else if (rhs.is_integer()) {
                    return rhs.get_integer() + lhs.get_boolean();
                } else if (rhs.is_number()) {
                    return rhs.get_number() + lhs.get_boolean();
                } else if (!lhs.get_boolean()) {
                    return rhs.boolean_evaluate();
                } else {
                    return lhs;
                }
            case type::integer:
                switch (rhs.m_type) {
                    case type::null:
                        return null;
                    case type::boolean:
                        return lhs.get_integer() + rhs.get_boolean();
                    case type::integer:
                        return lhs.get_integer() + rhs.get_integer();
                    case type::function:
                        return null;
                    case type::number:
                        return static_cast<number>(lhs.get_integer()) + rhs.get_number();
                    case type::string: {
                        std::string result{ std::to_string(lhs.get_integer()) };
                        result += rhs.get_string().to_string_view();
                        return a_environment.str(result);
                    }
                    case type::table:
                    case type::array:
                    case type::native_object:
                        return null;
                }
            case type::function:
                return null;
            case type::number:
                switch (rhs.m_type) {
                    case type::null:
                        return null;
                    case type::boolean:
                        return lhs.get_number() + rhs.get_boolean();
                    case type::integer:
                        return lhs.get_number() + static_cast<number>(rhs.get_integer());
                    case type::function:
                        return null;
                    case type::number:
                        return lhs.get_number() + rhs.get_number();
                    case type::string: {
                        std::string result{ std::to_string(lhs.get_number()) };
                        result += rhs.get_string().to_string_view();
                        return a_environment.str(result);
                    }
                    case type::table:
                    case type::array:
                    case type::native_object:
                        return null;
                }
            case type::string: {
                std::string result{ lhs.get_string().to_string_view() };

                if (rhs.is_string()) {
                    result += rhs.get_string().to_string_view();
                } else {
                    result += rhs.to_string();
                }

                return a_environment.str(result);
            }
            case type::array:
                lhs.get_array().push_back(rhs);
                return lhs;
            case type::table:
            case type::native_object:
                return lhs.get_native_object().overload_addition(a_environment, rhs);
        }
    }

    object object::multiply(environment& a_environment, object lhs, const object rhs) {
        switch (lhs.m_type) {
            case type::null:
                return null;
            case type::boolean:
                if (rhs.is_boolean()) {
                    return lhs.get_boolean() && rhs.get_boolean();
                } else {
                    return null;
                }
            case type::integer:
                switch (rhs.m_type) {
                    case type::null:
                        return null;
                    case type::boolean:
                        return lhs.get_integer() * rhs.get_boolean();
                    case type::integer:
                        return lhs.get_integer() * rhs.get_integer();
                    case type::function:
                        return null;
                    case type::number:
                        return static_cast<number>(lhs.get_integer()) * rhs.get_number();
                    case type::string:
                    case type::table:
                    case type::array:
                    case type::native_object:
                        return null;
                }
            case type::function:
                return null;
            case type::number:
                switch (rhs.m_type) {
                    case type::null:
                        return null;
                    case type::boolean:
                        return lhs.get_number() * rhs.get_boolean();
                    case type::integer:
                        return lhs.get_number() * static_cast<number>(rhs.get_integer());
                    case type::function:
                        return {};
                    case type::number:
                        return lhs.get_number() * rhs.get_number();
                    case type::string:
                    case type::table:
                    case type::array:
                    case type::native_object:
                        return null;
                }
            case type::string: {
                if (rhs.is_integer()) {
                    string str = lhs.get_string();
                    std::string_view str_view = str.to_string_view();
                    integer multiplier = rhs.get_integer();

                    std::string result;
                    result.reserve(multiplier * str.length());

                    for (size_t i = 0; i < multiplier; i++) {
                        result += str_view;
                    }

                    return a_environment.str(result);
                } else {
                    return {};
                }
            }
            case type::table:
            case type::array:
                return null;
            case type::native_object:
                return lhs.get_native_object().overload_multiplication(a_environment, rhs);
        }
    }

    object object::subtract(environment& a_environment, object lhs, const object rhs) {
        switch (lhs.m_type) {
            case type::null:
                return null;
            case type::boolean:
                if (lhs.get_boolean() && rhs.is_boolean()) {
                    return !rhs.get_boolean();
                } else {
                    return null;
                }
            case type::integer:
                switch (rhs.m_type) {
                    case type::null:
                        return null;
                    case type::boolean:
                        return lhs.get_integer() - rhs.get_boolean();
                    case type::integer:
                        return lhs.get_integer() - rhs.get_integer();
                    case type::function:
                        return null;
                    case type::number:
                        return static_cast<number>(lhs.get_integer()) - rhs.get_number();
                    case type::string:
                    case type::table:
                    case type::array:
                    case type::native_object:
                        return null;
                }
            case type::function:
                return null;
            case type::number:
                switch (rhs.m_type) {
                    case type::null:
                        return null;
                    case type::boolean:
                        return lhs.get_number() - rhs.get_boolean();
                    case type::integer:
                        return lhs.get_number() - static_cast<number>(rhs.get_integer());
                    case type::function:
                        return null;
                    case type::number:
                        return lhs.get_number() - rhs.get_number();
                    case type::string:
                    case type::table:
                    case type::array:
                    case type::native_object:
                        return null;
                }
            case type::string:
            case type::table:
            case type::array:
                return null;
            case type::native_object:
                return lhs.get_native_object().overload_subtraction(a_environment, rhs);
        }
    }

    object object::divide(environment& a_environment, object lhs, const object rhs) {
        switch (lhs.m_type) {
            case type::null:
            case type::boolean:
                return null;
            case type::integer:
                switch (rhs.m_type) {
                    case type::null:
                    case type::boolean:
                        return null;
                    case type::integer:
                        return static_cast<number>(lhs.get_integer()) / static_cast<number>(rhs.get_integer());
                    case type::function:
                        return null;
                    case type::number:
                        return static_cast<number>(lhs.get_integer()) / rhs.get_number();
                    case type::string:
                    case type::table:
                    case type::array:
                    case type::native_object:
                        return null;
                }
            case type::function:
                return {};
            case type::number:
                switch (rhs.m_type) {
                    case type::null:
                    case type::boolean:
                        return null;
                    case type::integer:
                        return lhs.get_number() / static_cast<number>(rhs.get_integer());
                    case type::function:
                        return null;
                    case type::number:
                        return lhs.get_number() / rhs.get_number();
                    case type::string:
                    case type::table:
                    case type::array:
                    case type::native_object:
                        return null;
                }
            case type::string:
            case type::table:
            case type::array:
                return null;
            case type::native_object:
                return lhs.get_native_object().overload_division(a_environment, rhs);
        }
    }

    object object::modulus(environment& a_environment, object lhs, const object rhs) {
        switch (lhs.m_type) {
            case type::null:
            case type::boolean:
            case type::integer:
                if (rhs.is_integer()) {
                    return lhs.get_integer() % rhs.get_integer();
                } else if (rhs.is_number()) {
                    return std::fmod(static_cast<number>(lhs.get_integer()), rhs.get_number());
                }
            case type::function:
                return null;
            case type::number:
                if (rhs.is_integer()) {
                    return std::fmod(lhs.get_number(), rhs.get_integer());
                } else if (rhs.is_number()) {
                    return std::fmod(lhs.get_number(), rhs.get_number());
                }
            case type::string:
            case type::table:
            case type::array:
                return null;
            case type::native_object:
                return lhs.get_native_object().overload_modulus(a_environment, rhs);
        }
    }

    object object::exponentiate(environment& a_environment, object lhs, const object rhs) {
        switch (lhs.m_type) {
            case type::null:
                return null;
            case type::boolean:
                return null;
            case type::integer:
                switch (rhs.m_type) {
                    case type::null:
                    case type::boolean:
                        return null;
                    case type::integer:
                        return std::pow(lhs.get_integer(), rhs.get_integer());
                    case type::function:
                        return null;
                    case type::number:
                        return std::pow(static_cast<number>(lhs.get_integer()), rhs.get_number());
                    case type::string:
                    case type::table:
                    case type::array:
                    case type::native_object:
                        return null;
                }
            case type::function:
                return null;
            case type::number:
                switch (rhs.m_type) {
                    case type::null:
                    case type::boolean:
                        return null;
                    case type::integer:
                        return std::pow(lhs.get_number(), rhs.get_integer());
                    case type::function:
                        return null;
                    case type::number:
                        return std::pow(lhs.get_number(), rhs.get_number());
                    case type::string:
                    case type::table:
                    case type::array:
                    case type::native_object:
                        return null;
                }
            case type::string:
            case type::table:
            case type::array:
                return null;
            case type::native_object:
                return lhs.get_native_object().overload_exponent(a_environment, rhs);
        }
    }

    object object::equals(environment& a_environment, object lhs, const object rhs) {
        if (lhs.m_type != rhs.m_type) {
            return false;
        }

        if (lhs.is_simply_comparable()) {
            return lhs.m_data == rhs.m_data;
        } else {
            switch (lhs.m_type) {
                case type::native_object:
                    return lhs.get_native_object().overload_equality(a_environment, rhs);
            }

            // TODO: Implement comparisons for complexly comparable types.
        }

        return false;
    }

    object object::not_equals(environment& a_environment, object lhs, const object rhs) {
        if (lhs.m_type != rhs.m_type) {
            return true;
        }

        if (lhs.is_simply_comparable()) {
            return lhs.m_data != rhs.m_data;
        } else {
            switch (lhs.m_type) {
                case type::native_object:
                    return lhs.get_native_object().overload_inverse_equality(a_environment, rhs);
            }

            // TODO: Implement comparisons for complexly comparable types.
        }

        return false;
    }

    object object::shift_left(environment& a_environment, object lhs, const object rhs) {
        if (rhs.is_integer() && (lhs.is_integer() || lhs.is_number())) {
            return { lhs.m_type, lhs.m_data << rhs.get_integer() };
        } else {
            if (lhs.is_native_object()) {
                return lhs.get_native_object().overload_shift_left(a_environment, rhs);
            }

            // TODO: Throw invalid operation exception.
            // TODO: Provide overload for native objects.
            return null;
        }
    }

    object object::shift_right(environment& a_environment, object lhs, const object rhs) {
        if (rhs.is_integer() && (lhs.is_integer() || lhs.is_number())) {
            return { lhs.m_type, lhs.m_data >> rhs.get_integer() };
        } else {
            if (lhs.is_native_object()) {
                return lhs.get_native_object().overload_shift_right(a_environment, rhs);
            }

            // TODO: Throw invalid operation exception.
            // TODO: Provide overload for native objects.
            return null;
        }
    }

    object object::bitwise_xor(environment& a_environment, object lhs, const object rhs) {
        if ((lhs.is_integer() || lhs.is_number()) && (rhs.is_integer() || rhs.is_number())) {
            return { lhs.m_type, lhs.m_data ^ rhs.m_data };
        } else {
            if (lhs.is_native_object()) {
                return lhs.get_native_object().overload_bitwise_xor(a_environment, rhs);
            }

            // TODO: Throw invalid operation exception.
            // TODO: Provide overload for native objects.
            return null;
        }
    }

    object object::bitwise_or(environment& a_environment, object lhs, const object rhs) {
        if ((lhs.is_integer() || lhs.is_number()) && (rhs.is_integer() || rhs.is_number())) {
            return { lhs.m_type, lhs.m_data | rhs.m_data };
        } else {
            if (lhs.is_native_object()) {
                return lhs.get_native_object().overload_bitwise_or(a_environment, rhs);
            }

            // TODO: Throw invalid operation exception.
            // TODO: Provide overload for native objects.
            return null;
        }
    }

    object object::bitwise_and(environment& a_environment, object lhs, const object rhs) {
        if ((lhs.is_integer() || lhs.is_number()) && (rhs.is_integer() || rhs.is_number())) {
            return { lhs.m_type, lhs.m_data | rhs.m_data };
        } else {
            if (lhs.is_native_object()) {
                return lhs.get_native_object().overload_bitwise_and(a_environment, rhs);
            }

            // TODO: Throw invalid operation exception.
            // TODO: Provide overload for native objects.
            return null;
        }
    }

    object object::bitwise_not(environment& a_environment, object lhs) {
        if (lhs.is_integer() || lhs.is_number()) {
            return { lhs.m_type, ~lhs.m_data };
        } else {
            if (lhs.is_native_object()) {
                return lhs.get_native_object().overload_bitwise_not(a_environment);
            }

            // TODO: Throw invalid operation exception.
            // TODO: Provide overload for native objects.
            return null;
        }
    }

    object object::logical_not(environment& a_environment, object lhs) {
        if (lhs.is_native_object()) {
            return lhs.get_native_object().overload_logical_not(a_environment);
        } else {
            return !lhs.boolean_evaluate();
        }
    }

    object object::logical_or(environment& a_environment, object lhs, const object rhs) {
        if (lhs.is_native_object()) {
            return lhs.get_native_object().overload_logical_or(a_environment, rhs);
        } else {
            return lhs.boolean_evaluate() ? lhs : rhs;
        }
    }

    object object::logical_and(environment& a_environment, object lhs, const object rhs) {
       if (lhs.is_native_object()) {
           return lhs.get_native_object().overload_logical_and(a_environment, rhs);
       } else {
           return lhs.boolean_evaluate() && rhs.boolean_evaluate() ? rhs : object();
       }
    }

    object object::greater_than(environment& a_environment, object lhs, const object rhs) {
        if (lhs.is_integer()) {
            if (rhs.is_integer()) {
                return lhs.get_integer() > rhs.get_integer();
            } else if (rhs.is_number()) {
                return lhs.get_integer() > rhs.get_number();
            } else if (rhs.is_string()) {
                return lhs.get_integer() > rhs.get_string().length();
            }
        } else if (lhs.is_number()) {
            if (rhs.is_integer()) {
                return lhs.get_number() > rhs.get_integer();
            } else if (rhs.is_number()) {
                return lhs.get_number() > rhs.get_number();
            }
        } else if (lhs.is_string()) {
            if (rhs.is_integer()) {
                return (lhs.get_string().length() > rhs.get_integer());
            }
        } else if (lhs.is_native_object()) {
            return lhs.get_native_object().overload_greater(a_environment, rhs);
        }

        // TODO: Throw invalid operation exception.
        // TODO: Provide overload for native objects.
        return null;
    }

    object object::lesser_than(environment& a_environment, object lhs, const object rhs) {
        if (lhs.is_integer()) {
            if (rhs.is_integer()) {
                return lhs.get_integer() < rhs.get_integer();
            } else if (rhs.is_number()) {
                return lhs.get_integer() < rhs.get_number();
            } else if (rhs.is_string()) {
                return lhs.get_integer() < rhs.get_string().length();
            }
        } else if (lhs.is_number()) {
            if (rhs.is_integer()) {
                return lhs.get_number() < rhs.get_integer();
            } else if (rhs.is_number()) {
                return lhs.get_number() < rhs.get_number();
            }
        } else if (lhs.is_string()) {
            if (rhs.is_integer()) {
                return (lhs.get_string().length() < rhs.get_integer());
            }
        } else if (lhs.is_native_object()) {
            return lhs.get_native_object().overload_lesser(a_environment, rhs);
        }

        // TODO: Throw invalid operation exception.
        // TODO: Provide overload for native objects.
        return false;
    }

    object object::greater_than_equal_to(environment& a_environment, object lhs, const object rhs) {
        if (lhs.is_native_object()) {
            return lhs.get_native_object().overload_greater_equality(a_environment, rhs);
        } else {
            return !(rhs < rhs);
        }
    }

    object object::lesser_than_equal_to(environment& a_environment, object lhs, const object rhs) {
        if (lhs.is_native_object()) {
            return lhs.get_native_object().overload_lesser_equality(a_environment, rhs);
        } else {
            return !(rhs > rhs);
        }
    }

    object& object::index(environment& a_environment, const object rhs) {
        switch (m_type) {
            case type::null:
                std::cout << "NULL INDEX" << std::endl;
                return null;
            case type::array:
                if (rhs.is_integer()) {
                    return get_array()[static_cast<size_t>(rhs.get_integer())];
                } else {
                    // TODO: Throw invalid operand exception.
                    return null;
                }
            case type::table:
                return get_table()[rhs];
            case type::native_object:
                return get_native_object().overload_index(a_environment, rhs);
            default:
                // TODO: Throw invalid operation exception.
                // TODO: Implement overload for native objects.
                return null;
        }
    }

    object object::select(environment& a_environment, const object rhs) {
        switch (m_type) {
            case type::string:
                if (rhs.is_integer()) {
                    return static_cast<integer>(get_string().c_str()[static_cast<size_t>(rhs.get_integer())]);
                } else if (rhs.is_string()) {
                    return a_environment.get_string_virtual_table().index(rhs);
                } else {
                    // TODO: Throw invalid operand exception.
                    return null;
                }
            case type::table:
                return get_table().index(rhs);
            case type::array:
                if (rhs.is_integer()) {
                    return get_array()[static_cast<size_t>(rhs.get_integer())];
                } else {
                    // TODO: Throw invalid operand exception.
                    return null;
                }
            case type::native_object: {
                object obj = get_native_object().overload_select(a_environment, rhs);
                return obj ? obj : get_native_object().get_virtual_table().index(rhs);
            }
            default:
                // TODO: Throw invalid operation exception.
                // TODO: Implement overload for native objects and arrays.
                return null;
        }
    }

    object object::select(environment& a_environment, const object rhs1, const object rhs2) {
        switch (m_type) {
            case type::string:
                if (rhs1.is_integer() && rhs2.is_integer()) {
                    auto target_string = get_string().to_string_view();
                    auto target_string_length = static_cast<integer>(target_string.size());

                    integer lower_bound = rhs1.get_integer();
                    lower_bound = (lower_bound < 0) ? target_string_length + lower_bound : lower_bound;

                    integer upper_bound = rhs2.get_integer();
                    upper_bound = (upper_bound < 0) ? target_string_length + upper_bound : upper_bound;

                    if (lower_bound > upper_bound) {
                        std::swap(lower_bound, upper_bound);
                    }

                    return a_environment.str(target_string.substr(lower_bound, upper_bound - lower_bound + 1));
                } else {
                    // TODO: Throw invalid operand exception.
                    return null;
                }
            case type::array:
                if (rhs1.is_integer() && rhs2.is_integer()) {
                    auto target_string = get_string().to_string_view();
                    auto target_string_length = static_cast<integer>(target_string.size());

                    integer lower_bound = rhs1.get_integer();
                    lower_bound = (lower_bound < 0) ? target_string_length + lower_bound : lower_bound;

                    integer upper_bound = rhs2.get_integer();
                    upper_bound = (upper_bound < 0) ? target_string_length + upper_bound : upper_bound;

                    if (lower_bound > upper_bound) {
                        std::swap(lower_bound, upper_bound);
                    }

                    return get_array().sub_array(lower_bound, upper_bound - lower_bound + 1);
                } else {
                    // TODO: Throw invalid operand exception.
                    return null;
                }
            case type::native_object:
                return get_native_object().overload_ranged_select(a_environment, rhs1, rhs2);
            default:
                // TODO: Throw invalid operation exception.
                // TODO: Implement overload for native objects and arrays.
                return null;
        }
    }

    object object::length(environment& a_environment) noexcept {
        switch (m_type) {
            case type::string:
                return static_cast<integer>(get_string().length());
            case type::array:
                return static_cast<integer>(get_array().size());
            case type::native_object:
                return get_native_object().overload_length(a_environment);
            default:
                return *this;
        }
    }

    void object::dereference(object& a_object) {
        switch (a_object.m_type) {
            case type::string:
                a_object.get_string().dereference();
                break;
            case type::table:
                if (--(a_object.get_table().m_reference_count) == 0) {
                    delete &a_object.get_table();
                }

                break;
            case type::array:
                get_array().dereference();
                break;
            case type::native_object:
                a_object.get_native_object().dereference();
                break;
            default:
                break;
        }
    }

    void object::reference(object& a_object) {
        switch (a_object.m_type) {
            case type::string:
                a_object.get_string().reference();
                break;
            case type::table:
                ++(a_object.get_table().m_reference_count);
                break;
            case type::array:
                get_array().reference();
                break;
            case type::native_object:
                a_object.get_native_object().reference();
                break;
            default:
                break;
        }
    }

    template <typename... t_objects>
    object function::call(t_objects&&... a_objects) {
        m_environment.inc_arg_stack();

        if constexpr (sizeof...(a_objects) > 0) {
            std::vector<object> args{{ std::forward<t_objects>(a_objects)... }};
            m_environment.set_args(args);
        }

        object res = m_environment.m_provider->call(m_data);

        m_environment.dec_arg_stack();

        return res;
    }

    object function::call(const span<object> a_objects) {
        m_environment.inc_arg_stack();
        m_environment.set_args(a_objects);

        object res = m_environment.m_provider->call(m_data);

        m_environment.dec_arg_stack();

        return res;
    }

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

        std::function<object (const parse_unit::node::expression&)> evaluate_expression;
        std::function<object (const parse_unit::node&, const node_tags)> detail_resolve_node;

        std::function<object (const parse_unit::node&)> resolve_node = [this, &evaluate_expression, &find_variable, &resolve_node, &detail_resolve_node](const parse_unit::node& a_node, const node_tags a_nodes = node_tags::none) -> object {
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

        detail_resolve_node = [this, &resolve_node](const parse_unit::node& a_node, const node_tags a_tags) -> object {
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

        std::function<object& (const parse_unit::node&)> resolve_assignable;

        std::function<object& (const parse_unit::node::expression&)> resolve_assignable_expression = [this, &local_tables, &resolve_assignable, &resolve_node, &detail_resolve_node](const parse_unit::node::expression& a_expression) -> object& {
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

        resolve_assignable = [this, &resolve_assignable_expression, &find_variable](const parse_unit::node& a_node) -> object& {
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

        evaluate_expression = [this, &resolve_node, &resolve_assignable, &detail_resolve_node](const parse_unit::node::expression& a_expression) -> object {
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

        std::function<return_state (const span<parse_unit::node>)> evaluate_block = [this, &local_tables, &evaluate_expression, &evaluate_block, &resolve_assignable_expression](const span<parse_unit::node> a_block) -> return_state {
            local_tables.emplace_back();
            table& local_table = local_tables.back();

            bool prior_eval = true;

            for (const parse_unit::node& n : a_block) {
                switch (n.m_type) {
                    case parse_unit::node::type::expression:
                        evaluate_expression(n.get_expression());
                        break;
                    case parse_unit::node::type::block: {
                        return_state state{ evaluate_block(n.get_block()) };

                        if (state.status != return_status::normal) {
                            return state;
                        }

                        break;
                    }
                    case parse_unit::node::type::if_declaration: {
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
                    case parse_unit::node::type::else_if_declaration: {
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
                    case parse_unit::node::type::else_declaration: {
                        const auto& decl = n.get_else_declaration();

                        if (!prior_eval) {
                            return_state state{ evaluate_block(decl) };

                            if (state.status != return_status::normal) {
                                return state;
                            }
                        }

                        break;
                    }
                    case parse_unit::node::type::for_declaration: {
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
                    case parse_unit::node::type::function_declaration: {
                        const auto& decl = n.get_function_declaration();

                        object& assignee = resolve_assignable_expression(decl.m_identifier);

                        auto& env_interpreter = dynamic_cast<interpreter&>(m_environment.execution_provider());

                        env_interpreter.m_function_sources.emplace_back(dynamic_cast<function_source*>(new interpreted_function_source(m_environment, decl.m_parameters, decl.m_body)));

                        assignee = function(m_environment, reinterpret_cast<void*>(env_interpreter.m_function_sources.back().get()));

                        break;
                    }
                    case parse_unit::node::type::while_declaration: {
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
                    case parse_unit::node::type::do_declaration:
                        break;
                    case parse_unit::node::type::switch_declaration:
                        break;
                    case parse_unit::node::type::class_declaration:
                        break;
                    case parse_unit::node::type::return_statement:
                        return { return_status::function_return, evaluate_expression(n.get_return_statement()) };
                    case parse_unit::node::type::break_statement:
                        return { return_status::loop_break, null };
                    case parse_unit::node::type::continue_statement:
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
            const auto& identifier = *(arg.get_operands().end() - 1);

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

#endif // #ifndef INCLUDE_REBAR_H
