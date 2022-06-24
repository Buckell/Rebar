//
// Created by maxng on 6/21/2022.
//

#ifndef REBAR_FUNCTION_HPP
#define REBAR_FUNCTION_HPP

#include <type_traits>
#include <string>
#include <string_view>

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
        object call(t_objects&&... a_objects);
        object call(const span<object> a_objects);

        template <typename... t_objects>
        auto operator () (t_objects&&... a_objects) {
            return call(std::forward<t_objects>(a_objects)...);
        }
    };

    struct function_info {
        std::string m_name;
        std::string m_origin; // "FILE;C:\dev\main\main.rbr"
        size_t m_id;

        std::string_view m_plaintext_source;
        span<token> m_token_source;
        span<source_position> m_token_source_positions;
        node::block m_parse_source;

        [[nodiscard]] std::string_view get_name() const noexcept {
            return m_name;
        }

        [[nodiscard]] std::string_view get_origin() const noexcept {
            return m_origin;
        }

        [[nodiscard]] size_t get_id() const noexcept {
            return m_id;
        }

        [[nodiscard]] std::string_view get_plaintext_source() const noexcept {
            return m_plaintext_source;
        }

        [[nodiscard]] span<token> get_token_source() const noexcept {
            return m_token_source;
        }

        [[nodiscard]] span<token> get_token_source_positions() const noexcept {
            return m_token_source;
        }

        [[nodiscard]] node::block get_parse_source() const noexcept {
            return m_parse_source;
        }
    };
}

#endif //REBAR_FUNCTION_HPP
