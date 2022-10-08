//
// Created by maxng on 9/22/2022.
//

#ifndef REBAR_EXCEPTION_IMPL_HPP
#define REBAR_EXCEPTION_IMPL_HPP

#include "exception.hpp"
#include "environment.hpp"

namespace rebar {
    runtime_exception::runtime_exception(environment& a_env)
        : exception("RUNTIME", "", 0), m_environment(a_env) { get_message(); }

    std::string runtime_exception::generate_message() noexcept {
        return m_environment.generate_exception_message();
    }
}

#endif //REBAR_EXCEPTION_IMPL_HPP
