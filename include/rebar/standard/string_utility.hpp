//
// Created by maxng on 6/25/2022.
//

#ifndef REBAR_STRING_UTILITY_HPP
#define REBAR_STRING_UTILITY_HPP

#include "rebar/library.hpp"

namespace rebar::library::standard {
    struct string_utility : public library {
        string_utility() : library(usage::explicit_include) {}

        struct string_builder : public native_template<std::vector<std::string>> {
            REBAR_NATIVE_CLASS(string_builder, "REBAR::STD::STRING_UTILITY::STRING_BUILDER");

            REBAR_OVERLOAD_CONSTRUCT() {
                return env->create_native_object<contained_type>(class_name);
            }

            REBAR_OVERLOAD_ADDITION_ASSIGNMENT() {
                self.get_object<contained_type>().push_back(rhs.to_string());
            }

            REBAR_NATIVE_FUNCTION(ToString) {
                object this_object = env->arg(0);

                auto& strings = this_object.get_native_object().get_object<contained_type>();

                size_t size = 0;

                for (const auto& str : strings) {
                    size += str.length();
                }

                std::string final;
                final.reserve(size);

                for (const auto& str : strings) {
                    final += str;
                }

                REBAR_RETURN(env->str(final));
            }

            REBAR_NATIVE_FUNCTION(Append) {
                object this_object = env->arg(0);
                object obj = env->arg(1);

                this_object.get_native_object().get_object<contained_type>().push_back(obj.to_string());

                REBAR_RETURN(null);
            }
        };

        object load(environment& a_environment) override {
            table string_utility(a_environment);

            string_builder native_string_builder(a_environment);
            string_utility["StringBuilder"] = native_string_builder.build_object();

            return string_utility;
        }
    };

    REBAR_DEFINE_LIBRARY("StringUtility", string_utility);
}

#endif //REBAR_STRING_UTILITY_HPP
