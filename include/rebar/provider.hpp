//
// Created by maxng on 6/21/2022.
//

#ifndef REBAR_PROVIDER_HPP
#define REBAR_PROVIDER_HPP

#include "definitions.hpp"
#include "function.hpp"
#include "object.hpp"
#include "preprocess.hpp"

namespace rebar {
    struct provider {
        [[nodiscard]] virtual function compile(parse_unit& a_unit) = 0;
        [[nodiscard]] virtual function bind(callable a_function) = 0;
        [[nodiscard]] virtual object call(const void* a_data) = 0;
    };
}

#endif //REBAR_PROVIDER_HPP
