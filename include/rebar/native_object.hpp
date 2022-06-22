//
// Created by maxng on 6/21/2022.
//

#ifndef REBAR_NATIVE_OBJECT_HPP
#define REBAR_NATIVE_OBJECT_HPP

#include <type_traits>
#include <utility>

namespace rebar {
    class object;

    struct virtual_table;

    template <typename t_object>
    struct structured_native_object {
        using destructor_function = void (*)(void*);

        size_t m_reference_count;
        virtual_table* m_virtual_table;
        destructor_function m_destructor;
        size_t m_data_size;
        t_object m_data;
    };

    class native_object {
        using destructor_function = void (*)(void*);

        void* m_root_pointer;

        // 0 00  size_t m_ref_count;
        // 1 08 virtual_table* m_v_table;
        // 2 16 destructor_function m_destructor;
        // 3 24 size_t m_data_size;
        // 4 32 uint8_t m_data[m_data_size];

    public:
        native_object(void* a_data) noexcept : m_root_pointer(a_data) {
            reference();
        }

        native_object(const native_object& a_native_object) noexcept : m_root_pointer(a_native_object.m_root_pointer) {
            reference();
        }

        native_object(native_object&& a_native_object) noexcept : m_root_pointer(a_native_object.m_root_pointer) {
            reference();
        }

        ~native_object() noexcept {
            if (m_root_pointer != nullptr) {
                dereference();
            }
        }

        native_object& operator = (const native_object& a_native_object) noexcept {
            dereference();
            m_root_pointer = a_native_object.m_root_pointer;
            reference();

            return *this;
        }

        native_object& operator = (native_object&& a_native_object) noexcept {
            dereference();

            m_root_pointer = a_native_object.m_root_pointer;
            a_native_object.m_root_pointer = nullptr;

            return *this;
        }

        template <typename t_object>
        static native_object create(virtual_table& a_v_table, t_object a_object) {
            destructor_function destructor = nullptr;

            if constexpr (!std::is_trivially_destructible_v<t_object>) {
                destructor = [](void* a_obj) noexcept(std::is_nothrow_destructible_v<t_object>) {
                    reinterpret_cast<t_object*>(a_obj)->~t_object();
                };
            }

            structured_native_object<t_object>* obj = reinterpret_cast<structured_native_object<t_object>*>(std::malloc(sizeof(structured_native_object<t_object>)));

            obj->m_reference_count = 0;          // Reference count. Initialize to 0. "Real" lifetime begins when native_object is constructed and returned.
            obj->m_virtual_table = &a_v_table;   // Virtual table.
            obj->m_destructor = destructor;      // Destructor for garbage collection.
            obj->m_data_size = sizeof(t_object); // Data size (size of object in bytes).

            new (&obj->m_data) t_object(std::move(a_object)); // Data (object).

            return { reinterpret_cast<void*>(obj) };
        }

        template <typename t_object, typename... t_args>
        static native_object create(virtual_table& a_v_table, std::in_place_type_t<t_object>, t_args... a_args) {
            destructor_function destructor = nullptr;

            if constexpr (!std::is_trivially_destructible_v<t_object>) {
                destructor = [](void* a_obj){
                    reinterpret_cast<t_object*>(a_obj)->~t_object();
                };
            }

            structured_native_object<t_object>* obj = reinterpret_cast<structured_native_object<t_object>*>(std::malloc(sizeof(structured_native_object<t_object>)));

            obj->m_reference_count = 0;          // Reference count. Initialize to 0. "Real" lifetime begins when native_object is constructed and returned.
            obj->m_virtual_table = &a_v_table;   // Virtual table.
            obj->m_destructor = destructor;      // Destructor for garbage collection.
            obj->m_data_size = sizeof(t_object); // Data size (size of object in bytes).

            new (&obj->m_data) t_object(std::forward<t_args>(a_args)...); // Data (object).

            return { reinterpret_cast<void*>(obj) };
        }

        [[nodiscard]] constexpr void* data() noexcept {
            return m_root_pointer;
        }

        [[nodiscard]] constexpr const void* data() const noexcept {
            return m_root_pointer;
        }

        [[nodiscard]] inline size_t get_reference_count() const noexcept {
            return reinterpret_cast<structured_native_object<std::nullptr_t>*>(m_root_pointer)->m_reference_count;
        }

        [[nodiscard]] inline virtual_table& get_virtual_table() noexcept {
            return *reinterpret_cast<structured_native_object<std::nullptr_t>*>(m_root_pointer)->m_virtual_table;
        }

        [[nodiscard]] inline destructor_function get_destructor() const noexcept {
            return reinterpret_cast<structured_native_object<std::nullptr_t>*>(m_root_pointer)->m_destructor;
        }

        [[nodiscard]] inline size_t get_object_size() const noexcept {
            return reinterpret_cast<structured_native_object<std::nullptr_t>*>(m_root_pointer)->m_data_size;
        }

        template <typename t_object>
        [[nodiscard]] inline t_object& get_object() const noexcept {
            return reinterpret_cast<structured_native_object<t_object>*>(m_root_pointer)->m_data;
        }

                      inline void    overload_assignment                  (environment& a_env, const object rhs);
        [[nodiscard]] inline object  overload_addition                    (environment& a_env, const object rhs);
                      inline void    overload_addition_assignment         (environment& a_env, const object rhs);
        [[nodiscard]] inline object  overload_multiplication              (environment& a_env, const object rhs);
                      inline void    overload_multiplication_assignment   (environment& a_env, const object rhs);
        [[nodiscard]] inline object  overload_division                    (environment& a_env, const object rhs);
                      inline void    overload_division_assignment         (environment& a_env, const object rhs);
        [[nodiscard]] inline object  overload_subtraction                 (environment& a_env, const object rhs);
                      inline void    overload_subtraction_assignment      (environment& a_env, const object rhs);
        [[nodiscard]] inline object  overload_equality                    (environment& a_env, const object rhs);
        [[nodiscard]] inline object  overload_inverse_equality            (environment& a_env, const object rhs);
        [[nodiscard]] inline object  overload_greater                     (environment& a_env, const object rhs);
        [[nodiscard]] inline object  overload_lesser                      (environment& a_env, const object rhs);
        [[nodiscard]] inline object  overload_greater_equality            (environment& a_env, const object rhs);
        [[nodiscard]] inline object  overload_lesser_equality             (environment& a_env, const object rhs);
        [[nodiscard]] inline object  overload_logical_or                  (environment& a_env, const object rhs);
        [[nodiscard]] inline object  overload_logical_and                 (environment& a_env, const object rhs);
        [[nodiscard]] inline object  overload_logical_not                 (environment& a_env);
        [[nodiscard]] inline object  overload_bitwise_or                  (environment& a_env, const object rhs);
                      inline void    overload_bitwise_or_assignment       (environment& a_env, const object rhs);
        [[nodiscard]] inline object  overload_bitwise_xor                 (environment& a_env, const object rhs);
                      inline void    overload_bitwise_xor_assignment      (environment& a_env, const object rhs);
        [[nodiscard]] inline object  overload_bitwise_and                 (environment& a_env, const object rhs);
                      inline void    overload_bitwise_and_assignment      (environment& a_env, const object rhs);
        [[nodiscard]] inline object  overload_bitwise_not                 (environment& a_env);
        [[nodiscard]] inline object  overload_shift_right                 (environment& a_env, const object rhs);
                      inline void    overload_shift_right_assignment      (environment& a_env, const object rhs);
        [[nodiscard]] inline object  overload_shift_left                  (environment& a_env, const object rhs);
                      inline void    overload_shift_left_assignment       (environment& a_env, const object rhs);
        [[nodiscard]] inline object  overload_exponent                    (environment& a_env, const object rhs);
                      inline void    overload_exponent_assignment         (environment& a_env, const object rhs);
        [[nodiscard]] inline object  overload_modulus                     (environment& a_env, const object rhs);
                      inline void    overload_modulus_assignment          (environment& a_env, const object rhs);
                      inline object  overload_length                      (environment& a_env);
                      inline void    overload_prefix_increment            (environment& a_env);
                      inline object  overload_postfix_increment           (environment& a_env);
                      inline void    overload_prefix_decrement            (environment& a_env);
                      inline object  overload_postfix_decrement           (environment& a_env);
        [[nodiscard]] inline object& overload_index                       (environment& a_env, const object rhs);
        [[nodiscard]] inline object  overload_select                      (environment& a_env, const object rhs);
        [[nodiscard]] inline object  overload_ranged_select               (environment& a_env, const object lhs, const object rhs);
        [[nodiscard]] inline object  overload_call                        (environment& a_env);
        [[nodiscard]] inline object  overload_new                         (environment& a_env);

        void reference() noexcept {
            ++reinterpret_cast<structured_native_object<std::nullptr_t>*>(m_root_pointer)->m_reference_count;
        }

        void dereference() noexcept {
            if (--reinterpret_cast<structured_native_object<std::nullptr_t>*>(m_root_pointer)->m_reference_count == 0) {
                destructor_function destructor = get_destructor();

                if (destructor != nullptr) {
                    destructor(&reinterpret_cast<structured_native_object<std::nullptr_t>*>(m_root_pointer)->m_data);
                }

                std::free(m_root_pointer);
            }
        }
    };
}

#endif //REBAR_NATIVE_OBJECT_HPP
