//
// Created by maxng on 8/27/2022.
//

#ifndef REBAR_OBJECT_EXT_HPP
#define REBAR_OBJECT_EXT_HPP

namespace rebar {
    void _ext_object_select(environment* a_env, object* a_lhs_return, type a_rhs_type, size_t a_rhs_data) {
        *a_lhs_return = std::move(a_lhs_return->select(*a_env, object(a_rhs_type, a_rhs_data)));
    }

    object* _ext_object_index(environment* a_env, object* a_lhs_return, type a_rhs_type, size_t a_rhs_data) {
        return &a_lhs_return->index(*a_env, object(a_rhs_type, a_rhs_data));
    }

    void _ext_object_new(environment* a_env, object* a_lhs_return) {
        *a_lhs_return = a_lhs_return->new_object(*a_env, span<object>(a_env->get_arguments_pointer(), a_env->arg_count()));
    }

    void _ext_object_add(environment* a_env, object* a_lhs_return, type a_rhs_type, size_t a_rhs_data) {
        *a_lhs_return = object::add(*a_env, *a_lhs_return, { a_rhs_type, a_rhs_data });
    }

    void _ext_object_subtract(environment* a_env, object* a_lhs_return, type a_rhs_type, size_t a_rhs_data) {
        *a_lhs_return = object::subtract(*a_env, *a_lhs_return, { a_rhs_type, a_rhs_data });
    }

    void _ext_object_multiply(environment* a_env, object* a_lhs_return, type a_rhs_type, size_t a_rhs_data) {
        *a_lhs_return = object::multiply(*a_env, *a_lhs_return, { a_rhs_type, a_rhs_data });
    }

    void _ext_object_divide(environment* a_env, object* a_lhs_return, type a_rhs_type, size_t a_rhs_data) {
        *a_lhs_return = object::divide(*a_env, *a_lhs_return, { a_rhs_type, a_rhs_data });
    }

    void _ext_object_bitwise_or(environment* a_env, object* a_lhs_return, type a_rhs_type, size_t a_rhs_data) {
        *a_lhs_return = object::bitwise_or(*a_env, *a_lhs_return, { a_rhs_type, a_rhs_data });
    }

    void _ext_object_bitwise_xor(environment* a_env, object* a_lhs_return, type a_rhs_type, size_t a_rhs_data) {
        *a_lhs_return = object::bitwise_xor(*a_env, *a_lhs_return, { a_rhs_type, a_rhs_data });
    }

    void _ext_object_bitwise_and(environment* a_env, object* a_lhs_return, type a_rhs_type, size_t a_rhs_data) {
        *a_lhs_return = object::bitwise_and(*a_env, *a_lhs_return, { a_rhs_type, a_rhs_data });
    }

    void _ext_object_bitwise_not(environment* a_env, object* a_lhs_return) {
        *a_lhs_return = object::bitwise_not(*a_env, *a_lhs_return);
    }

    void _ext_object_shift_right(environment* a_env, object* a_lhs_return, type a_rhs_type, size_t a_rhs_data) {
        *a_lhs_return = object::shift_right(*a_env, *a_lhs_return, { a_rhs_type, a_rhs_data });
    }

    void _ext_object_shift_left(environment* a_env, object* a_lhs_return, type a_rhs_type, size_t a_rhs_data) {
        *a_lhs_return = object::shift_left(*a_env, *a_lhs_return, {a_rhs_type, a_rhs_data});
    }

    void _ext_object_exponentiate(environment* a_env, object* a_lhs_return, type a_rhs_type, size_t a_rhs_data) {
        *a_lhs_return = object::exponentiate(*a_env, *a_lhs_return, {a_rhs_type, a_rhs_data});
    }

    void _ext_object_modulus(environment* a_env, object* a_lhs_return, type a_rhs_type, size_t a_rhs_data) {
        *a_lhs_return = object::modulus(*a_env, *a_lhs_return, {a_rhs_type, a_rhs_data});
    }

    void _ext_object_length(environment* a_env, object* a_lhs_return) {
        *a_lhs_return = a_lhs_return->length(*a_env);
    }

    void _ext_object_prefix_increment(environment* a_env, object* a_lhs) {
        a_lhs->prefix_increment(*a_env);
    }

    void _ext_object_prefix_decrement(environment* a_env, object* a_lhs) {
        a_lhs->prefix_decrement(*a_env);
    }

    void _ext_object_postfix_increment(environment* a_env, object* a_return, object* a_lhs) {
        *a_return = a_lhs->postfix_increment(*a_env);
    }

    void _ext_object_postfix_decrement(environment* a_env, object* a_return, object* a_lhs) {
        *a_return = a_lhs->postfix_decrement(*a_env);
    }

    void _ext_object_equals(environment* a_env, object* a_lhs_return, type a_rhs_type, size_t a_rhs_data) {
        *a_lhs_return = object::equals(*a_env, *a_lhs_return, { a_rhs_type, a_rhs_data });
    }

    void _ext_object_not_equals(environment* a_env, object* a_lhs_return, type a_rhs_type, size_t a_rhs_data) {
        *a_lhs_return = object::not_equals(*a_env, *a_lhs_return, { a_rhs_type, a_rhs_data });
    }
}

#endif //REBAR_OBJECT_EXT_HPP
