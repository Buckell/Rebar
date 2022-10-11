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

        object load(environment& a_environment) override {
            auto& array_table = a_environment.get_array_virtual_table();

            const auto define_array_function = [&a_environment, &array_table](const std::string_view a_identifier, callable a_function) noexcept {
                array_table[a_environment.str(a_identifier)] = a_environment.bind(a_function, std::string(a_identifier), { { "CLASS", "REBAR::STD::ARRAY_BASE" } });
            };

            define_array_function("Size", Size);

            return null;
        }
    };
}

#endif //REBAR_ARRAY_BASE_HPP
