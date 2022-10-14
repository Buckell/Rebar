//
// Created by maxng on 6/21/2022.
//

#ifndef REBAR_ARRAY_HPP
#define REBAR_ARRAY_HPP

#include <cstdlib>
#include <vector>

#include "definitions.hpp"

namespace rebar {
    struct array {
        enum class type : enum_base {
            managed,
            view
        };

    private:
        void* m_root_pointer = nullptr;

        // type = managed (array_type)
        // reference_count (size_t)
        // objects (vector<object>)
        //
        // type = view (array_type)
        // reference_count (size_t)
        // reference_managed_array (array)
        // size (size_t)
        // index (size_t)

        static constexpr size_t structure_size(type a_type) {
            switch (a_type) {
                case type::managed:
                    return (2 * sizeof(size_t)) + sizeof(std::vector<object>);
                case type::view:
                    return 5 * sizeof(size_t);
                default:
                    return 0;
            }
        }

        [[nodiscard]] inline type& type_reference() noexcept {
            return *reinterpret_cast<type*>(m_root_pointer);
        }

        [[nodiscard]] inline size_t& reference_count_reference() noexcept {
            return *(reinterpret_cast<size_t*>(m_root_pointer) + 1);
        }

        [[nodiscard]] inline array& view_array() noexcept {
            return *(reinterpret_cast<array*>(m_root_pointer) + 2);
        }

        [[nodiscard]] inline size_t& view_offset() noexcept {
            return *(reinterpret_cast<size_t*>(m_root_pointer) + 4);
        }

        [[nodiscard]] inline size_t view_offset() const noexcept {
            return *(reinterpret_cast<size_t*>(m_root_pointer) + 4);
        }

    public:
        static constexpr size_t max_display_elements = 100;

        array() noexcept : m_root_pointer(nullptr) {}

        explicit array(const size_t a_size) {
            initialize(type::managed, a_size);
        }

        array(array& a_reference, const size_t a_offset, const size_t a_length) noexcept {
            initialize(type::view);

            view_array() = a_reference;
            view_offset() = a_offset;
        }

        array(const array& a_array) noexcept : m_root_pointer(a_array.m_root_pointer) {
            reference();
        }

        array(array&& a_array) noexcept : m_root_pointer(a_array.m_root_pointer) {
            reference();
        }

        ~array() noexcept {
            if (m_root_pointer != nullptr) {
                dereference();
            }
        }

        array& operator = (const array& rhs) {
            if (&rhs == this) {
                return *this;
            }

            dereference();
            m_root_pointer = rhs.m_root_pointer;
            reference();

            return *this;
        }

        array& operator = (array&& rhs) noexcept {
            if (&rhs == this) {
                return *this;
            }

            dereference();

            m_root_pointer = rhs.m_root_pointer;
            rhs.m_root_pointer = nullptr;

            return *this;
        }

        void initialize(type a_type, size_t a_capacity = 4) noexcept;

        [[nodiscard]] inline std::vector<object>& vector_reference() noexcept {
            switch (get_type()) {
                case type::managed:
                    return *(reinterpret_cast<std::vector<object>*>(reinterpret_cast<size_t*>(m_root_pointer) + 2));
                case type::view:
                    return (reinterpret_cast<array*>(m_root_pointer) + 2)->vector_reference();
            }
        }

        [[nodiscard]] const std::vector<object>& vector_reference() const noexcept {
            switch (get_type()) {
                case type::managed:
                    return *(reinterpret_cast<std::vector<object>*>(reinterpret_cast<size_t*>(m_root_pointer) + 2));
                case type::view:
                    return (reinterpret_cast<array*>(m_root_pointer) + 2)->vector_reference();
            }
        }

        [[nodiscard]] inline type get_type() const noexcept {
            return *reinterpret_cast<type*>(m_root_pointer);
        }

        [[nodiscard]] inline size_t size() const noexcept;

        [[nodiscard]] inline size_t capacity() const noexcept;

        [[nodiscard]] inline object* data() noexcept {
            return vector_reference().data();
        }

        [[nodiscard]] inline const object* data() const noexcept {
            return vector_reference().data();
        }

        [[nodiscard]] inline std::vector<object>::iterator begin() noexcept;

        [[nodiscard]] inline std::vector<object>::const_iterator cbegin() const noexcept;

        [[nodiscard]] inline std::vector<object>::iterator end() noexcept;

        [[nodiscard]] inline std::vector<object>::const_iterator cend() const noexcept;

        [[nodiscard]] object& operator [] (size_t index) noexcept;

        void push_back(const rebar::object& a_object) noexcept;

        [[nodiscard]] array sub_array(const size_t a_offset, const size_t a_length) {
            // TODO: Bounds checking.

            switch (get_type()) {
                case type::managed:
                    return { *this, a_offset, a_length };
                case type::view:
                    return { view_array(), view_offset() + a_offset, a_length };
            }

            return {};
        }

        [[nodiscard]] std::string to_string() noexcept;

        void reference() {
            ++reference_count_reference();

            if (get_type() == type::view) {
                ++view_array().reference_count_reference();
            }
        }

        void dereference();

        [[nodiscard]] inline size_t get_reference_count() const noexcept {
            return *(reinterpret_cast<size_t*>(m_root_pointer) + 1);
        }
    };
}

#endif //REBAR_ARRAY_HPP
