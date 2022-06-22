//
// Created by maxng on 6/21/2022.
//

#ifndef REBAR_OPERATOR_PRECEDENCE_HPP
#define REBAR_OPERATOR_PRECEDENCE_HPP

namespace rebar {
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
}

#endif //REBAR_OPERATOR_PRECEDENCE_HPP
