//
// Created by maxng on 6/21/2022.
//

#ifndef REBAR_DEFINITIONS_HPP
#define REBAR_DEFINITIONS_HPP

#include <type_traits>
#include <string_view>

#define DEFINE_REBAR_FLAG(flag, number) constexpr static size_t flag = (number == 0) ? 0x0 : 0x1 << (number - 1)

namespace rebar {
    using integer = std::conditional_t<sizeof(void*) == 8, int64_t, int32_t>;
    using number = std::conditional_t<sizeof(void*) == 8, double, float>;
    using enum_base = size_t;

    static_assert(sizeof(integer) == sizeof(void*));
    static_assert(sizeof(number) == sizeof(void*));

    using flags = size_t;

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

    class object;
    class environment;

    using callable = void (*)(object*, environment*);

    struct platform {
        constexpr static size_t windows = 0x1;
        constexpr static size_t apple = 0x1 << 1;
        constexpr static size_t linux = 0x1 << 2;
        constexpr static size_t x64 = 0x1 << 3;
        constexpr static size_t x86 = 0x1 << 4;
#if defined(REBAR_PLATFORM_WINDOWS)
        constexpr static size_t current = windows | (sizeof(void*) == 8 ? x64 : x86);
#elif defined(REBAR_PLATFORM_LINUX)
        constexpr static size_t current = linux | (sizeof(void*) == 8 ? x64 : x86);
#elif defined(REBAR_PLATFORM_APPLE)
        constexpr static size_t current = apple | (sizeof(void*) == 8 ? x64 : x86);
#endif
    };

#ifdef REBAR_DEBUG
    constexpr bool debug_mode = true;
#else
    constexpr bool debug_mode = false;
#endif

    constexpr size_t object_data_offset = sizeof(size_t);

    constexpr size_t default_argument_allocation = 16;
}

#endif //REBAR_DEFINITIONS_HPP
