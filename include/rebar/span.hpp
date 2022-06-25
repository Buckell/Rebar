//
// Created by maxng on 6/21/2022.
//

#ifndef REBAR_SPAN_HPP
#define REBAR_SPAN_HPP

namespace rebar {
    template <typename t_type>
    struct span_convertible {
        static constexpr bool value = has_std_size_function<t_type>::value && has_std_data_function<t_type>::value;
    };

    template <typename t_type>
    struct span {
        struct iterator {
            friend struct span<t_type>;

            using iterator_category = std::random_access_iterator_tag;
            using difference_type = std::ptrdiff_t;
            using value_type = t_type;
            using pointer = const t_type*;
            using reference = const t_type&;

        private:
            pointer m_ptr;

        public:
            constexpr iterator(pointer ptr) noexcept : m_ptr(ptr) {}

            constexpr iterator& operator++() noexcept {
                ++m_ptr;
                return *this;
            }

            constexpr iterator operator++(int) & noexcept {
                return iterator(m_ptr++);
            }

            constexpr iterator& operator--() noexcept {
                --m_ptr;
                return *this;
            }

            constexpr iterator operator--(int) & noexcept {
                return iterator(m_ptr--);
            }

            [[nodiscard]] constexpr reference operator*() noexcept {
                return *m_ptr;
            }

            constexpr pointer operator->() noexcept {
                return m_ptr;
            }

            constexpr pointer operator->() const noexcept {
                return m_ptr;
            }

            constexpr iterator& operator+=(difference_type rhs) noexcept {
                m_ptr += rhs;
                return *this;
            }

            constexpr iterator& operator-=(difference_type rhs) noexcept {
                m_ptr -= rhs;
                return *this;
            }

            [[nodiscard]] constexpr reference operator[](difference_type rhs) noexcept {
                return *(m_ptr + rhs);
            }

            [[nodiscard]] constexpr bool operator<(const iterator rhs) const noexcept {
                return m_ptr < rhs.m_ptr;
            }

            [[nodiscard]] constexpr bool operator>(const iterator rhs) const noexcept {
                return m_ptr > rhs.m_ptr;
            }

            [[nodiscard]] constexpr bool operator>=(const iterator rhs) const noexcept {
                return m_ptr >= rhs.m_ptr;
            }

            [[nodiscard]] constexpr bool operator<=(const iterator rhs) const noexcept {
                return m_ptr <= rhs.m_ptr;
            }

            [[nodiscard]] constexpr bool operator==(const iterator rhs) const noexcept {
                return m_ptr == rhs.m_ptr;
            }

            [[nodiscard]] constexpr bool operator!=(const iterator rhs) const noexcept {
                return m_ptr != rhs.m_ptr;
            }

            [[nodiscard]] friend constexpr iterator operator+(iterator lhs, iterator::difference_type rhs) noexcept {
                return iterator(lhs.m_ptr + rhs);
            }

            [[nodiscard]] friend constexpr iterator operator+(iterator::difference_type rhs, iterator lhs) noexcept {
                return iterator(lhs.m_ptr + rhs);
            }

            [[nodiscard]] friend constexpr iterator operator-(iterator lhs, iterator::difference_type rhs) noexcept {
                return iterator(lhs.m_ptr - rhs);
            }

            [[nodiscard]] friend constexpr iterator::difference_type operator-(iterator lhs, iterator rhs) {
                return rhs.m_ptr - lhs.m_ptr;
            }
        };

        using value_type = t_type;
        using const_iterator = const iterator;

    private:
        const t_type* m_data;
        const size_t m_size;

    public:
        constexpr span() noexcept : m_data(nullptr), m_size(0) {}

        constexpr span(const t_type* a_data, const size_t a_size) noexcept : m_data(a_data), m_size(a_size) {}

        template <size_t c_size>
        constexpr span(const t_type (&a_array)[c_size]) noexcept : m_data(a_array), m_size(c_size) {}

        template <size_t c_size>
        constexpr span(const std::array<t_type, c_size> a_array) noexcept : m_data(a_array.data()), m_size(c_size) {}

        template <typename t_container, typename = std::enable_if<span_convertible<t_container>::value>>
        constexpr span(const t_container& a_container) noexcept(noexcept(a_container.size() && a_container.data())) : m_data(a_container.data()), m_size(a_container.size()) {}

        constexpr span(const iterator a_begin, const iterator a_end) : m_data(a_begin.m_ptr), m_size(std::distance(a_end, a_begin)) {}

        constexpr span(const span& a_span) noexcept = default;
        constexpr span(span&& a_span) noexcept = default;

        [[nodiscard]] constexpr const t_type* data() const noexcept {
            return m_data;
        }

        [[nodiscard]] constexpr size_t size() const noexcept {
            return m_size;
        }

        [[nodiscard]] constexpr bool empty() const noexcept {
            return m_size == 0;
        }

        [[nodiscard]] constexpr span<t_type> subspan(const size_t index, const size_t size = std::numeric_limits<size_t>::max()) const noexcept {
            return { m_data + index, std::min(size, m_size - index) };
        }

        [[nodiscard]] constexpr iterator cbegin() const noexcept {
            return iterator(m_data);
        }

        [[nodiscard]] constexpr iterator begin() const noexcept {
            return iterator(m_data);
        }

        [[nodiscard]] constexpr iterator cend() const noexcept {
            return iterator(m_data + m_size);
        }

        [[nodiscard]] constexpr iterator end() const noexcept {
            return iterator(m_data + m_size);
        }

        [[nodiscard]] constexpr iterator find(const t_type& a_object) const noexcept {
            return std::find(cbegin(), cend(), a_object);
        }

        [[nodiscard]] constexpr bool contains(const t_type& a_object) const noexcept {
            return find(a_object) != cend();
        }

        [[nodiscard]] const t_type& operator[](const size_t a_index) const noexcept {
            return m_data[a_index];
        }
    };
}

#endif //REBAR_SPAN_HPP
