//
// Created by maxng on 6/21/2022.
//

#ifndef REBAR_INTERPRETER_HPP
#define REBAR_INTERPRETER_HPP

#include "provider.hpp"
#include "object.hpp"
#include "table.hpp"

namespace rebar {
    class environment;

    struct interpreter : public provider {
        class function_source {
        protected:
            environment& m_environment;

        public:
            explicit function_source(environment& a_environment) noexcept : m_environment(a_environment) {}

            [[nodiscard]] environment& env() noexcept {
                return m_environment;
            }

            virtual object internal_call() = 0;
        };

        class native_function_source : public function_source {
            callable m_function;

        public:
            native_function_source(environment& a_environment, callable a_callable) noexcept :
                    function_source(a_environment),
                    m_function(a_callable) {}

        protected:
            object internal_call() override {
                return m_function(&m_environment);
            }
        };

        class interpreted_function_source : public function_source {
            node::argument_list m_arguments;
            const node::block& m_body;

        public:
            interpreted_function_source(environment& a_environment, node::argument_list a_arguments, const node::block& a_body) noexcept :
                    function_source(a_environment),
                    m_arguments(std::move(a_arguments)),
                    m_body(a_body) {}

        protected:
            object internal_call() override;
        };

        explicit interpreter(environment& a_environment) noexcept : m_environment(a_environment), m_arguments(1) {}

        [[nodiscard]] function compile(parse_unit a_unit) override {
            m_parse_units.push_back(std::make_unique<parse_unit>(std::move(a_unit)));
            m_function_sources.emplace_back(dynamic_cast<function_source*>(new interpreted_function_source(m_environment, node::argument_list(), m_parse_units.back()->m_block)));
            return { m_environment, m_function_sources.back().get() };
        }

        [[nodiscard]] function bind(callable a_function) override {
            m_function_sources.emplace_back(dynamic_cast<function_source*>(new native_function_source(m_environment, a_function)));
            return { m_environment, m_function_sources.back().get() };
        }

        [[nodiscard]] object call(const void* a_data) override {
            // I know, I know. It should be relatively safe.
            auto* func = const_cast<function_source*>(reinterpret_cast<const function_source*>(a_data));

            return func->internal_call();
        }

    private:
        environment& m_environment;
        size_t m_argument_stack_position = 0;
        std::vector<std::vector<object>> m_arguments;
        std::vector<std::unique_ptr<parse_unit>> m_parse_units;
        std::vector<std::unique_ptr<function_source>> m_function_sources;
    };
}

#endif //REBAR_INTERPRETER_HPP
