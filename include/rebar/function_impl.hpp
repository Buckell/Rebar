//
// Created by maxng on 6/21/2022.
//

#ifndef REBAR_FUNCTION_IMPL_HPP
#define REBAR_FUNCTION_IMPL_HPP

#include <array>

#include "function.hpp"

#include "object.hpp"
#include "environment.hpp"

namespace rebar {
    template <typename... t_objects>
    object function::call_v(t_objects&&... a_objects) {
        std::array<object, sizeof...(a_objects)> temp{ std::forward<t_objects...>(a_objects)... };

        m_environment.set_arguments_pointer(temp.data());
        m_environment.set_argument_count(temp.size());

        return m_environment.m_provider->call(m_data);
    }

    object function::call(const span<object> a_objects) {
        m_environment.set_arguments_pointer(a_objects.data());
        m_environment.set_argument_count(a_objects.size());

        return m_environment.m_provider->call(m_data);
    }
}

#endif //REBAR_FUNCTION_IMPL_HPP
