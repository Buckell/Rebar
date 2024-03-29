//
// Created by maxng on 6/21/2022.
//

#ifndef REBAR_ENVIRONMENT_HPP
#define REBAR_ENVIRONMENT_HPP

#include <array>

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

    class environment {
        friend class function;

        size_t m_argument_count;
        std::array<object, 16> m_arguments;

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

        table m_string_virtual_table;

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
        std::vector<function> m_functions;
        table m_global_table;
        std::unique_ptr<provider> m_provider;

    public:
        environment() noexcept : m_provider(std::make_unique<default_provider>(*this)) {}

        template <typename t_provider>
        explicit environment(const use_provider_t<t_provider>) noexcept : m_provider(std::make_unique<t_provider>(*this)) {};

        environment(const environment&) = delete;
        environment(environment&&) = delete;

        [[nodiscard]] string str(const std::string_view a_string) {
            auto found = m_string_table.find(a_string);

            if (found == m_string_table.cend()) {
                string created_string(a_string);

                m_string_table.emplace(created_string.to_string_view(), std::move(created_string));

                return created_string;
            }

            return found->second;
        }

        [[nodiscard]] table& get_string_virtual_table() noexcept {
            return m_string_virtual_table;
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

        [[nodiscard]] function compile_string(std::string a_string) {
            return m_provider->compile(parse(m_lexer, std::move(a_string)));
        }

        [[nodiscard]] object bind(callable a_function) {
            return m_provider->bind(a_function);
        }

        // FUNCTION PARAMETERS

        [[nodiscard]] size_t arg_count() const noexcept {
            return m_argument_count;
        }

        [[nodiscard]] object arg(const size_t a_index) const noexcept {
            return m_arguments[a_index];
        }

        void push_arg(const object a_object) {
            m_arguments[m_argument_count] = a_object;
            ++m_argument_count;
        }

        void set_args(const span<object> a_objects) {
            std::copy_n(a_objects.begin(), a_objects.size(), m_arguments.begin());
            m_argument_count = a_objects.size();
        }

        [[nodiscard]] auto& get_args() const noexcept {
            return m_arguments;
        }

        void clear_args() noexcept {
            m_argument_count = 0;
        }

        // - FUNCTION PARAMETERS

        /*
        [[nodiscard]] table& get_metadata(function a_function) noexcept {

        }

        [[nodiscard]] source_position plaintext_source(function a_function) const noexcept {

        }

        [[nodiscard]] parse_unit& function_source_unit(function a_function) noexcept {

        }
        */

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
