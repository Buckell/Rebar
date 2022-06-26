//
// Created by maxng on 6/25/2022.
//

#ifndef REBAR_META_HPP
#define REBAR_META_HPP

#include "../rebar.hpp"

namespace rebar::library::standard {
    struct meta : public library {
        meta() : library(usage::explicit_include) {}

        static object GetFunctionInfo(environment* a_env) {
            auto func_object = a_env->arg(0);

            std::cout << "________________" << std::endl;

            if (func_object.is_function()) {
                function func = func_object.get_function(*a_env);

                function_info& info = a_env->get_function_info(func);

                table* info_table = new table;

                (*info_table)[a_env->str("Name")] = a_env->str(info.get_name());
                (*info_table)[a_env->str("Origin")] = a_env->str(info.get_origin());
                (*info_table)[a_env->str("ID")] = static_cast<integer>(info.get_id());

                return info_table;
            }

            return null;
        }

        object load(environment& a_environment) override {
            table* lib_table = new table;

            (*lib_table)[a_environment.str("GetFunctionInfo")] = a_environment.bind(GetFunctionInfo, "GetFunctionInfo", "REBAR::STD::META");

            return lib_table;
        }
    };

    const char library_meta[] = "Meta";
    define_library<library_meta, meta> d_meta;
}

#endif //REBAR_META_HPP
