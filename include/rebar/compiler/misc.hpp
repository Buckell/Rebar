//
// Created by maxng on 8/22/2022.
//

#ifndef REBAR_MISC_HPP
#define REBAR_MISC_HPP

#include "../compiler.hpp"

#include "../environment.hpp"

namespace rebar {
    string& compiler_function_source::emplace_string_dependency(const std::string_view a_string) {
        for (auto& dep : string_dependencies) {
            if (dep.to_string_view() == a_string) {
                return dep;
            }
        }

        return string_dependencies.emplace_back(env.str(a_string));
    }
}

#endif //REBAR_MISC_HPP
