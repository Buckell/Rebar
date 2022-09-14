//
// Created by maxng on 6/21/2022.
//

#ifndef REBAR_ENVIRONMENT_HPP
#define REBAR_ENVIRONMENT_HPP

#include <array>
#include <optional>

#include "object.hpp"
#include "preprocess.hpp"
#include "interpreter.hpp"
#include "table.hpp"
#include "native_object_impl.hpp"

#include <xxhash.hpp>
#include <skarupke_map.hpp>

namespace rebar {
    template <typename t_provider>
    struct use_provider_t {};

    template <typename t_provider>
    inline constexpr use_provider_t<t_provider> use_provider{};

    using default_provider = interpreter;

    using namespace std::literals::string_literals;

    class environment {
        friend class function;
        friend class string;

        size_t m_argument_count;
        const object* m_arguments_pointer;

        ska::detailv3::sherwood_v3_table<
            std::pair<std::string_view, string>,
            std::string_view,
            xxh_string_view_hash,
            ska::detailv3::KeyOrValueHasher<std::string_view, std::pair<std::string_view, string>, xxh_string_view_hash>,
            std::equal_to<std::string_view>,
            ska::detailv3::KeyOrValueEquality<std::string_view, std::pair<std::string_view, string>, std::equal_to<std::string_view>>,
            std::allocator<std::pair<std::string_view, string>>,
            typename std::allocator_traits<std::allocator<std::pair<std::string_view, string>>>::template rebind_alloc<ska::detailv3::sherwood_v3_entry<std::pair<std::string_view, string>>>
        > m_string_table; // I don't like it any more than you do.

        ska::detailv3::sherwood_v3_table<
            std::pair<object, virtual_table>,
            object,
            std::hash<object>,
            ska::detailv3::KeyOrValueHasher<object, std::pair<object, virtual_table>, std::hash<object>>,
            std::equal_to<object>,
            ska::detailv3::KeyOrValueEquality<object, std::pair<object, virtual_table>, std::equal_to<object>>,
            std::allocator<std::pair<object, virtual_table>>,
            typename std::allocator_traits<std::allocator<std::pair<object, virtual_table>>>::template rebind_alloc<ska::detailv3::sherwood_v3_entry<std::pair<object, virtual_table>>>
        > m_native_class_table; // Ditto.

        lexer m_lexer;
        std::unique_ptr<provider> m_provider;

        std::vector<std::unique_ptr<parse_unit>> m_parse_units;

        std::vector<function> m_functions;
        std::map<size_t, std::unique_ptr<function_info>> m_function_infos;
        size_t m_function_id_count;

        table m_global_table;
        table m_string_virtual_table;
        table m_array_virtual_table;

        std::istream* m_stream_in = &std::cin;
        std::ostream* m_stream_out = &std::cout;
        std::ostream* m_stream_log = &std::clog;
        std::ostream* m_stream_error = &std::cerr;

    public:
        environment() noexcept : m_provider(std::make_unique<default_provider>(*this)) {}

        template <typename t_provider>
        explicit environment(const use_provider_t<t_provider>) noexcept : m_provider(std::make_unique<t_provider>(*this)) {};

        environment(const environment&) = delete;
        environment(environment&&) = delete;

        [[nodiscard]] string str(const std::string_view a_string) {
            auto found = m_string_table.find(a_string);

            if (found == m_string_table.cend()) {
                string created_string(this, a_string);

                m_string_table.emplace(created_string.to_string_view(), std::move(created_string));

                return created_string;
            }

            return found->second;
        }

        [[nodiscard]] inline table& get_string_virtual_table() noexcept {
            return m_string_virtual_table;
        }

        [[nodiscard]] inline table& get_array_virtual_table() noexcept {
            return m_array_virtual_table;
        }

        virtual_table& register_native_class(const object a_identifier, virtual_table a_table = {}) {
            auto iterator_pair = m_native_class_table.emplace(a_identifier, std::move(a_table));
            return iterator_pair.first->second;
        }

        virtual_table& register_native_class(const std::string_view a_identifier, virtual_table a_table = {}) {
            auto iterator_pair = m_native_class_table.emplace(str(a_identifier), std::move(a_table));
            return iterator_pair.first->second;
        }

        template <class t_class>
        virtual_table& register_native_class(const object a_identifier, t_class& a_class) {
            // TODO: Implement.
        }

        template <class t_class>
        virtual_table& register_native_class(const std::string_view a_identifier, t_class& a_class) {
            // TODO: Implement.
        }

        [[nodiscard]] virtual_table& get_native_class(const object a_identifier) {
            auto found = m_native_class_table.find(a_identifier);

            if (found == m_native_class_table.cend()) {
                throw std::out_of_range("Identifier passed to get_native_class() was not registered.");
            }

            return found->second;
        }

        [[nodiscard]] virtual_table& get_native_class(const std::string_view a_identifier) {
            auto found = m_native_class_table.find(str(a_identifier));

            if (found == m_native_class_table.cend()) {
                throw std::out_of_range("Identifier passed to get_native_class() was not registered.");
            }

            return found->second;
        }

        template <typename t_object>
        [[maybe_unused]] [[nodiscard]] native_object create_native_object(virtual_table& a_virtual_table) {
            return native_object::create<t_object>(a_virtual_table, std::in_place_type<t_object>);
        }

        template <typename t_object>
        [[maybe_unused]] [[nodiscard]] native_object create_native_object(const object a_identifier) {
            return native_object::create<t_object>(get_native_class(a_identifier), std::in_place_type<t_object>);
        }

        template <typename t_object>
        [[maybe_unused]] [[nodiscard]] native_object create_native_object(const std::string_view a_identifier) {
            return native_object::create<t_object>(get_native_class(a_identifier), std::in_place_type<t_object>);
        }

        template <typename t_object>
        [[maybe_unused]] [[nodiscard]] native_object create_native_object(virtual_table& a_virtual_table, t_object a_object) {
            return native_object::create<t_object>(a_virtual_table, std::move(a_object));
        }

        template <typename t_object>
        [[maybe_unused]] [[nodiscard]] native_object create_native_object(const object a_identifier, t_object a_object) {
            return native_object::create<t_object>(get_native_class(a_identifier), std::move(a_object));
        }

        template <typename t_object>
        [[maybe_unused]] [[nodiscard]] native_object create_native_object(const std::string_view a_identifier, t_object a_object) {
            return native_object::create<t_object>(get_native_class(a_identifier), std::move(a_object));
        }

        template <typename t_object, typename... t_args>
        [[maybe_unused]] [[nodiscard]] native_object create_native_object(virtual_table& a_virtual_table, std::in_place_type_t<t_object> a_in_place, t_args... a_args) {
            return native_object::create<t_object>(a_virtual_table, a_in_place, std::forward<t_args>(a_args)...);
        }

        template <typename t_object, typename... t_args>
        [[maybe_unused]] [[nodiscard]] native_object create_native_object(const object a_identifier, std::in_place_type_t<t_object> a_in_place, t_args... a_args) {
            return native_object::create<t_object>(get_native_class(a_identifier), a_in_place, std::forward<t_args>(a_args)...);
        }

        template <typename t_object, typename... t_args>
        [[maybe_unused]] [[nodiscard]] native_object create_native_object(const std::string_view a_identifier, std::in_place_type_t<t_object> a_in_place, t_args... a_args) {
            return native_object::create<t_object>(get_native_class(a_identifier), a_in_place, std::forward<t_args>(a_args)...);
        }

        [[nodiscard]] function compile_string(std::string a_string, std::optional<std::string> a_name = std::nullopt, std::optional<std::string> a_origin = std::nullopt) {
            m_parse_units.push_back(std::make_unique<parse_unit>(parse(m_lexer, std::move(a_string))));

            parse_unit& punit = *m_parse_units.back();

            function func = m_provider->compile(punit);

            emplace_function_info(func, {
                a_name.has_value() ? std::move(a_name.value()) : "UNNAMED",
                a_origin.has_value() ? a_origin.value() : "IMMEDIATE;"s + std::to_string(m_function_id_count),
                0,
                std::make_unique<function_info_source::rebar>(punit.m_plaintext, node {
                    punit.m_lex_unit.tokens(),
                    punit.m_lex_unit.source_positions(),
                    node::type::block,
                    punit.m_block
                })
            });

            return func;
        }

        void emplace_function_info(const function a_function, function_info a_function_info) noexcept {
            a_function_info.m_id = m_function_id_count++;
            m_function_infos.emplace(bit_cast<size_t>(a_function.m_data), std::make_unique<function_info>(std::move(a_function_info)));
        }

        [[nodiscard]] function_info& get_function_info(const function a_function) noexcept {
            return *m_function_infos.at(bit_cast<size_t>(a_function.m_data));
        }

        [[nodiscard]] function_info& get_function_info(const function a_function) const noexcept {
            return *m_function_infos.at(bit_cast<size_t>(a_function.m_data));
        }

        [[nodiscard]] object bind(callable a_function, std::optional<std::string> a_name = std::nullopt, std::optional<std::string> a_origin = std::nullopt) {
            function func = m_provider->bind(a_function);

            emplace_function_info(func, {
                a_name.has_value() ? a_name.value() : "UNNAMED",
                a_origin.has_value() ? a_origin.value() : "NATIVE;"s + std::to_string(m_function_id_count),
                0,
                std::make_unique<function_info_source::native>(a_function)
            });

            return func;
        }

        [[nodiscard]] size_t get_current_function_id_stack() const noexcept {
            return m_function_id_count;
        }

        // FUNCTION PARAMETERS

        [[nodiscard]] size_t arg_count() const noexcept {
            return m_argument_count;
        }

        [[nodiscard]] const object& arg(const size_t a_index) const noexcept {
            return a_index >= m_argument_count ? null : m_arguments_pointer[a_index];
        }

        [[nodiscard]] const auto* get_args() const noexcept {
            return m_arguments_pointer;
        }

        [[nodiscard]] const object* get_arguments_pointer() noexcept {
            return m_arguments_pointer;
        }

        [[nodiscard]] const object** get_arguments_pointer_ref() noexcept {
            return &m_arguments_pointer;
        }

        [[nodiscard]] size_t* get_arguments_size_pointer() noexcept {
            return &m_argument_count;
        }

        void set_argument_count(size_t a_count) noexcept {
            m_argument_count = a_count;
        }

        void set_arguments_pointer(const object* a_pointer) noexcept {
            m_arguments_pointer = a_pointer;
        }

        void set_in_stream(std::istream& a_stream) noexcept {
            m_stream_in = &a_stream;
        }

        void set_out_stream(std::ostream& a_stream) noexcept {
            m_stream_out = &a_stream;
        }

        void set_log_stream(std::ostream& a_stream) noexcept {
            m_stream_log = &a_stream;
        }

        void set_error_stream(std::ostream& a_stream) noexcept {
            m_stream_error = &a_stream;
        }

        [[nodiscard]] std::istream& cin() noexcept {
            return *m_stream_in;
        }

        [[nodiscard]] std::ostream& cout() noexcept {
            return *m_stream_out;
        }

        [[nodiscard]] std::ostream& clog() noexcept {
            return *m_stream_log;
        }

        [[nodiscard]] std::ostream& cerr() noexcept {
            return *m_stream_error;
        }

        [[nodiscard]] table& global_table() noexcept {
            return m_global_table;
        }

        [[nodiscard]] lexer& code_lexer() noexcept {
            return m_lexer;
        }

        [[nodiscard]] provider& execution_provider() noexcept {
            return *m_provider;
        }
    };
}

#endif //REBAR_ENVIRONMENT_HPP
