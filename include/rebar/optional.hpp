//
// Created by maxng on 6/21/2022.
//

#ifndef REBAR_OPTIONAL_HPP
#define REBAR_OPTIONAL_HPP

#include <utility>

namespace rebar {
    template <typename t_type>
    class optional {
        t_type* m_object;

    public:
        constexpr optional() noexcept : m_object(nullptr) {}
        constexpr explicit optional(const t_type& a_object) noexcept(noexcept(t_type(a_object))) : m_object(new t_type(a_object)) {}
        constexpr explicit optional(t_type&& a_object) noexcept(noexcept(t_type(std::move(a_object)))) : m_object(new t_type(std::move(a_object))) {}
        constexpr explicit optional(t_type* a_object) noexcept : m_object(*a_object) {}

        template <typename... t_args>
        constexpr explicit optional(std::in_place_t, t_args&&... a_args) noexcept(noexcept(t_type(std::forward<t_args>(a_args)...))) : m_object(new t_type(std::forward<t_args>(a_args)...)) {}

        constexpr optional(const optional& a_optional) noexcept(noexcept(t_type(*a_optional.m_object))) : m_object(new t_type(*a_optional.m_object)) {}
        constexpr optional(optional&& a_optional) noexcept {
            delete m_object;

            m_object = a_optional.m_object;
            a_optional.m_object = nullptr;
        }

        ~optional() {
            delete m_object;
        }

        constexpr optional& operator=(const optional& rhs) noexcept(noexcept(t_type(&rhs.m_object))) {
            delete m_object;
            m_object = rhs.m_object == nullptr ? nullptr : new t_type(*rhs.m_object);
            return *this;
        }

        constexpr optional& operator=(optional&& rhs) noexcept(noexcept(t_type(std::move(rhs.m_object)))) {
            delete m_object;
            m_object = new t_type(std::move(*rhs.m_object));
            rhs.m_object = nullptr;
            return *this;
        }

        constexpr optional& operator=(const t_type& rhs) noexcept(noexcept(t_type(rhs))) {
            delete m_object;
            m_object = new t_type(rhs);
            return *this;
        }

        constexpr optional& operator=(t_type&& rhs) noexcept(noexcept(t_type(std::move(rhs)))) {
            delete m_object;
            m_object = new t_type(std::move(rhs));
            return *this;
        }

        constexpr optional& operator=(t_type* rhs) noexcept {
            delete m_object;
            m_object = rhs;
            return *this;
        }

        [[nodiscard]] constexpr t_type* operator->() noexcept {
            return m_object;
        }

        [[nodiscard]] constexpr const t_type& operator->() const noexcept {
            return *m_object;
        }

        [[nodiscard]] constexpr t_type& operator*() noexcept {
            return *m_object;
        }

        [[nodiscard]] constexpr const t_type& operator*() const noexcept {
            return *m_object;
        }

        [[nodiscard]] constexpr t_type* raw() noexcept {
            return m_object;
        }

        [[nodiscard]] constexpr const t_type* raw() const noexcept {
            return m_object;
        }

        [[nodiscard]] constexpr t_type& get() noexcept {
            return *m_object;
        }

        [[nodiscard]] constexpr const t_type& get() const noexcept {
            return *m_object;
        }

        [[nodiscard]] constexpr bool has_value() const noexcept {
            return m_object != nullptr;
        }
    };

    template <typename t_type>
    class optional_view {
        t_type* m_object;

    public:
        constexpr optional_view() noexcept : m_object(nullptr) {}
        constexpr explicit optional_view(t_type& a_object) noexcept : m_object(&a_object) {}
        constexpr explicit optional_view(t_type* a_object) noexcept : m_object(a_object) {}

        constexpr optional_view(const optional_view& a_optional_view) noexcept = default;
        constexpr optional_view(optional_view&& a_optional_view) noexcept = default;

        constexpr optional_view& operator=(const optional_view& rhs) noexcept = default;
        constexpr optional_view& operator=(optional_view&& rhs) noexcept = default;

        constexpr optional_view& operator=(t_type& rhs) noexcept {
            m_object = &rhs;
            return *this;
        }

        constexpr optional_view& operator=(t_type* rhs) noexcept {
            m_object = rhs;
            return *this;
        }

        [[nodiscard]] constexpr t_type& operator->() noexcept {
            return *m_object;
        }

        [[nodiscard]] constexpr const t_type& operator->() const noexcept {
            return *m_object;
        }

        [[nodiscard]] constexpr t_type& operator*() noexcept {
            return *m_object;
        }

        [[nodiscard]] constexpr const t_type& operator*() const noexcept {
            return *m_object;
        }

        [[nodiscard]] constexpr t_type* raw() noexcept {
            return m_object;
        }

        [[nodiscard]] constexpr const t_type* raw() const noexcept {
            return m_object;
        }

        [[nodiscard]] constexpr t_type& get() noexcept {
            return *m_object;
        }

        [[nodiscard]] constexpr const t_type& get() const noexcept {
            return *m_object;
        }

        [[nodiscard]] constexpr bool has_value() const noexcept {
            return m_object != nullptr;
        }
    };
}

#endif //REBAR_OPTIONAL_HPP
