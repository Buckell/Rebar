//
// Created by maxng on 6/21/2022.
//

#ifndef REBAR_FUNCTION_HPP
#define REBAR_FUNCTION_HPP

#include <type_traits>

#include "span.hpp"

namespace rebar {
    class environment;

    class function {
        friend class object;

        environment& m_environment;
        const void* m_data;

    public:
        function(environment& a_environment, const void* a_data) noexcept : m_environment(a_environment), m_data(a_data) {}

        template <typename... t_objects>
        object call(t_objects&&... a_objects);
        object call(const span<object> a_objects);

        template <typename... t_objects>
        auto operator () (t_objects&&... a_objects) {
            return call(std::forward<t_objects>(a_objects)...);
        }
    };
}

#endif //REBAR_FUNCTION_HPP
