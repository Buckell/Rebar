//
// Created by maxng on 10/11/2022.
//

#include "table_interface.hpp"
#include "table.hpp"

#ifndef REBAR_TABLE_INTERFACE_IMPL_HPP
#define REBAR_TABLE_INTERFACE_IMPL_HPP

namespace rebar {
    table::table(environment& a_environment) : m_environment(a_environment), m_table(new rtable) {
        ++m_table->m_reference_count;
    }

    table::table(const table& a_table) : m_environment(a_table.m_environment), m_table(a_table.m_table) {
        ++m_table->m_reference_count;
    }

    table::~table() {
        if (m_table && --m_table->m_reference_count == 0) {
            delete m_table;
        }
    }

    table& table::operator = (const table& a_table) noexcept {
        if (this == &a_table) {
            return *this;
        }

        if (--m_table->m_reference_count == 0) {
            delete m_table;
        }

        m_table = a_table.m_table;
        ++m_table->m_reference_count;

        return *this;
    }

    table& table::operator = (table&& a_table) noexcept {
        if (this == &a_table) {
            return *this;
        }

        if (--m_table->m_reference_count == 0) {
            delete m_table;
        }

        m_table = a_table.m_table;
        a_table.m_table = nullptr;

        return *this;
    }

    object& table::operator [] (const object& a_key) {
        return (*m_table)[a_key];
    }

    object& table::operator [] (const std::string_view a_key) {
        return (*m_table)[m_environment.str(a_key)];
    }

    object& table::at(const object& a_key) {
        return m_table->at(a_key);
    }

    object& table::at(const std::string_view a_key) {
        return m_table->at(m_environment.str(a_key));
    }

    template <typename... t_args>
    decltype(auto) table::emplace(t_args&&... a_args) {
        return m_table->emplace(std::forward<t_args>(a_args)...);
    }
}

#endif //REBAR_TABLE_INTERFACE_IMPL_HPP
