//
// Created by maxng on 6/21/2022.
//

#ifndef REBAR_OBJECT_HPP
#define REBAR_OBJECT_HPP

#include "definitions.hpp"
#include "string.hpp"
#include "array.hpp"
#include "function.hpp"
#include "native_object.hpp"

namespace rebar {
    class table;

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
        object(const type a_type, const size_t a_data) noexcept : m_type(a_type), m_data(a_data) {
            if (a_type > simple_type_end_boundary) {
                reference(*this);
            }
        }

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
            if (a_object.is_simple_type()) {
                m_type = a_object.m_type;
                m_data = a_object.m_data;
            } else {
                dereference(*this);

                m_type = a_object.m_type;
                m_data = a_object.m_data;

                reference(*this);
            }

            return *this;
        }

        object& operator = (object&& a_object) noexcept {
            m_type = a_object.m_type;
            m_data = a_object.m_data;

            a_object.m_type = type::null;
            a_object.m_data = 0;

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
        object call_v(environment& a_environment, t_objects&&... a_objects);
        object call(environment& a_environment, span<object> a_objects);

        template <typename... t_objects>
        object new_object_v(environment& a_environment, t_objects&&... a_objects);
        object new_object(environment& a_environment, span<object> a_objects);

        [[nodiscard]] object& index(environment& a_environment, const object& rhs);
        [[nodiscard]] object select(environment& a_environment, const object& rhs);
        [[nodiscard]] object select(environment& a_environment, const object& rhs1, const object& rhs2);

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
            lhs << rhs.to_string();
            return lhs;
        }

        [[nodiscard]] operator bool() const noexcept {
            return boolean_evaluate();
        }

        // TODO: Complete dereference function.
        static void dereference(object& a_object);
        static void reference(object& a_object);
        static size_t get_reference_count(object& a_object);

        static void block_dereference(object* a_objects, size_t a_object_count);

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
}

template <>
struct std::hash <rebar::object> {
    size_t operator()(const rebar::object s) const noexcept {
        return s.data();
    }
};

#endif //REBAR_OBJECT_HPP
