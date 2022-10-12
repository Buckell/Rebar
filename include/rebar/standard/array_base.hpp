//
// Created by maxng on 9/13/2022.
//

#ifndef REBAR_ARRAY_BASE_HPP
#define REBAR_ARRAY_BASE_HPP

#include "../rebar.hpp"

namespace rebar::library::standard {
    struct array_base : public library {
        array_base() : library(usage::implicit_include) {}

        static REBAR_FUNCTION(Size) {
            auto& self = env->arg(0).get_array();
            REBAR_RETURN(static_cast<integer>(self.size()));
        }

        object load(environment& a_env) override {
            table array_table{ a_env, a_env.get_array_virtual_table() };

            std::map<std::string, std::string> omap{ { "CLASS", "REBAR::STD::ARRAY_BASE" } };

            array_table["Size"] = a_env.bind(Size, "Size", omap);

            return null;
        }
    };
}

#endif //REBAR_ARRAY_BASE_HPP
