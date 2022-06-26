//
// Created by maxng on 6/25/2022.
//

#ifndef REBAR_LIBRARY_HPP
#define REBAR_LIBRARY_HPP

#include <map>
#include <string_view>

#include "environment.hpp"

namespace rebar::library {
    enum class usage {
        implicit_include,
        explicit_include
    };

    struct library {
        usage include;

        library(usage a_include) noexcept : include(a_include) {}

        virtual object load(environment&) = 0;
    };

    std::map<std::string_view, library*> registry;

    template <const char* v_identifier, class t_library>
    struct define_library {
        define_library() {
            registry[v_identifier] = new t_library;
        }
    };

    object load_library(environment& a_environment, const std::string_view a_identifier) {
        return registry[a_identifier]->load(a_environment);
    }

    void load_implicit_libraries(environment& a_environment) {
        for (auto& lib : registry) {
            if (lib.second->include == usage::implicit_include) {
                lib.second->load(a_environment);
            }
        }
    }
}

#endif //REBAR_LIBRARY_HPP
