//
// Created by maxng on 6/21/2022.
//

#ifndef REBAR_TABLE_HPP
#define REBAR_TABLE_HPP

#include "object.hpp"

#include <skarupke_map.hpp>

namespace rebar {
    struct table : public ska::detailv3::sherwood_v3_table<
            std::pair<object, object>,
            object,
            std::hash<object>,
            ska::detailv3::KeyOrValueHasher<object, std::pair<object, object>, std::hash<object>>,
            std::equal_to<object>,
            ska::detailv3::KeyOrValueEquality<object, std::pair<object, object>, std::equal_to<object>>,
            std::allocator<std::pair<object, object>>,
            typename std::allocator_traits<std::allocator<std::pair<object, object>>>::template rebind_alloc<ska::detailv3::sherwood_v3_entry<std::pair<object, object>>>
    > {
        size_t m_reference_count = 0;

        [[nodiscard]] inline object& operator[](const object a_key) {
            return emplace(a_key, object()).first->second;
        }

        [[nodiscard]] object& at(const object a_key) {
            auto found = this->find(a_key);

            if (found == this->cend()) {
                throw std::out_of_range("Argument passed to at() was not in the map.");
            }

            return found->second;
        }

        [[nodiscard]] const object& at(const object a_key) const {
            auto found = this->find(a_key);

            if (found == this->cend()) {
                throw std::out_of_range("Argument passed to at() was not in the map.");
            }

            return found->second;
        }

        [[nodiscard]] object index(const object a_key) const {
            auto found = this->find(a_key);

            if (found == this->cend()) {
                return {};
            }

            return found->second;
        }

        void add(const table& a_table) {
            for (const std::pair<object, object>& pair : a_table) {
                emplace(pair);
            }
        }

    public:
        [[nodiscard]] friend bool operator==(const table& lhs, const table& rhs) {
            if (lhs.size() != rhs.size()) {
                return false;
            }

            for (const std::pair<object, object>& value : lhs) {
                auto found = rhs.find(value.first);

                if (found == rhs.end() || value.second != found->second){
                    return false;
                }
            }

            return true;
        }

        [[nodiscard]] friend bool operator!=(const table& lhs, const table& rhs) {
            return !(lhs == rhs);
        }

        // TODO: Add "add" and "add assignment" operator.

        [[nodiscard]] friend object index(const table* a_table, const object& a_key) {
            auto found = a_table->find(a_key);

            if (found == a_table->end()) {
                return object();
            }

            return found->second;
        }

        friend void emplace(table* a_table, const object a_key, const object a_value) {
            a_table->emplace(a_key, a_value);
        }
    };
}

#endif //REBAR_TABLE_HPP
