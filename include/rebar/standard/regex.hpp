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

        struct regex_instance : public native_template<std::unique_ptr<RE2>> {
            REBAR_NATIVE_CLASS(regex_instance, "REBAR::STD::REGEX::ENGINE");

            REBAR_OVERLOAD_CONSTRUCT() {
                auto& expression_object = env->arg(0);

                if (expression_object.is_string()) {
                    object instance = env->create_native_object<contained_type>(class_name);
                    instance.get_native_object().get_object<contained_type>() = std::make_unique<RE2>(expression_object.get_string().to_string_view());
                    return instance;
                }

                return env->create_native_object<contained_type>(class_name);
            }

            REBAR_NATIVE_FUNCTION(FullMatch) {
                const auto& this_object = env->arg(0);
                const auto& compare_object = env->arg(1);

                auto& instance = this_object.get_native_object().get_object<contained_type>();

                if (!instance || !compare_object.is_string()) {
                    REBAR_RETURN(null);
                }

                REBAR_RETURN(object::from_bool(RE2::FullMatch(compare_object.get_string().to_string_view(), *instance)));
            }

            REBAR_NATIVE_FUNCTION(PartialMatch) {
                const auto& this_object = env->arg(0);
                const auto& compare_object = env->arg(1);

                auto& instance = this_object.get_native_object().get_object<contained_type>();

                if (!instance || !compare_object.is_string()) {
                    REBAR_RETURN(null);
                }

                REBAR_RETURN(object::from_bool(RE2::PartialMatch(compare_object.get_string().to_string_view(), *instance)));
            }

            REBAR_NATIVE_FUNCTION(Replace) {
                const auto& this_object = env->arg(0);
                const auto& string = env->arg(1);
                const auto& replace = env->arg(2);

                auto& instance = this_object.get_native_object().get_object<contained_type>();

                if (!instance || !string.is_string() || !replace.is_string()) {
                    REBAR_RETURN(null);
                }

                std::string out(string.get_string().to_string_view());

                RE2::Replace(&out, *instance, replace.get_string().to_string_view());

                REBAR_RETURN(env->str(out));
            }

            REBAR_NATIVE_FUNCTION(ReplaceAll) {
                const auto& this_object = env->arg(0);
                const auto& string = env->arg(1);
                const auto& replace = env->arg(2);

                auto& instance = this_object.get_native_object().get_object<contained_type>();

                if (!instance || !string.is_string() || !replace.is_string()) {
                    REBAR_RETURN(null);
                }

                std::string out(string.get_string().to_string_view());

                RE2::GlobalReplace(&out, *instance, replace.get_string().to_string_view());

                REBAR_RETURN(env->str(out));
            }
        };

        object load(environment& a_env) override {
            table regex_table(a_env);

            regex_instance native_regex_instance(a_env);
            regex_table["Instance"] = native_regex_instance.build_object();

            return regex_table;
        }
    };

    REBAR_DEFINE_LIBRARY("Regex", regex);
}

#endif //REBAR_REGEX_HPP
