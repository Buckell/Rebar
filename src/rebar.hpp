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
#include <map>
#include <string_view>
#include <vector>
#include <algorithm>

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
        Length,
        Ellipsis,
        EndStatement
    };

    enum class KeyWord : EnumBase {
        Global,
        For,
        Function,
        If,
        Else,
        TypeOf,
        While,
        Do,
        Constant,
        Switch,
        Case,
        Default,
        Break,
        Continue,
        Class,
        New
    };
    
    struct Token {
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
        constexpr Token(const Type token_type, std::in_place_type_t<Type_> in_place, Args_&&... data_args) noexcept : type(token_type), data(in_place, std::forward<Args_>(data_args)...) {}

        Token(const Token& token) noexcept : type(token.type), data(token.data) {}
        Token(Token&& token) noexcept : type(token.type), data(std::move(token.data)) {}

        Token& operator=(const Token& token) noexcept {
            type = token.type;
            data = token.data;

            return *this;
        }

        Token& operator=(Token&& token) noexcept {
            type = token.type;
            data = std::move(token.data);

            return *this;
        }

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

    template <typename Type_>
    class Optional {
        Type_* m_object;

    public:
        constexpr Optional() noexcept : m_object(nullptr) {}
        constexpr Optional(const Type_& object) noexcept(noexcept(Type_(object))) : m_object(new Type_(object)) {}
        constexpr Optional(Type_&& object) noexcept(noexcept(Type_(std::move(object)))) : m_object(new Type_(std::move(object))) {}
        constexpr Optional(Type_* object) noexcept : m_object(object) {}

        template <typename... Args_>
        constexpr Optional(std::in_place_t, Args_&&... args) noexcept(noexcept(Type_(std::forward<Args_>(args)...))) : m_object(new Type_(std::forward<Args_>(args)...)) {}

        ~Optional() {
            if (m_object != nullptr) {
                delete m_object;
            }
        }

        constexpr Optional& operator=(const Optional& optional) noexcept(noexcept(Type_(&optional.m_object))) {
            m_object = optional.m_object == nullptr ? nullptr : new Type_(*optional.m_object);

            return *this;
        }

        constexpr Optional& operator=(Optional&& optional) noexcept(noexcept(Type_(std::move(optional.m_object)))) {
            m_object = new Type_(std::move(*optional.m_object));
            optional.m_object = nullptr;

            return *this;
        }

        constexpr Optional& operator=(const Type_& object) noexcept(noexcept(Type_(object))) {
            m_object = new Type_(object);

            return *this;
        }

        constexpr Optional& operator=(Type_&& object) noexcept(noexcept(Type_(std::move(object)))) {
            m_object = new Type_(std::move(object));

            return *this;
        }

        constexpr Optional& operator=(Type_* object) noexcept {
            m_object = object;

            return *this;
        }

        [[nodiscard]] constexpr Type_* operator->() noexcept {
            return m_object;
        }

        [[nodiscard]] constexpr const Type_& operator->() const noexcept {
            return *m_object;
        }

        [[nodiscard]] constexpr Type_& operator*() noexcept {
            return *m_object;
        }

        [[nodiscard]] constexpr const Type_& operator*() const noexcept {
            return *m_object;
        }

        [[nodiscard]] constexpr Type_* Raw() noexcept {
            return m_object;
        }

        [[nodiscard]] constexpr const Type_* Raw() const noexcept {
            return m_object;
        }

        [[nodiscard]] constexpr Type_& Get() noexcept {
            return *m_object;
        }

        [[nodiscard]] constexpr const Type_& Get() const noexcept {
            return *m_object;
        }

        [[nodiscard]] constexpr bool HasValue() const noexcept {
            return m_object != nullptr;
        }
    };

    template <typename Type_>
    class OptionalView {
        Type_* m_object;

    public:
        constexpr OptionalView() noexcept : m_object(nullptr) {}
        constexpr OptionalView(Type_& o) noexcept : m_object(&o) {}
        constexpr OptionalView(Type_* o) noexcept : m_object(o) {}
        
        constexpr OptionalView& operator=(const OptionalView& optional_view) noexcept {
            m_object = optional_view.object;

            return *this;
        }

        constexpr OptionalView& operator=(Type_& o) noexcept {
            m_object = &o;

            return *this;
        }

        constexpr OptionalView& operator=(Type_* o) noexcept {
            m_object = o;

            return *this;
        }

        [[nodiscard]] constexpr OptionalView& operator->() noexcept {
            return *m_object;
        }

        [[nodiscard]] constexpr Type_& operator*() noexcept {
            return *m_object;
        }

        [[nodiscard]] constexpr Type_* Raw() noexcept {
            return m_object;
        }

        [[nodiscard]] constexpr Type_& Get() noexcept {
            return *m_object;
        }

        [[nodiscard]] constexpr bool HasValue() const noexcept {
            return m_object != nullptr;
        }
    };

    template <typename Type_>
    class HasStdSizeFunction {
        template <typename C_, typename = decltype(std::declval<C_>().size())>
        static std::true_type test(int);

        template <typename C_>
        static std::false_type test(...);

    public:
        constexpr static bool value = decltype(test<Type_>(0))::value;
    };

    template <typename Type_>
    class HasStdDataFunction {
        template <typename C_, typename = decltype(std::declval<C_>().data())>
        static std::true_type test(int);

        template <typename C_>
        static std::false_type test(...);

    public:
        constexpr static bool value = decltype(test<Type_>(0))::value;
    };

    template <typename Type_>
    struct SpanConvertible {
        static constexpr bool value = HasStdSizeFunction<Type_>::value && HasStdDataFunction<Type_>::value;
    };

    template <typename Type_>
    struct Span {
        struct Iterator {
            using iterator_category = std::random_access_iterator_tag;
            using difference_type = std::ptrdiff_t;
            using value_type = Type_;
            using pointer = Type_*;
            using reference = Type_&;

        private:
            pointer m_ptr;

        public:
            constexpr Iterator(pointer ptr) noexcept : m_ptr(ptr) {}

            constexpr Iterator& operator++() noexcept {
                ++m_ptr;
                return *this;
            }

            constexpr Iterator operator++(int) noexcept {
                Iterator it(m_ptr);
                ++m_ptr;
                return it;
            }

            constexpr Iterator& operator--() noexcept {
                --m_ptr;
                return *this;
            }

            constexpr Iterator operator--(int) noexcept {
                Iterator it(m_ptr);
                --m_ptr;
                return it;
            }

            [[nodiscard]] constexpr reference operator*() noexcept {
                return *m_ptr;
            }

            constexpr pointer operator->() noexcept {
                return m_ptr;
            }

            constexpr const pointer operator->() const noexcept {
                return m_ptr;
            }

            constexpr Iterator& operator+=(difference_type rhs) noexcept {
                m_ptr += rhs;
                return *this;
            }

            friend Iterator operator+(Iterator lhs, Iterator::difference_type rhs) noexcept;
            friend Iterator operator+(Iterator::difference_type rhs, Iterator lhs) noexcept;
        
            constexpr Iterator& operator-=(difference_type rhs) noexcept {
                m_ptr -= rhs;
                return *this;
            }

            friend Iterator operator-(Iterator lhs, Iterator::difference_type rhs) noexcept;
        
            [[nodiscard]] constexpr reference operator[](difference_type rhs) noexcept {
                return *(m_ptr + rhs);
            }

            [[nodiscard]] constexpr bool operator<(const Iterator rhs) const noexcept {
                return m_ptr < rhs.m_ptr;
            }

            [[nodiscard]] constexpr bool operator>(const Iterator rhs) const noexcept {
                return m_ptr > rhs.m_ptr;
            }

            [[nodiscard]] constexpr bool operator>=(const Iterator rhs) const noexcept {
                return m_ptr >= rhs.m_ptr;
            }

            [[nodiscard]] constexpr bool operator<=(const Iterator rhs) const noexcept {
                return m_ptr <= rhs.m_ptr;
            }

            [[nodiscard]] constexpr bool operator==(const Iterator rhs) const noexcept {
                return m_ptr == rhs.m_ptr;
            }

            [[nodiscard]] constexpr bool operator!=(const Iterator rhs) const noexcept {
                return m_ptr != rhs.m_ptr;
            }
        };

    private:
        Type_* m_data;
        size_t m_size;

    public:
        constexpr Span(Type_* data, size_t size) noexcept : m_data(data), m_size(size) {}
        
        template <size_t size_>
        constexpr Span(Type_ (&array)[size_]) noexcept : m_data(array), m_size(size_) {}

        template <size_t size_>
        constexpr Span(std::array<Type_, size_> array) noexcept : m_data(array.size()), m_size(size_) {}
    
        template <typename Container_, typename = std::enable_if<SpanConvertible<Type_>::value>>
        constexpr Span(Container_& container) noexcept(noexcept(container.size() && container.data())) : m_data(container.data()), m_size(container.size()) {}

        [[nodiscard]] constexpr Type_* GetData() noexcept {
            return m_data;
        }

        [[nodiscard]] constexpr const Type_* GetData() const noexcept {
            return m_data;
        }

        [[nodiscard]] constexpr size_t GetSize() const noexcept {
            return m_size;
        }

        [[nodiscard]] constexpr Span<Type_> SubSpan(const size_t index, const size_t size = 18446744073709551615ULL) noexcept {
            return { m_data + index, std::min(size, m_size - index) };
        }

        [[nodiscard]] constexpr const Span<Type_> SubSpan(const size_t index, const size_t size = 18446744073709551615ULL) const noexcept {
            return { m_data + index, std::min(size, m_size - index) };
        }

        [[nodiscard]] constexpr size_t size() const noexcept {
            return m_size;
        }

        [[nodiscard]] constexpr Type_* data() noexcept {
            return m_data;
        }

        [[nodiscard]] constexpr const Type_* data() const noexcept {
            return m_data;
        }

        [[nodiscard]] constexpr const Iterator cbegin() const noexcept {
            return m_data;
        }

        [[nodiscard]] constexpr Iterator begin() noexcept {
            return m_data;
        }

        [[nodiscard]] constexpr const Iterator begin() const noexcept {
            return m_data;
        }

        [[nodiscard]] constexpr const Iterator cend() const noexcept {
            return m_data + m_size;
        }

        [[nodiscard]] constexpr Iterator end() noexcept {
            return m_data + m_size;
        }

        [[nodiscard]] constexpr const Iterator end() const noexcept {
            return m_data + m_size;
        }
    };

    template <typename Type_>
    [[nodiscard]] constexpr typename Span<Type_>::Iterator operator+(typename Span<Type_>::Iterator lhs, typename Span<Type_>::Iterator::difference_type rhs) noexcept {
        return Iterator(lhs.m_ptr + rhs);
    }

    template <typename Type_>
    [[nodiscard]] constexpr typename Span<Type_>::Iterator operator+(typename Span<Type_>::Iterator::difference_type rhs, typename Span<Type_>::Iterator lhs) noexcept {
        return Iterator(lhs.m_ptr + rhs);
    }

    template <typename Type_>
    [[nodiscard]] constexpr typename Span<Type_>::Iterator operator-(typename Span<Type_>::Iterator lhs, typename Span<Type_>::Iterator::difference_type rhs) noexcept {
        return Iterator(lhs.m_ptr - rhs);
    }

    class SymbolMap {
    public:
        using MapType = std::map<std::string_view, Token>;
        using PairType = std::pair<std::string_view, Token>;

    private:
        MapType m_tokens;

    public:
        SymbolMap() noexcept : m_tokens() {}
        
        template <size_t Size_>
        constexpr SymbolMap(const PairType (&array)[Size_]) noexcept : m_tokens(array) {}
        
        template <size_t Size_>
        constexpr SymbolMap(const std::array<PairType, Size_>& array) noexcept : m_tokens(array) {}
        
        SymbolMap(const std::vector<PairType>& map) noexcept : m_tokens(map.begin(), map.end()) {}

        SymbolMap(const SymbolMap& symbol_token_map) noexcept : m_tokens(symbol_token_map.m_tokens) {}
        SymbolMap(SymbolMap&& symbol_token_map) noexcept : m_tokens(std::move(symbol_token_map.m_tokens)) {}

        constexpr SymbolMap& operator=(const SymbolMap& symbol_token_map) noexcept {
            m_tokens = symbol_token_map.m_tokens;
            
            return *this;
        }

        constexpr SymbolMap& operator=(SymbolMap&& symbol_token_map) noexcept {
            m_tokens = std::move(symbol_token_map.m_tokens);
            
            return *this;
        }

        template <typename... Args_>
        constexpr void Emplace(const std::string_view symbol, Args_&&... args) noexcept {
            m_tokens.emplace(symbol, Token(std::forward<Args_>(args)...));
        }

        [[nodiscard]] OptionalView<Token> Get(const std::string_view symbol) noexcept {
            MapType::iterator it{ m_tokens.find(symbol) };
            return (it != m_tokens.end()) ? OptionalView<Token>(it->second) : OptionalView<Token>();
        }

        [[nodiscard]] OptionalView<const Token> Get(const std::string_view symbol) const noexcept {
            MapType::const_iterator it{ m_tokens.find(symbol) };
            return (it != m_tokens.end()) ? OptionalView<const Token>(it->second) : OptionalView<const Token>();
        }

        [[nodiscard]] Optional<std::pair<std::string_view, const Token&>> NextToken(const std::string_view text) const noexcept {
            for (const auto& pair : m_tokens) {
                if (pair.first == text.substr(0, pair.first.size())) {
                    return Optional<std::pair<std::string_view, const Token&>>(std::in_place, pair.first, pair.second);
                }
            }

            return Optional<std::pair<std::string_view, const Token&>>();
        }

        [[nodiscard]] static SymbolMap GetDefault() noexcept {
            return {{
                { " ",   { Token::Type::Operator, Operator::Space } },
                { "\n",  { Token::Type::Operator, Operator::Space } },
                { "\t",  { Token::Type::Operator, Operator::Space } },
                { "=",   { Token::Type::Operator, Operator::Assignment } },
                { "*",   { Token::Type::Operator, Operator::Multiplication } },
                { "*=",  { Token::Type::Operator, Operator::MultiplicationAssignment } },
                { "/",   { Token::Type::Operator, Operator::Division } },
                { "/=",  { Token::Type::Operator, Operator::DivisionAssignment } },
                { "+",   { Token::Type::Operator, Operator::Addition } },
                { "+=",  { Token::Type::Operator, Operator::AdditionAssignment } },
                { "-",   { Token::Type::Operator, Operator::Subtraction } },
                { "-=",  { Token::Type::Operator, Operator::SubtractionAssignment } },
                { "++",  { Token::Type::Operator, Operator::Increment } },
                { "--",  { Token::Type::Operator, Operator::Decrement } },
                { "(",   { Token::Type::Operator, Operator::GroupOpen } },
                { ")",   { Token::Type::Operator, Operator::GroupClose } },
                { "[",   { Token::Type::Operator, Operator::SelectorOpen } },
                { "]",   { Token::Type::Operator, Operator::SelectorClose } },
                { "{",   { Token::Type::Operator, Operator::ScopeOpen } },
                { "}",   { Token::Type::Operator, Operator::ScopeClose } },
                { "==",  { Token::Type::Operator, Operator::Equality } },
                { "!=",  { Token::Type::Operator, Operator::InverseEquality } },
                { ">",   { Token::Type::Operator, Operator::Greater } },
                { "<",   { Token::Type::Operator, Operator::Lesser } },
                { ">=",  { Token::Type::Operator, Operator::GreaterEquality } },
                { "<=",  { Token::Type::Operator, Operator::LesserEquality } },
                { "||",  { Token::Type::Operator, Operator::LogicalOR } },
                { "or",  { Token::Type::Operator, Operator::LogicalOR } },
                { "&&",  { Token::Type::Operator, Operator::LogicalAND } },
                { "and", { Token::Type::Operator, Operator::LogicalAND } },
                { "!",   { Token::Type::Operator, Operator::LogicalNOT } },
                { "not", { Token::Type::Operator, Operator::LogicalNOT } },
                { "|",   { Token::Type::Operator, Operator::BitwiseOR } },
                { "|=",  { Token::Type::Operator, Operator::BitwiseORAssignment } },
                { ">|",  { Token::Type::Operator, Operator::BitwiseXOR } },
                { ">|=", { Token::Type::Operator, Operator::BitwiseXORAssignment } },
                { "&",   { Token::Type::Operator, Operator::BitwiseAND } },
                { "&=",  { Token::Type::Operator, Operator::BitwiseANDAssignment } },
                { "~",   { Token::Type::Operator, Operator::BitwiseNOT } },
                { ">>",  { Token::Type::Operator, Operator::ShiftRight } },
                { ">>=", { Token::Type::Operator, Operator::ShiftRightAssignment } },
                { "<<",  { Token::Type::Operator, Operator::ShiftLeft } },
                { "<<=", { Token::Type::Operator, Operator::ShiftLeftAssignment } },
                { "^",   { Token::Type::Operator, Operator::Exponent } },
                { "^=",  { Token::Type::Operator, Operator::ExponentAssignment } },
                { "%",   { Token::Type::Operator, Operator::Modulus } },
                { "%=",  { Token::Type::Operator, Operator::ModulusAssignment } },
                { ":",   { Token::Type::Operator, Operator::Seek } },
                { "?",   { Token::Type::Operator, Operator::Ternary } },
                { ".",   { Token::Type::Operator, Operator::Dot } },
                { ",",   { Token::Type::Operator, Operator::List } },
                { "->",  { Token::Type::Operator, Operator::Direct } },
                { "#",   { Token::Type::Operator, Operator::Length } },
                { "...", { Token::Type::Operator, Operator::Ellipsis } },
                { ";",   { Token::Type::Operator, Operator::EndStatement } },
                
                { "global",   { Token::Type::KeyWord, KeyWord::Global } },
                { "for",      { Token::Type::KeyWord, KeyWord::For } },
                { "function", { Token::Type::KeyWord, KeyWord::Function } },
                { "if",       { Token::Type::KeyWord, KeyWord::If } },
                { "else",     { Token::Type::KeyWord, KeyWord::Else } },
                { "typeof",   { Token::Type::KeyWord, KeyWord::TypeOf } },
                { "while",    { Token::Type::KeyWord, KeyWord::While } },
                { "do",       { Token::Type::KeyWord, KeyWord::Do } },
                { "const",    { Token::Type::KeyWord, KeyWord::Constant } },
                { "switch",   { Token::Type::KeyWord, KeyWord::Switch } },
                { "case",     { Token::Type::KeyWord, KeyWord::Case } },
                { "default",  { Token::Type::KeyWord, KeyWord::Default } },
                { "break",    { Token::Type::KeyWord, KeyWord::Break } },
                { "continue", { Token::Type::KeyWord, KeyWord::Continue } },
                { "class",    { Token::Type::KeyWord, KeyWord::Class } },
                { "new",      { Token::Type::KeyWord, KeyWord::New } }
            }};
        }
    };

    enum class FunctionTags : EnumBase {
        Basic,
        Constant,
        Global,
        GlobalConstant
    };

    enum class ClassTags : EnumBase {
        Basic,
        Global
    };

    struct ParseUnit {
        struct Node {
            enum class Type : EnumBase {
                Empty,
                Token,
                Statement,
                Block,
                Group,
                Selector,
                RangedSelector,
                ArgumentList,
                IfDeclaration,
                ElseIfDeclaration,
                ElseDeclaration,
                ForDeclaration,
                FunctionDeclaration,
                WhileDeclaration,
                DoWhileDeclaration,
                SwitchDeclaration,
                ClassDeclaration
            };

            using Statement = std::vector<Node>;
            using Block = std::vector<Node>;
            using Group = std::vector<Node>;
            using Selector = std::vector<Node>;
            using RangedSelector = std::pair<Selector, Selector>;
            using ArgumentList = std::vector<Node>;

            struct IfDeclaration {
                Group m_conditional;
                Block m_body;

                IfDeclaration(Group conditional, Block body) noexcept : m_conditional(std::move(conditional)), m_body(std::move(body)) {}

                IfDeclaration(const IfDeclaration& decl) noexcept : m_conditional(decl.m_conditional), m_body(decl.m_body) {}
                IfDeclaration(IfDeclaration&& decl) noexcept : m_conditional(std::move(decl.m_conditional)), m_body(std::move(decl.m_body)) {}
            };

            using ElseDeclaration = Block;

            struct ForDeclaration {
                Statement m_initialization;
                Group m_conditional;
                Statement m_iteration;
                Block m_body;

                ForDeclaration(Statement initialization, Group conditional, Statement iteration, Block body) noexcept : 
                    m_initialization(initialization), m_conditional(conditional), m_iteration(iteration), m_body(body) {}
            
                ForDeclaration(const ForDeclaration& decl) noexcept : m_initialization(decl.m_initialization), 
                    m_conditional(decl.m_conditional), m_iteration(decl.m_iteration), m_body(decl.m_body) {}
                    
                ForDeclaration(ForDeclaration&& decl) noexcept : m_initialization(decl.m_initialization), 
                    m_conditional(decl.m_conditional), m_iteration(decl.m_iteration), m_body(decl.m_body) {}
            };

            struct FunctionDeclaration {
                std::string_view m_identifier;
                FunctionTags m_tags;
                ArgumentList m_parameters;
                Block m_body;

                FunctionDeclaration(const std::string_view identifier, const FunctionTags tags, ArgumentList parameters, Block body) noexcept : 
                    m_identifier(std::move(identifier)), m_tags(std::move(tags)), m_parameters(std::move(parameters)), m_body(std::move(body)) {}

                FunctionDeclaration(const FunctionDeclaration& decl) noexcept : m_identifier(decl.m_identifier), m_tags(decl.m_tags), 
                    m_parameters(decl.m_parameters), m_body(decl.m_body) {}

                FunctionDeclaration(FunctionDeclaration&& decl) noexcept : m_identifier(decl.m_identifier), m_tags(decl.m_tags), 
                    m_parameters(std::move(decl.m_parameters)), m_body(std::move(decl.m_body)) {}
            };
            
            using WhileDeclaration = IfDeclaration;
            using DoWhileDeclaration = IfDeclaration;

            struct SwitchDeclaration {
                struct CaseDeclaration {
                    bool m_ranged;
                    std::pair<Group, Group> m_data;

                    CaseDeclaration(Group group) noexcept : m_ranged(false), m_data(std::move(group), Group()) {}
                    CaseDeclaration(Group begin, Group end) noexcept : m_ranged(true), m_data(std::move(begin), std::move(end)) {}

                    CaseDeclaration(const CaseDeclaration& decl) noexcept : m_ranged(decl.m_ranged), m_data(decl.m_data) {}
                    CaseDeclaration(CaseDeclaration&& decl) noexcept : m_ranged(decl.m_ranged), m_data(std::move(decl.m_data)) {}
                };

                Group m_expression;
                std::vector<CaseDeclaration> m_cases;

                SwitchDeclaration(Group expression, std::vector<CaseDeclaration> cases) noexcept : m_expression(std::move(expression)), m_cases(std::move(cases)) {}
            
                SwitchDeclaration(const SwitchDeclaration& decl) noexcept : m_expression(decl.m_expression), m_cases(decl.m_cases) {}
                SwitchDeclaration(SwitchDeclaration&& decl) noexcept : m_expression(std::move(decl.m_expression)), m_cases(std::move(decl.m_cases)) {}
            };

            struct ClassDeclaration {
                std::string_view m_identifier;
                ClassTags m_tags;
                std::vector<FunctionDeclaration> m_functions;

                ClassDeclaration(const std::string_view identifier, const ClassTags tags, std::vector<FunctionDeclaration> functions) noexcept : m_identifier(identifier), m_tags(tags), m_functions(std::move(functions)) {}

                ClassDeclaration(const ClassDeclaration& decl) noexcept : m_identifier(decl.m_identifier), m_tags(decl.m_tags), m_functions(decl.m_functions) {}
                ClassDeclaration(ClassDeclaration& decl) noexcept : m_identifier(decl.m_identifier), m_tags(decl.m_tags), m_functions(std::move(decl.m_functions)) {}
            };

            using DataType = std::variant<std::nullptr_t, Token*, std::vector<Node>, IfDeclaration, ForDeclaration, FunctionDeclaration, SwitchDeclaration, ClassDeclaration>;

            Type m_type;
            DataType m_data;

            Node() noexcept : m_type(Type::Empty), m_data(nullptr) {}
            Node(const Type type, DataType data) noexcept : m_type(type), m_data(std::move(data)) {}
            
            template <typename Type_, typename... Args_>
            Node(const Type type, std::in_place_type_t<Type_> in_place, Args_... args) noexcept : m_type(type), m_data(in_place, std::forward<Args_>(args)...) {}

            Node(const Node& node) noexcept : m_type(node.m_type), m_data(std::move(node.m_data)) {}
            Node(Node&& node) noexcept : m_type(node.m_type), m_data(std::move(node.m_data)) {}
        };

        std::string m_plaintext;
        std::vector<Token> tokens;
        Node::Block block;
    };

    class Parser {
        SymbolMap m_map;

    public:
        Parser() noexcept : m_map(SymbolMap::GetDefault()) {}
        Parser(SymbolMap map) noexcept : m_map(std::move(map)) {}

        void SetSymbolMap(SymbolMap map) noexcept {
            m_map = std::move(map);
        }

        [[nodiscard]] std::vector<Token> Lex(const std::string_view string) {
            std::vector<Token> tokens;

            bool string_mode = false;
            bool identifier_mode = true;
            bool escape_mode = false;

            size_t string_start_index = 0;
            size_t identifier_start_index = 0;

            size_t scan_index = 0;
            while (scan_index < string.size()) {
                char character = string[scan_index];
                
                if (string_mode) {
                    if (escape_mode) {
                        escape_mode = false;
                    } else if (character == '"') {
                        string_mode = false;
                        tokens.emplace_back(Token::Type::StringLiteral, std::in_place_type<std::string_view>, string.substr(string_start_index, scan_index - string_start_index));
                    }
                    
                    escape_mode = character == '\\';
                    scan_index++;
                } else if (character == '"') {
                    if (identifier_mode) {
                        identifier_mode = false;
                        tokens.emplace_back(Token::Type::Identifier, std::in_place_type<std::string_view>, string.substr(identifier_start_index, scan_index - identifier_start_index));
                    }

                    string_mode = true;
                    string_start_index = ++scan_index;
                } else {
                    Optional<std::pair<std::string_view, const Token&>> token = m_map.NextToken(string.substr(scan_index));

                    if (token.HasValue()) {
                        if (identifier_mode) {
                            identifier_mode = false;
                            tokens.emplace_back(Token::Type::Identifier, std::in_place_type<std::string_view>, string.substr(identifier_start_index, scan_index - identifier_start_index));
                        }

                        tokens.push_back(token->second);
                        scan_index += token->first.size();
                    } else {
                        identifier_start_index = identifier_mode ? identifier_start_index : scan_index;
                        identifier_mode = true;
                    }
                }
            }

            if (identifier_mode) {
                tokens.emplace_back(Token::Type::Identifier, std::in_place_type<std::string_view>, string.substr(identifier_start_index, scan_index - identifier_start_index));
            }

            tokens.erase(std::remove_if(tokens.begin(), tokens.end(), [](const Token& token) noexcept -> bool {
                return token == Operator::Space;
            }), tokens.end());

            return tokens;
        }

        [[nodiscard]] ParseUnit Parse(std::string string) noexcept {
            ParseUnit unit { std::move(string), Lex(string), ParseBlock(unit.tokens) };

            

            return unit;
        }

    private:
        [[nodiscard]] ParseUnit::Node::Block ParseBlock(const Span<Token> tokens) {
            std::vector<ParseUnit::Node> nodes;
            
            return nodes;
        }
    };
}
