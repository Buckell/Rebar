/*
Copyright (C) 2021 Max Goddard

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <type_traits>
#include <variant>
#include <unordered_map>
#include <string_view>

namespace Rebar {
    using Integer = std::conditional_t<sizeof(size_t) == 8, int64_t, int32_t>;
    using Float = std::conditional_t<sizeof(size_t) == 8, double, float>;
    using EnumBase = size_t;

    static_assert(sizeof(Integer) == sizeof(size_t));
    static_assert(sizeof(Float) == sizeof(size_t));

    enum class Operator : EnumBase {
        Space,
        Assignment,
        Multiplication,
        MultiplicationAssignment,
        Division,
        DivisionAssignment,
        Addition,
        AdditionAssignment,
        Subtraction,
        SubtractionAssignment,
        Increment,
        Decrement,
        GroupOpen,
        GroupClose,
        SelectorOpen,
        SelectorClose,
        ScopeOpen,
        ScopeClose,
        Equality,
        InverseEquality,
        Greater,
        Lesser,
        GreaterEquality,
        LesserEquality,
        LogicalOR,
        LogicalAND,
        LogicalNOT,
        BitwiseOR,
        BitwiseORAssignment,
        BitwiseXOR,
        BitwiseXORAssignment,
        BitwiseAND,
        BitwiseANDAssignment,
        BitwiseNOT,
        ShiftRight,
        ShiftRightAssignment,
        ShiftLeft,
        ShiftLeftAssignment,
        Exponent,
        ExponentAssignment,
        Modulus,
        ModulusAssignment,
        Seek,
        Ternary,
        Dot,
        List,
        Direct,
        EndStatement
    };

    enum class KeyWord : EnumBase {
        Function,
        For,
        While,
        Do
    };
    
    struct [[nodiscard]] Token {
        enum class Type : EnumBase {
            Operator,
            KeyWord,
            StringLiteral,
            Identifier,
            IntegerLiteral,
            FloatLiteral
        };

        using Data = std::variant<Operator, KeyWord, std::string_view, Integer, Float>;

        Type type;
        Data data;

        Token() = delete;
        Token(const Type token_type, Data token_data) noexcept : type(token_type), data(std::move(token_data)) {}
        
        template <typename Type_, typename... Args_>
        constexpr Token(const Type token_type, std::in_place_type_t<Type_>, Args_&&... data_args) noexcept : type(token_type), 
            data(type == Type::Operator ? std::in_place_type<Operator> :
            (type == Type::KeyWord ? std::in_place_type<KeyWord> :
            ((type == Type::StringLiteral || type == Type::Identifier) ? std::in_place_type<std::string_view> :
            (type == Type::IntegerLiteral ? std::in_place_type<Integer> :
            (type == Type::FloatLiteral ? std::in_place_type<Float> : std::in_place_type<void>)))), std::forward<Args_>(data_args)...) {}

        [[nodiscard]] constexpr bool IsOperator() const noexcept {
            return type == Type::Operator;
        }

        [[nodiscard]] constexpr bool IsKeyWord() const noexcept {
            return type == Type::KeyWord;
        }

        [[nodiscard]] constexpr bool IsStringLiteral() const noexcept {
            return type == Type::StringLiteral;
        }

        [[nodiscard]] constexpr bool IsIdentifier() const noexcept {
            return type == Type::Identifier;
        }

        [[nodiscard]] constexpr bool IsIntegerLiteral() const noexcept {
            return type == Type::IntegerLiteral;
        }

        [[nodiscard]] constexpr bool IsFloatLiteral() const noexcept {
            return type == Type::FloatLiteral;
        }

        [[nodiscard]] constexpr Operator GetOperator() const noexcept {
            return std::get<Operator>(data);
        }

        [[nodiscard]] constexpr KeyWord GetKeyWord() const noexcept {
            return std::get<KeyWord>(data);
        }

        [[nodiscard]] constexpr std::string_view GetStringLiteral() const noexcept {
            return std::get<std::string_view>(data);
        }

        [[nodiscard]] constexpr std::string_view GetIdentifier() const noexcept {
            return std::get<std::string_view>(data);
        }

        [[nodiscard]] constexpr Integer GetIntegerLiteral() const noexcept {
            return std::get<Integer>(data);
        }

        [[nodiscard]] constexpr Float GetFloatLiteral() const noexcept {
            return std::get<Float>(data);
        }

        [[nodiscard]] constexpr bool operator==(const Operator op) const noexcept {
            return IsOperator() && GetOperator() == op;
        }

        [[nodiscard]] constexpr bool operator==(const KeyWord keyword) const noexcept {
            return IsKeyWord() && GetKeyWord() == keyword;
        }

        [[nodiscard]] constexpr bool operator==(const std::string_view string) const noexcept {
            return (IsStringLiteral() || IsIdentifier()) && GetStringLiteral() == string;
        }

        [[nodiscard]] constexpr bool operator==(const Integer integer) const noexcept {
            return IsIntegerLiteral() && GetIntegerLiteral() == integer;
        }

        [[nodiscard]] constexpr bool operator==(const Float floating_point) const noexcept {
            return IsFloatLiteral() && GetFloatLiteral() == floating_point;
        }
    };
}