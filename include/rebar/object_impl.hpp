//
// Created by maxng on 6/21/2022.
//

#ifndef REBAR_OBJECT_IMPL_HPP
#define REBAR_OBJECT_IMPL_HPP

#include "object.hpp"

#include "environment.hpp"
#include "span.hpp"

namespace rebar {
    template <typename... t_objects>
    object object::call_v(environment& a_environment, t_objects&&... a_objects) {
        switch (m_type) {
            case type::function:
                return get_function(a_environment).call(std::forward<t_objects>(a_objects)...);
            case type::native_object: {
                std::array<object, sizeof...(a_objects)> temp{ std::forward<t_objects...>(a_objects)... };

                a_environment.set_arguments_pointer(temp.data());
                a_environment.set_argument_count(temp.size());

                return get_native_object().overload_call(a_environment);
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
                a_environment.set_arguments_pointer(a_objects.data());
                a_environment.set_argument_count(a_objects.size());

                return get_native_object().overload_call(a_environment);
            }
            default:
                return {};
        }
    }

    template <typename... t_objects>
    object object::new_object_v(environment& a_environment, t_objects&&... a_objects) {
        switch (m_type) {
            case type::native_object: {
                std::array<object, sizeof...(a_objects)> temp{ std::forward<t_objects...>(a_objects)... };

                a_environment.set_arguments_pointer(temp.data());
                a_environment.set_argument_count(temp.size());

                return get_native_object().overload_new(a_environment);
            }
            default:
                return {};
        }
    }

    object object::new_object(environment& a_environment, const span<rebar::object> a_objects) {
        switch (m_type) {
            case type::native_object: {
                a_environment.set_arguments_pointer(a_objects.data());
                a_environment.set_argument_count(a_objects.size());

                return get_native_object().overload_new(a_environment);
            }
            default:
                return {};
        }
    }

    std::string object::to_string() const noexcept {
        using namespace std::string_literals;

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
                return std::string(get_string().to_string_view());
            case type::array:
                return get_array().to_string();
            case type::native_object:
                return "NATIVE_OBJECT";
            case type::function:
                return "Function<"s + std::to_string(m_data) + ">";
            case type::table:
                return "Table<"s + std::to_string(m_data) + ">";
            default:
                return std::to_string(m_data);
        }
    }

    object object::add(environment& a_environment, const object& lhs, const object& rhs) {
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

    object object::multiply(environment& a_environment, const object& lhs, const object& rhs) {
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

    object object::subtract(environment& a_environment, const object& lhs, const object& rhs) {
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

    object object::divide(environment& a_environment, const object& lhs, const object& rhs) {
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

    object object::modulus(environment& a_environment, const object& lhs, const object& rhs) {
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

    object object::exponentiate(environment& a_environment, const object& lhs, const object& rhs) {
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

    object object::equals(environment& a_environment, const object& lhs, const object& rhs) {
        if (lhs.m_type != rhs.m_type) {
            return boolean_false;
        } else if (lhs.is_simply_comparable()) {
            return object::from_bool(lhs.m_data == rhs.m_data);
        } else {
            switch (lhs.m_type) {
                case type::native_object:
                    return lhs.get_native_object().overload_equality(a_environment, rhs);
                default:
                    return boolean_false;
            }

            // TODO: Implement comparisons for complexly comparable types.
        }
    }

    object object::not_equals(environment& a_environment, const object& lhs, const object& rhs) {
        if (lhs.m_type != rhs.m_type) {
            return boolean_true;
        }

        if (lhs.is_simply_comparable()) {
            return lhs.m_data != rhs.m_data;
        } else {
            switch (lhs.m_type) {
                case type::native_object:
                    return lhs.get_native_object().overload_inverse_equality(a_environment, rhs);
                default:
                    return boolean_false;
            }

            // TODO: Implement comparisons for complexly comparable types.
        }
    }

    object object::shift_left(environment& a_environment, const object& lhs, const object& rhs) {
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

    object object::shift_right(environment& a_environment, const object& lhs, const object& rhs) {
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

    object object::bitwise_xor(environment& a_environment, const object& lhs, const object& rhs) {
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

    object object::bitwise_or(environment& a_environment, const object& lhs, const object& rhs) {
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

    object object::bitwise_and(environment& a_environment, const object& lhs, const object& rhs) {
        if ((lhs.is_integer() || lhs.is_number()) && (rhs.is_integer() || rhs.is_number())) {
            return { lhs.m_type, lhs.m_data & rhs.m_data };
        } else {
            if (lhs.is_native_object()) {
                return lhs.get_native_object().overload_bitwise_and(a_environment, rhs);
            }

            // TODO: Throw invalid operation exception.
            // TODO: Provide overload for native objects.
            return null;
        }
    }

    object object::bitwise_not(environment& a_environment, const object& lhs) {
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

    object object::logical_not(environment& a_environment, const object& lhs) {
        if (lhs.is_native_object()) {
            return lhs.get_native_object().overload_logical_not(a_environment);
        } else {
            return object::from_bool(!lhs.boolean_evaluate());
        }
    }

    object object::logical_or(environment& a_environment, const object& lhs, const object& rhs) {
        if (lhs.is_native_object()) {
            return lhs.get_native_object().overload_logical_or(a_environment, rhs);
        } else {
            return lhs.boolean_evaluate() ? lhs : rhs;
        }
    }

    object object::logical_and(environment& a_environment, const object& lhs, const object& rhs) {
        if (lhs.is_native_object()) {
            return lhs.get_native_object().overload_logical_and(a_environment, rhs);
        } else {
            return lhs.boolean_evaluate() && rhs.boolean_evaluate() ? rhs : object();
        }
    }

    object object::greater_than(environment& a_environment, const object& lhs, const object& rhs) {
        if (lhs.is_integer()) {
            if (rhs.is_integer()) {
                return object::from_bool(lhs.get_integer() > rhs.get_integer());
            } else if (rhs.is_number()) {
                return object::from_bool(lhs.get_integer() > rhs.get_number());
            } else if (rhs.is_string()) {
                return object::from_bool(lhs.get_integer() > rhs.get_string().length());
            }
        } else if (lhs.is_number()) {
            if (rhs.is_integer()) {
                return object::from_bool(lhs.get_number() > rhs.get_integer());
            } else if (rhs.is_number()) {
                return object::from_bool(lhs.get_number() > rhs.get_number());
            }
        } else if (lhs.is_string()) {
            if (rhs.is_integer()) {
                return object::from_bool(lhs.get_string().length() > rhs.get_integer());
            }
        } else if (lhs.is_native_object()) {
            return lhs.get_native_object().overload_greater(a_environment, rhs);
        }

        // TODO: Throw invalid operation exception.

        return boolean_false;
    }

    object object::lesser_than(environment& a_environment, const object& lhs, const object& rhs) {
        if (lhs.is_integer()) {
            if (rhs.is_integer()) {
                return object::from_bool(lhs.get_integer() < rhs.get_integer());
            } else if (rhs.is_number()) {
                return object::from_bool(lhs.get_integer() < rhs.get_number());
            } else if (rhs.is_string()) {
                return object::from_bool(lhs.get_integer() < rhs.get_string().length());
            }
        } else if (lhs.is_number()) {
            if (rhs.is_integer()) {
                return object::from_bool(lhs.get_number() < rhs.get_integer());
            } else if (rhs.is_number()) {
                return object::from_bool(lhs.get_number() < rhs.get_number());
            }
        } else if (lhs.is_string()) {
            if (rhs.is_integer()) {
                return object::from_bool(lhs.get_string().length() < rhs.get_integer());
            }
        } else if (lhs.is_native_object()) {
            return lhs.get_native_object().overload_lesser(a_environment, rhs);
        }

        // TODO: Throw invalid operation exception.

        return boolean_false;
    }

    object object::greater_than_equal_to(environment& a_environment, const object& lhs, const object& rhs) {
        if (lhs.is_integer()) {
            if (rhs.is_integer()) {
                return object::from_bool(lhs.get_integer() >= rhs.get_integer());
            } else if (rhs.is_number()) {
                return object::from_bool(lhs.get_integer() >= rhs.get_number());
            } else if (rhs.is_string()) {
                return object::from_bool(lhs.get_integer() >= rhs.get_string().length());
            }
        } else if (lhs.is_number()) {
            if (rhs.is_integer()) {
                return object::from_bool(lhs.get_number() >= rhs.get_integer());
            } else if (rhs.is_number()) {
                return object::from_bool(lhs.get_number() >= rhs.get_number());
            }
        } else if (lhs.is_string()) {
            if (rhs.is_integer()) {
                return object::from_bool(lhs.get_string().length() >= rhs.get_integer());
            }
        } else if (lhs.is_native_object()) {
            return lhs.get_native_object().overload_greater_equality(a_environment, rhs);
        }

        // TODO: Throw invalid operation exception.

        return boolean_false;
    }

    object object::lesser_than_equal_to(environment& a_environment, const object& lhs, const object& rhs) {
        if (lhs.is_integer()) {
            if (rhs.is_integer()) {
                return object::from_bool(lhs.get_integer() <= rhs.get_integer());
            } else if (rhs.is_number()) {
                return object::from_bool(lhs.get_integer() <= rhs.get_number());
            } else if (rhs.is_string()) {
                return object::from_bool(lhs.get_integer() <= rhs.get_string().length());
            }
        } else if (lhs.is_number()) {
            if (rhs.is_integer()) {
                return object::from_bool(lhs.get_number() <= rhs.get_integer());
            } else if (rhs.is_number()) {
                return object::from_bool(lhs.get_number() <= rhs.get_number());
            }
        } else if (lhs.is_string()) {
            if (rhs.is_integer()) {
                return object::from_bool(lhs.get_string().length() <= rhs.get_integer());
            }
        } else if (lhs.is_native_object()) {
            return lhs.get_native_object().overload_lesser_equality(a_environment, rhs);
        }

        // TODO: Throw invalid operation exception.

        return boolean_false;
    }

    object& object::prefix_increment(environment& a_environment) {
        switch (m_type) {
            case type::null:
                break;
            case type::boolean:
            case type::integer: {
                integer result = get_integer() + 1;
                m_data = *reinterpret_cast<size_t*>(&result);
                break;
            }
            case type::function:
                break;
            case type::number: {
                number result = get_number() + 1;
                m_data = *reinterpret_cast<size_t*>(&result);
                break;
            }
            case type::string:
            case type::table:
            case type::array:
                break;
            case type::native_object:
                get_native_object().overload_prefix_increment(a_environment);
                break;
        }

        return *this;
    }

    object& object::prefix_decrement(environment& a_environment) {
        switch (m_type) {
            case type::null:
                break;
            case type::boolean:
            case type::integer: {
                integer result = get_integer() - 1;
                m_data = *reinterpret_cast<size_t*>(&result);
                break;
            }
            case type::function:
                break;
            case type::number: {
                number result = get_number() - 1;
                m_data = *reinterpret_cast<size_t*>(&result);
                break;
            }
            case type::string:
            case type::table:
            case type::array:
                break;
            case type::native_object:
                get_native_object().overload_prefix_decrement(a_environment);
                break;
        }

        return *this;
    }

    object object::postfix_increment(environment& a_environment) {
        switch (m_type) {
            case type::boolean:
            case type::integer: {
                integer result = get_integer() + 1;
                m_data = *reinterpret_cast<size_t*>(&result);
                return result - 1;
            }
            case type::number: {
                number result = get_number() + 1;
                m_data = *reinterpret_cast<size_t*>(&result);
                return result - 1;
            }
            case type::native_object:
                return get_native_object().overload_postfix_increment(a_environment);
            default:
                return null;
        }
    }

    object object::postfix_decrement(environment& a_environment) {
        switch (m_type) {
            case type::boolean:
            case type::integer: {
                integer result = get_integer() - 1;
                m_data = *reinterpret_cast<size_t*>(&result);
                return result + 1;
            }
            case type::number: {
                number result = get_number() - 1;
                m_data = *reinterpret_cast<size_t*>(&result);
                return result + 1;
            }
            case type::native_object:
                return get_native_object().overload_postfix_decrement(a_environment);
            default:
                return null;
        }
    }

    object& object::index(environment& a_environment, const object& rhs) {
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

    object object::select(environment& a_environment, const object& rhs) {
        switch (m_type) {
            case type::string:
                if (rhs.is_integer()) {
                    return static_cast<integer>(get_string().c_str()[static_cast<size_t>(rhs.get_integer())]);
                } else {
                    return a_environment.get_string_virtual_table().index(rhs);
                }
            case type::table:
                return get_table().index(rhs);
            case type::array:
                if (rhs.is_integer()) {
                    return get_array()[static_cast<size_t>(rhs.get_integer())];
                } else {
                    return a_environment.get_array_virtual_table().index(rhs);
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

    object object::select(environment& a_environment, const object& rhs1, const object& rhs2) {
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
                a_object.get_array().dereference();
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
                a_object.get_array().reference();
                break;
            case type::native_object:
                a_object.get_native_object().reference();
                break;
            default:
                break;
        }
    }

    size_t object::get_reference_count(object& a_object) {
        switch (a_object.object_type()) {
            case type::string:
                return a_object.get_string().reference_count();
            case type::table:
                return a_object.get_table().m_reference_count;
            case type::array:
                return a_object.get_array().get_reference_count();
            case type::native_object:
                return a_object.get_native_object().get_reference_count();
            default:
                return 0;
        }
    }

    void object::block_dereference(object* a_objects, size_t a_object_count) {
        for (size_t i = 0; i < a_object_count; ++i) {
            object& obj = a_objects[i];

            if (obj.m_type > object::simple_type_end_boundary) {
                switch (obj.m_type) {
                    case type::string:
                        obj.get_string().dereference();
                        break;
                    case type::table:
                        if (--(obj.get_table().m_reference_count) == 0) {
                            delete &obj.get_table();
                        }

                        break;
                    case type::array:
                        obj.get_array().dereference();
                        break;
                    case type::native_object:
                        obj.get_native_object().dereference();
                        break;
                    default:
                        break;
                }
            }
        }
    }
}

#endif //REBAR_OBJECT_IMPL_HPP
