//
// Created by maxng on 6/21/2022.
//

#ifndef REBAR_ARRAY_IMPL_HPP
#define REBAR_ARRAY_IMPL_HPP

#include "array.hpp"

#include "object.hpp"

namespace rebar {
    void array::initialize(type a_type, size_t a_capacity) noexcept {
        m_root_pointer = std::malloc(structure_size(a_type));
        std::memset(m_root_pointer, 0, structure_size(a_type));
        type_reference() = a_type;

        if (a_type == type::managed) {
            new (reinterpret_cast<std::vector<object>*>(reinterpret_cast<size_t*>(m_root_pointer) + 2)) std::vector<object>();
            vector_reference().reserve(a_capacity);
        }

        reference();
    }

    [[nodiscard]] inline size_t array::size() const noexcept {
        switch (get_type()) {
            case type::managed:
                return vector_reference().size();
            case type::view:
                return *(reinterpret_cast<size_t*>(m_root_pointer) + 3);
        }
    }

    [[nodiscard]] inline size_t array::capacity() const noexcept {
        return vector_reference().capacity();
    }

    [[nodiscard]] inline std::vector<object>::iterator array::begin() noexcept {
        return get_type() == type::view ? vector_reference().begin() + view_offset() : vector_reference().begin();
    }
    [[nodiscard]] inline std::vector<object>::const_iterator array::cbegin() const noexcept {
        return get_type() == type::view ? vector_reference().begin() + view_offset() : vector_reference().begin();
    }

    [[nodiscard]] inline std::vector<object>::iterator array::end() noexcept {
        return get_type() == type::view ? vector_reference().begin() + view_offset() + size() : vector_reference().end();
    }

    [[nodiscard]] inline std::vector<object>::const_iterator array::cend() const noexcept {
        return get_type() == type::view ? vector_reference().begin() + view_offset() + size() : vector_reference().end();
    }

    [[nodiscard]] object& array::operator [] (const size_t index) noexcept {
        // TODO: Bounds checking.

        switch (get_type()) {
            case type::managed:
                return vector_reference()[index];
            case type::view:
                return vector_reference()[view_offset() + index];
        }
    }

    void array::push_back(const object a_object) noexcept {
        // TODO: Block views.
        vector_reference().push_back(a_object);
    }

    std::string array::to_string() noexcept {
        size_t max_elements = std::min(size(), max_display_elements);

        if (max_elements == 0) {
            return "[]";
        }

        std::string representation = "[ ";

        for (size_t i = 0; i < max_elements; ++i) {
            representation += (*this)[i].to_string();
            representation += (i == max_elements - 1)
                              ? ((size() > max_display_elements) ? ", ... ]" : " ]")
                              : ", ";
        }

        return representation;
    }

    void array::dereference() {
        type arr_type = get_type();

        if (arr_type == type::view) {
            view_array().dereference();
        }

        if (--reference_count_reference() == 0) {
            if (arr_type == type::managed) {
                vector_reference().~vector();
            }

            std::free(m_root_pointer);
        }
    }
}

#endif //REBAR_ARRAY_IMPL_HPP
