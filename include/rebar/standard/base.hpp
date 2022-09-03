//
// Created by maxng on 6/25/2022.
//

#ifndef REBAR_BASE_HPP
#define REBAR_BASE_HPP

#include "macro.hpp"

#include "../rebar.hpp"

namespace rebar::library::standard {
    struct base : public library {
        base() : library(usage::implicit_include) {}

        static REBAR_FUNCTION(PrintLn) {
            auto& out = env->cout();

            if (env->arg_count() == 0) {
                out << std::endl;
                *ret = null;
                return;
            }

            out << env->arg(0);

            for (size_t i = 1; i < env->arg_count(); i++) {
                out << " " << env->arg(i);
            }

            out << std::endl;

            *ret = null;
        }

        static REBAR_FUNCTION(Print) {
            auto& out = env->cout();

            out << env->arg(0);

            for (size_t i = 1; i < env->arg_count(); i++) {
                out << " " << env->arg(i);
            }

            out << std::flush;

            *ret = rebar::null;
        }

        static REBAR_FUNCTION(Include) {
            std::string_view sv = env->arg(0).get_string().to_string_view();
            *ret = load_library(*env, sv);
        }

        static REBAR_FUNCTION(Input) {
            std::string input;
            std::getline(env->cin(), input);

            *ret = env->str(input);
        }

        object load(environment& a_environment) override {
            auto& global_table = a_environment.global_table();

            const auto define_global_function = [&a_environment, &global_table](const std::string_view a_identifier, callable a_function) noexcept {
                global_table[a_environment.str(a_identifier)] = a_environment.bind(a_function, std::string(a_identifier), "REBAR::STD::BASE");
            };

            define_global_function("PrintLn", PrintLn);
            define_global_function("Print", Print);
            define_global_function("Include", Include);
            define_global_function("Input", Input);

            return null;
        }
    };

    const char library_base[] = "Base";
    define_library<library_base, base> d_base;
}

#endif //REBAR_BASE_HPP
