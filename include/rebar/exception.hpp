//
// Created by maxng on 6/24/2022.
//

#ifndef REBAR_EXCEPTION_HPP
#define REBAR_EXCEPTION_HPP

#include <exception>
#include <string>
#include <optional>

#include "utility.hpp"

namespace rebar {
    class exception : public std::exception {
    protected:
        std::string m_output;
        std::string_view m_type = "GENERAL";
        std::string m_message;

    public:
        exception(std::string_view a_type, std::string a_message) : m_type(a_type), m_message(std::move(a_message)) { get_message(); }
        exception(std::string_view a_type, std::string a_message, int) : m_type(a_type), m_message(std::move(a_message)) { }

        std::string_view get_message() noexcept {
            if (is_message_cached()) {
                return m_output;
            } else {
                return m_output = generate_message();
            }
        }

        [[nodiscard]] const char* what() const noexcept override {
            return m_output.c_str();
        }

    protected:
        virtual std::string generate_message() noexcept {
            using namespace std::string_literals;

            std::string header{ "[EXCEPTION - " };
            header += m_type;
            header += "] ";
            return header + m_message;
        }

        void cache_output(std::string a_message) {
            m_output = std::move(a_message);
        }

        [[nodiscard]] bool is_message_cached() noexcept {
            return !m_output.empty();
        }
    };

    /* SYNTAX EXCEPTION:
     * [EXCEPTION - SYNTAX] Expected an operator following identifier/numeral/etc.
     * [ORIGIN - FILE;C:\file\dev\main.rbr] [1020] [LINE 34 : 20]
     * local some_variable = 10000
     *                            ^
     */

    struct syntax_exception : public exception {
        std::string m_origin;
        size_t m_index;
        file_position m_pos;
        std::string_view m_line;

        syntax_exception(std::string a_message, std::string a_origin, const size_t a_index, const file_position a_pos, const std::string_view a_line)
            : exception("SYNTAX", a_message, 0), m_origin(a_origin), m_index(a_index), m_pos(a_pos), m_line(a_line) { get_message(); }

        std::string generate_message() noexcept override {
            using namespace std::string_literals;

            return "[EXCEPTION - SYNTAX] "s + m_message + '\n' +
                   "[LINE " + std::to_string(m_pos.row) + " : " + std::to_string(m_pos.column) + "] [ORIGIN - " + m_origin + "] \n" +
                   std::string(m_line) + '\n' +
                   std::string(m_pos.column, ' ') + "^\n";
        }
    };

    /* RUNTIME EXCEPTION:
     * [EXCEPTION - RUNTIME] Invalid operands for addition operation (table + string).
     * 1. C:\prog\main.rbr - [ENTRY] [LINE 0]
     *   2. C:\prog\ext\sub.rbr - SomeFunction [LINE 12]
     *     3. C:\prog\ext\sub.rbr - AnotherFunction [LINE 30]
     *
     * local some_variable = some_table + "Hello!";
     *                                  ^
     */

    struct stack_trace_entry {
        std::string m_file;
        std::string m_function_name;
        file_position m_pos;
    };

    struct runtime_exception : public exception {
        environment& m_environment;
        //size_t m_index;
        //file_position m_pos;
        //std::string_view m_line;
        //std::vector<stack_trace_entry> m_stack_trace;

        runtime_exception(environment& a_environment);

        std::string generate_message() noexcept override;

        //std::string generate_message() noexcept override {
        //    using namespace std::string_literals;
        //
        //    std::string message = "[EXCEPTION - RUNTIME] "s + m_message + '\n';
        //
        //    for (size_t i = 0; i < m_stack_trace.size(); ++i) {
        //        stack_trace_entry& entry = m_stack_trace[i];
        //        message += std::string(i * 2, ' ') + std::to_string(i + 1) + ". " + entry.m_file + " [ LINE " + std::to_string(entry.m_pos.row + 1) + " : " + std::to_string(entry.m_pos.column) + "]\n";
        //    }
        //
        //    message += '\n';
        //    message += m_line;
        //    message += std::string(m_pos.column, ' ') + "^\n";
        //
        //    return message;
        //}
    };
}

#endif //REBAR_EXCEPTION_HPP
