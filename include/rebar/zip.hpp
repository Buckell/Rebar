//
// Created by maxng on 6/22/2022.
//

#ifndef REBAR_ZIP_HPP
#define REBAR_ZIP_HPP

#include <iterator>
#include <utility>

namespace rebar {
    template <typename... t_containers>
    struct zip {
        static constexpr size_t container_count = sizeof...(t_containers);

        using packed_element = std::tuple<typename t_containers::value_type&...>;

        struct iterator {
            friend struct zip<t_containers...>;

            using iterator_category = std::random_access_iterator_tag;
            using difference_type = std::ptrdiff_t;
            using value_type = packed_element;
            using pointer = const value_type*;
            using reference = const value_type&;

        private:
            zip& m_zip;
            size_t m_index;

            constexpr iterator(zip& a_zip, const size_t a_index = 0) noexcept : m_zip(a_zip), m_index(a_index) {}

        public:
            constexpr iterator& operator++() noexcept {
                ++m_index;
                return *this;
            }

            constexpr iterator operator++(int) & noexcept {
                return iterator(m_zip, m_index++);
            }

            constexpr iterator& operator--() noexcept {
                --m_index;
                return *this;
            }

            constexpr iterator operator--(int) & noexcept {
                return iterator(m_zip, m_index--);
            }

            constexpr value_type operator*() noexcept {
                return m_zip.get(m_index);
            }

            constexpr value_type operator->() noexcept {
                return (**this);
            }

            constexpr value_type operator->() const noexcept {
                return (**this);
            }

            constexpr iterator& operator+=(difference_type rhs) noexcept {
                m_index += rhs;
                return *this;
            }

            constexpr iterator& operator-=(difference_type rhs) noexcept {
                m_index -= rhs;
                return *this;
            }

            constexpr value_type operator[](difference_type rhs) noexcept {
                return *((*this) + rhs);
            }

            constexpr bool operator<(const iterator rhs) const noexcept {
                return m_index < rhs.m_index;
            }

            constexpr bool operator>(const iterator rhs) const noexcept {
                return m_index > rhs.m_index;
            }

            constexpr bool operator>=(const iterator rhs) const noexcept {
                return m_index >= rhs.m_index;
            }

            constexpr bool operator<=(const iterator rhs) const noexcept {
                return m_index <= rhs.m_index;
            }

            constexpr bool operator==(const iterator rhs) const noexcept {
                return m_index == rhs.m_index;
            }

            constexpr bool operator!=(const iterator rhs) const noexcept {
                return m_index != rhs.m_index;
            }

            friend constexpr iterator operator+(iterator lhs, iterator::difference_type rhs) noexcept {
                return iterator(lhs.m_index + rhs);
            }

            friend constexpr iterator operator+(iterator::difference_type lhs, iterator rhs) noexcept {
                return iterator(rhs.m_index + lhs);
            }

            friend constexpr iterator operator-(iterator lhs, iterator::difference_type rhs) noexcept {
                return iterator(lhs.m_index - rhs);
            }

            friend constexpr iterator::difference_type operator-(iterator lhs, iterator rhs) {
                return rhs.m_index - lhs.m_index;
            }
        };

    private:
        std::tuple<t_containers&...> m_containers;
        size_t m_size;

    public:
        explicit zip(t_containers&... a_containers) : m_containers(a_containers...) {
            m_size = std::max((a_containers.size(), ...));
        }

        inline packed_element get(size_t a_index) noexcept {
            if constexpr (container_count == 1) {
                return packed_element(
                    std::get<0>(m_containers)[a_index]
                );
            } else if constexpr (container_count == 2) {
                return packed_element(
                    std::get<0>(m_containers)[a_index],
                    std::get<1>(m_containers)[a_index]
                );
            } else if constexpr (container_count == 3) {
                return packed_element(
                    std::get<0>(m_containers)[a_index],
                    std::get<1>(m_containers)[a_index],
                    std::get<2>(m_containers)[a_index]
                );
            } else if constexpr (container_count == 4) {
                return packed_element(
                    std::get<0>(m_containers)[a_index],
                    std::get<1>(m_containers)[a_index],
                    std::get<2>(m_containers)[a_index],
                    std::get<3>(m_containers)[a_index]
                );
            } else if constexpr (container_count == 5) {
                return packed_element(
                    std::get<0>(m_containers)[a_index],
                    std::get<1>(m_containers)[a_index],
                    std::get<2>(m_containers)[a_index],
                    std::get<3>(m_containers)[a_index],
                    std::get<4>(m_containers)[a_index]
                );
            }
        }

        inline size_t size() const noexcept {
            return m_size;
        }

        constexpr iterator cbegin() const noexcept {
            return iterator(*this);
        }

        constexpr iterator begin() noexcept {
            return iterator(*this);
        }

        constexpr iterator cend() const noexcept {
            return iterator(*this, size());
        }

        constexpr iterator end() noexcept {
            return iterator(*this, size());
        }

    private:
        void calculate_size() noexcept {
            if constexpr (container_count == 1) {
                m_size = std::get<0>(m_containers).size();
            } else if constexpr (container_count == 2) {
                m_size = std::min(
                        std::get<0>(m_containers).size(),
                        std::get<1>(m_containers).size()
                );
            } else if constexpr (container_count == 3) {
                m_size = std::min(
                        std::get<0>(m_containers).size(),
                        std::get<1>(m_containers).size()
                );
            } else if constexpr (container_count == 4) {
                m_size = std::min(
                        std::get<0>(m_containers).size(),
                        std::get<1>(m_containers).size()
                );
            } else if constexpr (container_count == 5) {
                m_size = std::min(
                        std::get<0>(m_containers).size(),
                        std::get<1>(m_containers).size()
                );
            }
        }
    };
}

#endif //REBAR_ZIP_HPP
