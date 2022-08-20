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
        std::array<object, 16> temp = m_environment.get_args();

        if constexpr (sizeof...(a_objects) > 0) {
            std::vector<object> args{{ std::forward<t_objects>(a_objects)... }};
            m_environment.set_args(args);
        }

        object res = m_environment.m_provider->call(m_data);

        m_environment.set_args(temp);

        return res;
    }

    object function::call(const span<object> a_objects) {
        std::array<object, 16> temp = m_environment.get_args();

        m_environment.set_args(a_objects);

        object res = m_environment.m_provider->call(m_data);

        m_environment.set_args(temp);

        return res;
    }
}

#endif //REBAR_FUNCTION_IMPL_HPP
