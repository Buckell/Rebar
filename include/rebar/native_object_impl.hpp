//
// Created by maxng on 6/21/2022.
//

#ifndef REBAR_NATIVE_OBJECT_IMPL_HPP
#define REBAR_NATIVE_OBJECT_IMPL_HPP

#include "native_object.hpp"

#include "object.hpp"
#include "table.hpp"

namespace rebar {
    struct virtual_table : public table {
        template <size_t v_arguments>
        using assignment_operation_function = std::conditional_t<v_arguments == 0, void (*)(environment*, native_object),
        std::conditional_t<v_arguments == 1, void (*)(environment*, native_object, const object&),
                std::conditional_t<v_arguments == 2, void (*)(environment*, native_object, const object&, const object&),
                void>>>;

        template <size_t v_arguments>
        using operation_function = std::conditional_t<v_arguments == 0, object (*)(environment*, native_object),
        std::conditional_t<v_arguments == 1, object (*)(environment*, native_object, const object&),
                std::conditional_t<v_arguments == 2, object (*)(environment*, native_object, const object&, const object&),
                void>>>;

        using index_function = object& (*)(environment*, native_object, const object&);

        // TODO: Throw exceptions in null functions.
        constexpr static const assignment_operation_function<0> null_aof_0 = [](environment*, native_object) {};
        constexpr static const assignment_operation_function<1> null_aof_1 = [](environment*, native_object, const object&) {};
        constexpr static const assignment_operation_function<2> null_aof_2 = [](environment*, native_object, const object&, const object&) {};

        constexpr static const operation_function<0> null_of_0 = [](environment*, native_object) -> object { return null; };
        constexpr static const operation_function<1> null_of_1 = [](environment*, native_object, const object&) -> object { return null; };
        constexpr static const operation_function<2> null_of_2 = [](environment*, native_object, const object&, const object&) -> object { return null; };

        constexpr static const index_function null_idx = [](environment*, native_object, const object&) -> object& { static object temp; return temp; };

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

    inline void native_object::overload_assignment(environment& a_env, const object& rhs) {
        this->get_virtual_table().overload_assignment(&a_env, *this, rhs);
    }

    [[nodiscard]] inline object native_object::overload_addition(environment& a_env, const object& rhs) {
        return this->get_virtual_table().overload_addition(&a_env, *this, rhs);
    }

    inline void native_object::overload_addition_assignment(environment& a_env, const object& rhs) {
        this->get_virtual_table().overload_addition_assignment(&a_env, *this, rhs);
    }

    [[nodiscard]] inline object native_object::overload_multiplication(environment& a_env, const object& rhs) {
        return this->get_virtual_table().overload_multiplication(&a_env, *this, rhs);
    }

    inline void native_object::overload_multiplication_assignment(environment& a_env, const object& rhs) {
        this->get_virtual_table().overload_multiplication_assignment(&a_env, *this, rhs);
    }

    [[nodiscard]] inline object native_object::overload_division(environment& a_env, const object& rhs) {
        return this->get_virtual_table().overload_division(&a_env, *this, rhs);
    }

    inline void native_object::overload_division_assignment(environment& a_env, const object& rhs) {
        this->get_virtual_table().overload_division_assignment(&a_env, *this, rhs);
    }

    [[nodiscard]] inline object native_object::overload_subtraction(environment& a_env, const object& rhs) {
        return this->get_virtual_table().overload_subtraction(&a_env, *this, rhs);
    }

    inline void native_object::overload_subtraction_assignment(environment& a_env, const object& rhs) {
        this->get_virtual_table().overload_subtraction_assignment(&a_env, *this, rhs);
    }

    [[nodiscard]] inline object native_object::overload_equality(environment& a_env, const object& rhs) {
        return this->get_virtual_table().overload_equality(&a_env, *this, rhs);
    }

    [[nodiscard]] inline object native_object::overload_inverse_equality(environment& a_env, const object& rhs) {
        return this->get_virtual_table().overload_inverse_equality(&a_env, *this, rhs);
    }

    [[nodiscard]] inline object native_object::overload_greater(environment& a_env, const object& rhs) {
        return this->get_virtual_table().overload_greater(&a_env, *this, rhs);
    }

    [[nodiscard]] inline object native_object::overload_lesser(environment& a_env, const object& rhs) {
        return this->get_virtual_table().overload_lesser(&a_env, *this, rhs);
    }

    [[nodiscard]] inline object native_object::overload_greater_equality(environment& a_env, const object& rhs) {
        return this->get_virtual_table().overload_greater_equality(&a_env, *this, rhs);
    }

    [[nodiscard]] inline object native_object::overload_lesser_equality(environment& a_env, const object& rhs) {
        return this->get_virtual_table().overload_lesser_equality(&a_env, *this, rhs);
    }

    [[nodiscard]] inline object native_object::overload_logical_or(environment& a_env, const object& rhs) {
        return this->get_virtual_table().overload_logical_or(&a_env, *this, rhs);
    }

    [[nodiscard]] inline object native_object::overload_logical_and(environment& a_env, const object& rhs) {
        return this->get_virtual_table().overload_logical_and(&a_env, *this, rhs);
    }

    [[nodiscard]] inline object native_object::overload_logical_not(environment& a_env) {
        return this->get_virtual_table().overload_logical_not(&a_env, *this);
    }

    [[nodiscard]] inline object native_object::overload_bitwise_or(environment& a_env, const object& rhs) {
        return this->get_virtual_table().overload_bitwise_or(&a_env, *this, rhs);
    }

    inline void native_object::overload_bitwise_or_assignment(environment& a_env, const object& rhs) {
        this->get_virtual_table().overload_bitwise_or_assignment(&a_env, *this, rhs);
    }

    [[nodiscard]] inline object native_object::overload_bitwise_xor(environment& a_env, const object& rhs) {
        return this->get_virtual_table().overload_bitwise_xor(&a_env, *this, rhs);
    }

    inline void native_object::overload_bitwise_xor_assignment(environment& a_env, const object& rhs) {
        this->get_virtual_table().overload_bitwise_xor_assignment(&a_env, *this, rhs);
    }

    [[nodiscard]] inline object native_object::overload_bitwise_and(environment& a_env, const object& rhs) {
        return this->get_virtual_table().overload_bitwise_and(&a_env, *this, rhs);
    }

    inline void native_object::overload_bitwise_and_assignment(environment& a_env, const object& rhs) {
        this->get_virtual_table().overload_bitwise_and_assignment(&a_env, *this, rhs);
    }

    [[nodiscard]] inline object native_object::overload_bitwise_not(environment& a_env) {
        return this->get_virtual_table().overload_bitwise_not(&a_env, *this);
    }

    [[nodiscard]] inline object native_object::overload_shift_right(environment& a_env, const object& rhs) {
        return this->get_virtual_table().overload_shift_right(&a_env, *this, rhs);
    }

    inline void native_object::overload_shift_right_assignment(environment& a_env, const object& rhs) {
        this->get_virtual_table().overload_shift_right_assignment(&a_env, *this, rhs);
    }

    [[nodiscard]] inline object native_object::overload_shift_left(environment& a_env, const object& rhs) {
        return this->get_virtual_table().overload_shift_left(&a_env, *this, rhs);
    }

    inline void native_object::overload_shift_left_assignment(environment& a_env, const object& rhs) {
        this->get_virtual_table().overload_shift_left_assignment(&a_env, *this, rhs);
    }

    [[nodiscard]] inline object native_object::overload_exponent(environment& a_env, const object& rhs) {
        return this->get_virtual_table().overload_exponent(&a_env, *this, rhs);
    }

    inline void native_object::overload_exponent_assignment(environment& a_env, const object& rhs) {
        this->get_virtual_table().overload_exponent_assignment(&a_env, *this, rhs);
    }

    [[nodiscard]] inline object native_object::overload_modulus(environment& a_env, const object& rhs) {
        return this->get_virtual_table().overload_modulus(&a_env, *this, rhs);
    }

    inline void native_object::overload_modulus_assignment(environment& a_env, const object& rhs) {
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

    [[nodiscard]] inline object& native_object::overload_index(environment& a_env, const object& rhs) {
        return this->get_virtual_table().overload_operation_index(&a_env, *this, rhs);
    }

    [[nodiscard]] inline object native_object::overload_select(environment& a_env, const object& rhs) {
        return this->get_virtual_table().overload_operation_select(&a_env, *this, rhs);
    }

    [[nodiscard]] inline object native_object::overload_ranged_select(environment& a_env, const object& lhs, const object& rhs) {
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

#endif //REBAR_NATIVE_OBJECT_IMPL_HPP
