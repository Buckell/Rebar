//
// Created by maxng on 6/21/2022.
//

#ifndef REBAR_STRING_HPP
#define REBAR_STRING_HPP

#include "utility.hpp"

#include <xxhash.hpp>

namespace rebar {
    class environment;

    class string {
        // Stores size (size_t), reference count (size_t),
        // and string contents (char[]) in contiguous block of memory.
        //
        // Similar to Pascal strings. Optimized for Rebar object storage.

        // [size_t size][size_t reference count][environment* env][char[] data]

        void* m_root_pointer;

    public:
        explicit string(void* a_root_pointer) noexcept : m_root_pointer(a_root_pointer) {
            reference();
        }

        string(const string& a_string) noexcept : m_root_pointer(a_string.m_root_pointer) {
            reference();
        }

        string(environment& a_env, std::string_view a_string);

        ~string() noexcept {
            dereference();
        }

        [[nodiscard]] const char* c_str() const noexcept {
            return reinterpret_cast<char*>(m_root_pointer) + (sizeof(size_t) * 3);
        }

        [[nodiscard]] size_t length() const noexcept {
            return reinterpret_cast<size_t*>(m_root_pointer)[0];
        }

        [[nodiscard]] size_t reference_count() const noexcept {
            return reinterpret_cast<size_t*>(m_root_pointer)[1];
        }

        [[nodiscard]] environment* env() const noexcept {
            return reinterpret_cast<environment**>(m_root_pointer)[2];
        }

        [[nodiscard]] size_t size() const noexcept {
            return length() + sizeof(size_t);
        }

        [[nodiscard]] const void* data() const noexcept {
            return m_root_pointer;
        }

        [[nodiscard]] std::string_view to_string_view() const noexcept {
            return { c_str(), length() };
        }

        [[nodiscard]] bool operator==(const string& rhs) const noexcept {
            return m_root_pointer == rhs.m_root_pointer;
        }

        [[nodiscard]] bool operator!=(const string& rhs) const noexcept {
            return m_root_pointer != rhs.m_root_pointer;
        }

        friend std::ostream& operator<<(std::ostream& lhs, const string& rhs) {
            return (lhs << rhs.to_string_view());
        }

        inline size_t reference(const size_t a_increment = 1) noexcept {
            return reinterpret_cast<size_t*>(m_root_pointer)[1] += a_increment;
        }

        inline size_t dereference(const size_t a_decrement = 1) noexcept {
            size_t ref_count = (reinterpret_cast<size_t*>(m_root_pointer)[1] -= a_decrement);

            if (ref_count == 0) {
                deallocate();
            }

            return ref_count;
        }

    private:
        explicit string(environment* a_env, const std::string_view a_string) noexcept : m_root_pointer(reinterpret_cast<size_t*>(std::malloc((sizeof(size_t) * 3) + a_string.size() + 1))) {
            auto* root_pointer = reinterpret_cast<size_t*>(m_root_pointer);

            // String size/length.
            root_pointer[0] = a_string.length();

            // String reference counter.
            root_pointer[1] = 1;

            root_pointer[2] = reinterpret_cast<size_t>(a_env);

            // String data.
            memcpy(reinterpret_cast<char*>(m_root_pointer) + (sizeof(size_t) * 3), a_string.data(), a_string.size());
            *(reinterpret_cast<char*>(m_root_pointer) + a_string.size() + (sizeof(size_t) * 3)) = 0;
        }

        void deallocate();

        void set_reference_count(const size_t a_count) noexcept {
            reinterpret_cast<size_t*>(m_root_pointer)[1] = a_count;
        }

        friend class environment;

        friend struct object;
    };

    struct xxh_string_view_hash {
        size_t operator()(const std::string_view a_string) const noexcept {
            return xxh::xxhash3<rebar::cpu_bit_architecture()>(a_string);
        }
    };
}

template <>
struct std::hash <rebar::string> {
    size_t operator()(const rebar::string& a_string) const noexcept {
        return xxh::xxhash3<rebar::cpu_bit_architecture()>(a_string.to_string_view());
    }
};

#endif //REBAR_STRING_HPP
