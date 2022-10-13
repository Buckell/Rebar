//
// Created by maxng on 10/2/2022.
//

#ifndef REBAR_NATIVE_EXCEPTION_HPP
#define REBAR_NATIVE_EXCEPTION_HPP

#include "../native_template.hpp"
#include "../environment.hpp"
#include "load.hpp"

namespace rebar::native {
    struct exception : public native_template<> {
        REBAR_NATIVE_CLASS(exception, "EXCEPTION");

        REBAR_NATIVE_FUNCTION(GetObject) {
            REBAR_RETURN(env->get_exception_object());
        }

        REBAR_NATIVE_FUNCTION(GetType) {
            REBAR_RETURN(env->get_exception_type());
        }

        REBAR_NATIVE_FUNCTION(GetStackTrace) {
            REBAR_RETURN(env->str(env->generate_exception_message()));
        }

        REBAR_NATIVE_FUNCTION(PrintStackTrace) {
            env->cerr() << env->generate_exception_message() << std::endl;
            REBAR_RETURN(null);
        }

        REBAR_NATIVE_FUNCTION(GetTraceCount) {
            REBAR_RETURN(static_cast<integer>(env->get_stack_trace().entries().size()));
        }

        REBAR_NATIVE_FUNCTION(GetTraceRow) {
            table row_table(*env);

            size_t index = static_cast<size_t>(env->arg(1).get_integer());

            auto& entry = env->get_stack_trace().entries().at(index);

            function func = entry.get_function();
            function_info& func_info = env->get_function_info(func);

            row_table["function_name"] = env->str(func_info.m_name);

            auto& orig = func_info.get_origin();

            auto file_find_it = orig.info.find("FILE");
            if (file_find_it != orig.info.cend()) {
                row_table["file"] = env->str(file_find_it->second);
            }

            auto pos = entry.get_position();

            row_table["source_index"] = static_cast<integer>(pos.first);
            row_table["source_row"] = static_cast<integer>(pos.second.row);
            row_table["source_column"] = static_cast<integer>(pos.second.column);

            REBAR_RETURN(row_table);
        }
    };

    object load_exception(environment& a_env) {
        exception e(a_env);
        return e.build_object();
    }
}

#endif //REBAR_NATIVE_EXCEPTION_HPP
