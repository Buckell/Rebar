//
// Created by maxng on 9/14/2022.
//

#ifndef REBAR_REGEX_HPP
#define REBAR_REGEX_HPP

#include "re2/re2.h"

#include "rebar/library.hpp"

namespace rebar::library::standard {
    struct regex : public library {
        regex() : library(usage::explicit_include) {}

        void load_regex_instance(environment& a_environment, rebar::virtual_table& a_regex_instance) {
            a_regex_instance.overload_new = [](environment* env, native_object o) -> rebar::object {
                auto& expression_object = env->arg(0);

                if (expression_object.is_string()) {
                    object instance = env->create_native_object<std::unique_ptr<RE2>>("REBAR::STD::REGEX::ENGINE");
                    instance.get_native_object().get_object<std::unique_ptr<RE2>>() = std::make_unique<RE2>(expression_object.get_string().to_string_view());
                    return instance;
                }

                return env->create_native_object<std::unique_ptr<RE2>>("REBAR::STD::REGEX::ENGINE");
            };

            a_regex_instance[a_environment.str("FullMatch")] = a_environment.bind([](object* ret, environment* env) -> void {
                const auto& this_object = env->arg(0);
                const auto& compare_object = env->arg(1);

                auto& instance = this_object.get_native_object().get_object<std::unique_ptr<RE2>>();

                if (!instance || !compare_object.is_string()) {
                    REBAR_RETURN(null);
                }

                REBAR_RETURN(object::from_bool(RE2::FullMatch(compare_object.get_string().to_string_view(), *instance)));
            }, "FullMatch", { { "CLASS", "REBAR::STD::REGEX::ENGINE" } });

            a_regex_instance[a_environment.str("PartialMatch")] = a_environment.bind([](object* ret, environment* env) -> void {
                const auto& this_object = env->arg(0);
                const auto& compare_object = env->arg(1);

                auto& instance = this_object.get_native_object().get_object<std::unique_ptr<RE2>>();

                if (!instance || !compare_object.is_string()) {
                    REBAR_RETURN(null);
                }

                REBAR_RETURN(object::from_bool(RE2::PartialMatch(compare_object.get_string().to_string_view(), *instance)));
            }, "PartialMatch", { { "CLASS", "REBAR::STD::REGEX::ENGINE" } });

            a_regex_instance[a_environment.str("Replace")] = a_environment.bind([](object* ret, environment* env) -> void {
                const auto& this_object = env->arg(0);
                const auto& string = env->arg(1);
                const auto& replace = env->arg(2);

                auto& instance = this_object.get_native_object().get_object<std::unique_ptr<RE2>>();

                if (!instance || !string.is_string() || !replace.is_string()) {
                    REBAR_RETURN(null);
                }

                std::string out(string.get_string().to_string_view());

                RE2::Replace(&out, *instance, replace.get_string().to_string_view());

                REBAR_RETURN(env->str(out));
            }, "Replace", { { "CLASS", "REBAR::STD::REGEX::ENGINE" } });

            a_regex_instance[a_environment.str("ReplaceAll")] = a_environment.bind([](object* ret, environment* env) -> void {
                const auto& this_object = env->arg(0);
                const auto& string = env->arg(1);
                const auto& replace = env->arg(2);

                auto& instance = this_object.get_native_object().get_object<std::unique_ptr<RE2>>();

                if (!instance || !string.is_string() || !replace.is_string()) {
                    REBAR_RETURN(null);
                }

                std::string out(string.get_string().to_string_view());

                RE2::GlobalReplace(&out, *instance, replace.get_string().to_string_view());

                REBAR_RETURN(env->str(out));
            }, "ReplaceAll", { { "CLASS", "REBAR::STD::REGEX::ENGINE" } });
        }

        object load(environment& a_environment) override {
            table* tbl = new table;

            rebar::virtual_table& regex_instance_virtual_table = a_environment.register_native_class("REBAR::STD::REGEX::ENGINE");
            load_regex_instance(a_environment, regex_instance_virtual_table);
            native_object n_obj = a_environment.create_native_object<std::unique_ptr<RE2>>(regex_instance_virtual_table);
            (*tbl)[a_environment.str("Instance")] = n_obj;

            return tbl;
        }
    };

    REBAR_DEFINE_LIBRARY("Regex", regex);
}

#endif //REBAR_REGEX_HPP
