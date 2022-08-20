//
// Created by maxng on 6/21/2022.
//

#ifndef REBAR_FUNCTION_HPP
#define REBAR_FUNCTION_HPP

#include <type_traits>
#include <string>
#include <string_view>
#include <memory>

#include "token.hpp"
#include "span.hpp"
#include "parser.hpp"

namespace rebar {
    class environment;

    class function {
        friend class object;
        friend class environment;

        environment& m_environment;
        const void* m_data;

    public:
        function(environment& a_environment, const void* a_data) noexcept : m_environment(a_environment), m_data(a_data) {}

        template <typename... t_objects>
        object call_v(t_objects&&... a_objects);

        object call(const span<object> a_objects);

        template <typename... t_objects>
        std::enable_if_t<sizeof...(t_objects) == 0 || ((std::is_convertible_v<t_objects, object>) && ...), object> operator () (t_objects&&... a_objects) {
            return call_v(std::forward<t_objects>(a_objects)...);
        }
    };

    namespace function_info_source {
        struct source {
            virtual void src() const {};
        };
    }

    struct function_info {
        std::string m_name;
        std::string m_origin; // "FILE;C:\dev\main\main.rbr"
        size_t m_id;

        std::unique_ptr<function_info_source::source> m_source;

        [[nodiscard]] std::string_view get_name() const noexcept {
            return m_name;
        }

        [[nodiscard]] std::string_view get_origin() const noexcept {
            return m_origin;
        }

        [[nodiscard]] size_t get_id() const noexcept {
            return m_id;
        }

        [[nodiscard]] function_info_source::source& get_source() const noexcept {
            return *m_source;
        }
    };

    namespace function_info_source {
        struct rebar : public source {
            std::string_view m_plaintext_source;
            node m_source_node;

            rebar(std::string_view a_plaintext, node a_source_node) : m_plaintext_source(a_plaintext), m_source_node(a_source_node) {}
        };

        struct native : public source {
            callable m_function;

            native(callable a_function) : m_function(a_function) {}
        };
    }
}

#endif //REBAR_FUNCTION_HPP
