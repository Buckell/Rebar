//
// Created by maxng on 10/3/2022.
//

#ifndef REBAR_NATIVE_TABLE_IMPL_HPP
#define REBAR_NATIVE_TABLE_IMPL_HPP

#include "native_template.hpp"
#include "environment.hpp"

namespace rebar {
    native_template_function_definition::native_template_function_definition(environment& a_env, std::string_view a_name, virtual_table& a_table, callable a_function) {
        a_table[a_env.str(a_name)] = a_env.bind(a_function);
    }

    template <typename t_internal_type>
    native_template<t_internal_type>::native_template(environment& a_environment, std::string_view a_identifier) :
            m_environment(a_environment) {

        m_table = &m_environment.register_native_class(a_identifier);
    }

    template <typename t_internal_type>
    object native_template<t_internal_type>::build_object() {
        return m_environment.create_native_object<t_internal_type>(*m_table);
    }
}

#endif //REBAR_NATIVE_TABLE_IMPL_HPP
