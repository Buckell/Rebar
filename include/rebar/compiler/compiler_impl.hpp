//
// Created by maxng on 9/23/2022.
//

#ifndef REBAR_COMPILER_IMPL_HPP
#define REBAR_COMPILER_IMPL_HPP

#include "../environment.hpp"
#include "../compiler.hpp"

namespace rebar {
    void compiler::ithrow(rebar::environment* a_env, object* a_object, size_t a_string) {
        auto& comp = a_env->execution_provider<compiler>();
        auto& handler = comp.m_exception_handler_stack[comp.m_exception_handler_stack_position - 1];

        a_env->set_runtime_exception_information(string(reinterpret_cast<void*>(a_string)), *a_object);

        comp.perform_exception_cleanup(handler.function_stack_position);

        // TODO: Stack trace construction.

        comp.m_function_stack = handler.function_stack_position;

        std::longjmp(*handler.buffer, 1);
    }

    void compiler::rthrow(environment* a_env) {
        auto& comp = a_env->execution_provider<compiler>();
        auto& handler = comp.m_exception_handler_stack[comp.m_exception_handler_stack_position - 1];

        comp.perform_exception_cleanup(handler.function_stack_position);

        // TODO: Stack trace construction.

        comp.m_function_stack = handler.function_stack_position;

        std::longjmp(*handler.buffer, 1);
    }

    [[noreturn]] void compiler::throw_exception() {
        auto& handler = m_exception_handler_stack[m_exception_handler_stack_position - 1];

        perform_exception_cleanup(handler.function_stack_position);

        // TODO: Stack trace construction.

        m_function_stack = handler.function_stack_position;

        std::longjmp(*handler.buffer, 1);
    }
}

#endif //REBAR_COMPILER_IMPL_HPP
