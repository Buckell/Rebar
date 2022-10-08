//
// Created by maxng on 10/4/2022.
//

#include "stack_trace.hpp"
#include "environment.hpp"

#ifndef REBAR_STACK_TRACE_IMPL_HPP
#define REBAR_STACK_TRACE_IMPL_HPP

namespace rebar {
    void stack_trace::entry::generate_line() {
        using namespace std::string_literals;

        function_info& info = m_environment.get_function_info(m_function);

        m_line = "";

        if (m_parse_unit) {
            auto pos = calculate_file_position(m_parse_unit->m_plaintext, m_expression->get_operand(0).m_origin_source_positions[0].get_index());

            m_line += '(';
            m_line += std::to_string(pos.row);
            m_line += ':';
            m_line += std::to_string(pos.column);
            m_line += ") ";
        }

        m_line += info.get_name();
        m_line += " at ";
        m_line += info.m_origin;
    }
}

#endif //REBAR_STACK_TRACE_IMPL_HPP
