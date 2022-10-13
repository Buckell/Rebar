//
// Created by maxng on 10/4/2022.
//

#ifndef REBAR_STACK_TRACE_HPP
#define REBAR_STACK_TRACE_HPP

#include "function.hpp"
#include <vector>

namespace rebar {
    struct stack_trace {
        class entry {
            environment& m_environment;
            function m_function;
            const parse_unit* m_parse_unit;
            const node::expression* m_expression;
            std::string m_line;
            bool m_cache_valid = false;

        public:
            entry(environment& a_environment, function a_function, const parse_unit* a_unit = nullptr, const node::expression* a_expression = nullptr) :
                m_environment(a_environment),
                m_function(a_function),
                m_parse_unit(a_unit),
                m_expression(a_expression) {}

            [[nodiscard]] function get_function() const noexcept {
                return m_function;
            }

            [[nodiscard]] std::pair<size_t, file_position> get_position() const noexcept {
                if (m_parse_unit && m_expression) {
                    auto index = m_expression->get_operand(0).m_origin_source_positions[0].get_index();
                    return { index, calculate_file_position(m_parse_unit->m_plaintext, index) };
                } else {
                    return { 0, { 0, 0 } };
                }
            }

            [[nodiscard]] std::string_view to_string() noexcept {
                if (!m_cache_valid) {
                    generate_line();
                }

                return m_line;
            }

        private:
            void generate_line();
        };

        explicit stack_trace(environment& a_environment) : m_environment(a_environment) {}

        void clear() noexcept {
            m_entries.clear();
            m_cache_valid = false;
        }

        void add(function a_function, const parse_unit* a_unit, const node::expression* a_expression) {
            m_entries.emplace_back(
                m_environment,
                a_function,
                a_unit,
                a_expression
            );

            m_cache_valid = false;
        }

        [[nodiscard]] std::string_view to_string() {
            if (!m_cache_valid) {
                generate_message();
            }

            return m_message;
        }

        [[nodiscard]] const std::vector<entry>& entries() const noexcept {
            return m_entries;
        }

    private:
        void generate_message() {
            m_message.clear();

            for (size_t i = 0; i < m_entries.size(); ++i) {
                m_message += std::string(i, ' ');
                m_message += std::to_string(i + 1) + ". ";
                m_message += m_entries[i].to_string();
                m_message += '\n';
            }
        }

        environment& m_environment;
        std::vector<entry> m_entries;
        std::string m_message;
        bool m_cache_valid = false;
    };
}

#endif //REBAR_STACK_TRACE_HPP
