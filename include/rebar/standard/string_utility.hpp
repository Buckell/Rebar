//
// Created by maxng on 6/25/2022.
//

#ifndef REBAR_STRING_UTILITY_HPP
#define REBAR_STRING_UTILITY_HPP

#include "rebar/library.hpp"

namespace rebar::library::standard {
    struct string_utility : public library {
        string_utility() : library(usage::explicit_include) {}

        void load_string_builder(environment& a_environment, rebar::virtual_table& a_string_builder) {
            a_string_builder.overload_new = [](environment* env, native_object) -> rebar::object {
                return env->create_native_object<std::vector<std::string>>("REBAR::STD::STRING_UTILITY::STRING_BUILDER");
            };

            a_string_builder.overload_addition_assignment = [](environment* env, native_object a_this, object& rhs) {
                a_this.get_object<std::vector<std::string>>().push_back(rhs.to_string());
            };

            a_string_builder[a_environment.str("ToString")] = a_environment.bind([](object* ret, environment* env) -> void {
                object this_object = env->arg(0);

                auto& strings = this_object.get_native_object().get_object<std::vector<std::string>>();

                size_t size = 0;

                for (const auto& str : strings) {
                    size += str.length();
                }

                std::string final;
                final.reserve(size);

                for (const auto& str : strings) {
                    final += str;
                }

                *ret = env->str(final);
            }, "ToString", "REBAR::STD::STRING_UTILITY::STRING_BUILDER");

            a_string_builder[a_environment.str("Append")] = a_environment.bind([](object* ret, environment* env) -> void {
                object this_object = env->arg(0);
                object obj = env->arg(1);

                this_object.get_native_object().get_object<std::vector<std::string>>().push_back(obj.to_string());

                *ret = null;
            }, "Append", "REBAR::STD::STRING_UTILITY::STRING_BUILDER");
        }

        object load(environment& a_environment) override {
            table* tbl = new table;

            rebar::virtual_table& string_builder_virtual_table = a_environment.register_native_class("REBAR::STD::STRING_UTILITY::STRING_BUILDER");
            load_string_builder(a_environment, string_builder_virtual_table);
            native_object n_obj = a_environment.create_native_object<std::vector<std::string>>(string_builder_virtual_table);
            (*tbl)[a_environment.str("StringBuilder")] = n_obj;

            return tbl;
        }
    };

    REBAR_DEFINE_LIBRARY("StringUtility", string_utility);
}

#endif //REBAR_STRING_UTILITY_HPP
