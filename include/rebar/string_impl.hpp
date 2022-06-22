//
// Created by maxng on 6/21/2022.
//

#ifndef REBAR_STRING_IMPL_HPP
#define REBAR_STRING_IMPL_HPP

#include "string.hpp"

#include "environment.hpp"

namespace rebar {
    string::string(environment& a_env, const std::string_view a_string) : m_root_pointer(a_env.str(a_string).m_root_pointer) {}
}

#endif //REBAR_STRING_IMPL_HPP
