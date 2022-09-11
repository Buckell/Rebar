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
        object rhs(a_rhs_type, a_rhs_data);
        *a_lhs_return = object::add(*a_env, *a_lhs_return, rhs);
    }
}

#endif //REBAR_OBJECT_EXT_HPP
