//
// Created by maxng on 6/25/2022.
//

#ifndef REBAR_META_HPP
#define REBAR_META_HPP

#include "../rebar.hpp"

namespace rebar::library::standard {
    struct meta : public library {
        meta() : library(usage::explicit_include) {}

        static REBAR_FUNCTION(GetFunctionInfo) {
            auto func_object = env->arg(0);

            if (func_object.is_function()) {
                function func = func_object.get_function(*env);

                function_info& info = env->get_function_info(func);

                table info_table(*env);

                info_table["Name"] = env->str(info.get_name());
                info_table["ID"] = static_cast<integer>(info.get_id());

                *ret = info_table;
                return;
            }

            *ret = null;
        }

        object load(environment& a_env) override {
            table meta_table(a_env);

            std::map<std::string, std::string> omap{ { "CLASS", "REBAR::STD::META" } };

            meta_table["GetFunctionInfo"] = a_env.bind(GetFunctionInfo, "GetFunctionInfo", omap);

            return meta_table;
        }
    };

    REBAR_DEFINE_LIBRARY("Meta", meta);
}

#endif //REBAR_META_HPP
