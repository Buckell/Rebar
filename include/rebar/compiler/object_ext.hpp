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
}

#endif //REBAR_OBJECT_EXT_HPP
