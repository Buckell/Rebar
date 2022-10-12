//
// Created by maxng on 10/11/2022.
//

#ifndef REBAR_TABLE_INTERFACE_HPP
#define REBAR_TABLE_INTERFACE_HPP

namespace rebar {
    struct rtable;

    class table {
        environment& m_environment;
        rtable* m_table;

    public:
        explicit table(environment& a_environment);
        table(environment& a_environment, rtable* a_table) : m_environment(a_environment), m_table(a_table) {}
        table(environment& a_environment, rtable& a_table) : m_environment(a_environment), m_table(&a_table) {}

        table(const table& a_table);

        table(table&& a_table) : m_environment(a_table.m_environment), m_table(a_table.m_table) {
            a_table.m_table = nullptr;
        }

        ~table();

        table& operator = (const table& a_table) noexcept;

        table& operator = (table&& a_table) noexcept;

        [[nodiscard]] object& operator [] (const object& a_key);

        [[nodiscard]] object& operator [] (const std::string_view a_key);

        [[nodiscard]] rtable* operator * () const noexcept {
            return m_table;
        }

        [[nodiscard]] rtable* get_rtable() const noexcept {
            return m_table;
        }

        [[nodiscard]] object& at(const object& a_key);

        [[nodiscard]] object& at(const std::string_view a_key);

        template <typename... t_args>
        decltype(auto) emplace(t_args&&... a_args);
    };
}

#endif //REBAR_TABLE_INTERFACE_HPP
