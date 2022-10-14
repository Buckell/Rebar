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
        friend struct object;
        friend class environment;

        environment& m_environment;
        const void* m_data;

    public:
        function(environment& a_environment, const void* a_data) noexcept : m_environment(a_environment), m_data(a_data) {}

        template <typename... t_objects>
        object call_v(t_objects&&... a_objects);

        object call(span<object> a_objects);

        template <typename... t_objects>
        std::enable_if_t<sizeof...(t_objects) == 0 || ((std::is_convertible_v<t_objects, object>) && ...), object> operator () (t_objects&&... a_objects) {
            return call_v(std::forward<t_objects>(a_objects)...);
        }

        [[nodiscard]] const void* data() const noexcept {
            return m_data;
        }
    };

    namespace function_info_source {
        struct source {
            virtual void src() const {};
        };
    }

    struct origin {
        flags type_flags;
        std::map<std::string, std::string> info;
        // FILE/UNIT:
        //   info["PATH"] - File path.
        // LIBRARY:
        //   info["LIB_NAME"] - Library name.
        //   info["LIB_ID"] - Library ID.
        //
        // info["CLASS_PATH"] - Optional class path.

        struct flag {
            REBAR_DEFINE_FLAG(none, 0);

            // Intrinsic Rebar functions. (E.g. for Exception class.)
            REBAR_DEFINE_FLAG(internal, 1);
            // Standard Rebar functions. Compiled function as a part of a larger
            // compilation unit. (E.g. function defined within a file.)
            REBAR_DEFINE_FLAG(standard, 2);
            // Functions that are compiled with code from an unknown source.
            // Compiled with functions such as compile_string.
            REBAR_DEFINE_FLAG(immediate, 3);
            // Functions compiled within a file.
            REBAR_DEFINE_FLAG(file, 4);
            // Entire file compiled to a function.
            REBAR_DEFINE_FLAG(unit, 5);
            // Function from a library.
            REBAR_DEFINE_FLAG(library, 6);
            // Function bound from a native function.
            REBAR_DEFINE_FLAG(bound, 7);
        };
    };

    struct function_info {
        std::string m_name;
        origin m_origin;
        size_t m_id;

        std::unique_ptr<function_info_source::source> m_source;

        [[nodiscard]] std::string_view get_name() const noexcept {
            return m_name;
        }

        [[nodiscard]] const origin& get_origin() const noexcept {
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

            explicit native(callable a_function) : m_function(a_function) {}
        };
    }
}

#endif //REBAR_FUNCTION_HPP
