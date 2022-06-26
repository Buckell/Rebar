//
// Created by maxng on 6/25/2022.
//

#ifndef REBAR_BASE_HPP
#define REBAR_BASE_HPP

#include "../rebar.hpp"

namespace rebar::library::standard {
    struct base : public library {
        base() : library(usage::implicit_include) {}

        static rebar::object PrintLn(rebar::environment* env) {
            if (env->arg_count() == 0) {
                std::cout << '\n';
            }

            std::cout << env->arg(0);

            for (size_t i = 1; i < env->arg_count(); i++) {
                std::cout << "    " << env->arg(i);
            }

            std::cout << '\n';

            return rebar::null;
        }

        static rebar::object Print(rebar::environment* env) {
            std::cout << env->arg(0);

            for (size_t i = 1; i < env->arg_count(); i++) {
                std::cout << "    " << env->arg(i);
            }

            return rebar::null;
        }

        static rebar::object Include(rebar::environment* env) {
            std::string_view sv = env->arg(0).get_string().to_string_view();
            return load_library(*env, sv);
        }

        static rebar::object Input(rebar::environment* env) {
            std::string input;
            std::getline(std::cin, input);
            return env->str(input);
        }

        object load(environment& a_environment) override {
            auto& global_table = a_environment.global_table();

            const auto define_global_function = [&a_environment, &global_table](const std::string_view a_identifier, callable a_function) noexcept {
                global_table[a_environment.str(a_identifier)] = a_environment.bind(a_function);
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
