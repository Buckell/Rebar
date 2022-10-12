//
// Created by maxng on 6/25/2022.
//

#ifndef REBAR_BASE_HPP
#define REBAR_BASE_HPP

#include "../rebar.hpp"

namespace rebar::library::standard {
    struct base : public library {
        base() : library(usage::implicit_include) {}

        static REBAR_FUNCTION(PrintLn) {
            auto& out = env->cout();

            if (env->arg_count() == 0) {
                out << std::endl;
                REBAR_RETURN(null);
            }

            out << env->arg(0);

            for (size_t i = 1; i < env->arg_count(); i++) {
                out << " " << env->arg(i);
            }

            out << std::endl;

            REBAR_RETURN(null);
        }

        static REBAR_FUNCTION(Print) {
            auto& out = env->cout();

            out << env->arg(0);

            for (size_t i = 1; i < env->arg_count(); i++) {
                out << " " << env->arg(i);
            }

            out << std::flush;

            REBAR_RETURN(null);
        }

        static REBAR_FUNCTION(Include) {
            std::string_view sv = env->arg(0).get_string().to_string_view();
            REBAR_RETURN(load_library(*env, sv));
        }

        static REBAR_FUNCTION(Input) {
            std::string input;
            std::getline(env->cin(), input);

            REBAR_RETURN(env->str(input));
        }

        object load(environment& a_env) override {
            auto global_table = a_env.global_table();

            std::map<std::string, std::string> omap{ { "BASE", "REBAR::STD::BASE" } };

            global_table["PrintLn"] = a_env.bind(PrintLn, "PrintLn", omap);
            global_table["Print"]   = a_env.bind(Print, "Print", omap);
            global_table["Include"] = a_env.bind(Include, "Include", omap);
            global_table["Input"]   = a_env.bind(Input, "Input", omap);

            return null;
        }
    };

    REBAR_DEFINE_LIBRARY("Base", base);
}

#endif //REBAR_BASE_HPP
