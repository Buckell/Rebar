//
// Created by maxng on 6/21/2022.
//

#ifndef REBAR_UTILITY_HPP
#define REBAR_UTILITY_HPP

#include <functional>
#include <sstream>
#include <fstream>

#include "definitions.hpp"

namespace rebar {
    [[nodiscard]] constexpr bool is_integer_string(const std::string_view a_string) noexcept {
        return !a_string.empty() && a_string.find_first_not_of("0123456789-+") == std::string::npos;
    }

    [[nodiscard]] constexpr bool is_number_string(const std::string_view a_string) noexcept {
        return !a_string.empty() && a_string.find_first_not_of("0123456789.-+") == std::string::npos;
    }

    [[nodiscard]] constexpr bool is_integer_string(const std::wstring_view a_string) noexcept {
        return !a_string.empty() && a_string.find_first_not_of(L"0123456789-+") == std::string::npos;
    }

    [[nodiscard]] constexpr bool is_number_string(const std::wstring_view a_string) noexcept {
        return !a_string.empty() && a_string.find_first_not_of(L"0123456789.-+") == std::string::npos;
    }

    [[nodiscard]] constexpr size_t cpu_bit_architecture() noexcept {
        if constexpr (sizeof(void*) == 8) {
            return 64;
        } else {
            return 32;
        }
    }

    // Finds the next object that is equal to a_search in an iterable container and
    // returns an iterator to that object. The last two arguments are for excluding finding
    // within those two objects. For example, if you have a vector { 'a', '(', 'b', ')', 'b' }
    // and are searching for the character 'b' but with a_open_exclude and a_close_exclude as
    // '(' and ')', respectively, then it will return an iterator to the last character 'b'
    // because the first character is surrounded by the characters '(' and ')'. Useful for
    // finding characters on the "same level" in recursive/enclosed sequences.

    // A function may be used for a_search of signature:
    // bool (const t_container::value_type&)

    // A function may be used for a_open_exclude of signature:
    // find_next_exclude (const t_container::value_type&)

    // * If a function is specified for a_open_exclude, a_close_exclude
    //   is not used and can be left unspecified.

    enum class find_next_exclude : enum_base {
        none,
        open,
        close
    };

    template <typename t_container, typename t_search, typename t_exclude_open = std::nullptr_t, typename t_exclude_close = std::nullptr_t>
    [[nodiscard]] static typename t_container::iterator find_next(const t_container& a_container, const t_search& a_search, const t_exclude_open& a_open_exclude = nullptr, const t_exclude_close& a_close_exclude = nullptr) noexcept {
        typename t_container::iterator it = a_container.begin();
        typename t_container::iterator end_it = a_container.end();

        // A "search" function has been specified.
        constexpr bool using_search_function = std::is_convertible_v<t_search, const std::function<bool (const typename t_container::value_type&)>>;

        // An "open/close exclude" function has been specified.
        constexpr bool using_exclude_function = std::is_convertible_v<t_search, const std::function<find_next_exclude (const typename t_container::value_type&)>>;

        // Standard "open/close exclude" objects have been specified.
        constexpr bool using_exclude = !using_exclude_function && !std::is_same_v<t_exclude_open, std::nullptr_t> && !std::is_same_v<t_exclude_close, std::nullptr_t>;

        size_t exclude_increment = 0;

        for (; it != end_it; ++it) {
            if constexpr (using_search_function) {
                const std::function<bool (const typename t_container::value_type&)> search_function = a_search;

                if (search_function(*it) && exclude_increment == 0) {
                    return it;
                }
            } else {
                if (*it == a_search && exclude_increment == 0) {
                    return it;
                }
            }

            if constexpr (using_exclude) {
                exclude_increment += *it == a_open_exclude;
                exclude_increment -= *it == a_close_exclude;
            } else if constexpr (using_exclude_function) {
                const std::function<find_next_exclude (const typename t_container::value_type&)> exclude_function = a_open_exclude;

                switch (exclude_function(*it)) {
                    case find_next_exclude::open:
                        exclude_increment++;
                        break;
                    case find_next_exclude::close:
                        exclude_increment--;
                        break;
                    default:
                        break;
                }
            }
        }

        return end_it;
    }

    inline void* aligned_alloc(const size_t a_alignment, const size_t a_size) {
#if defined(_MSC_VER)
        return _aligned_malloc(a_size, a_alignment);
#else
        return std::aligned_alloc(a_alignment, a_size);
#endif
        return nullptr;
    }

    [[maybe_unused]] [[nodiscard]] std::string read_file(const std::string_view a_file) {
        std::ifstream file_stream(std::string(a_file), std::ifstream::in);
        std::stringstream in_string_stream;
        in_string_stream << file_stream.rdbuf();
        return in_string_stream.str();
    }

    template <typename t_pointer>
    constexpr inline t_pointer* pointer_byte_offset(void* a_ptr, ptrdiff_t a_offset) noexcept {
        return reinterpret_cast<t_pointer*>(reinterpret_cast<uint8_t*>(a_ptr) + a_offset);
    }

    template <typename t_pointer>
    constexpr inline const t_pointer* pointer_byte_offset(const void* a_ptr, ptrdiff_t a_offset) noexcept {
        return reinterpret_cast<const t_pointer*>(reinterpret_cast<const uint8_t*>(a_ptr) + a_offset);
    }

    template <typename t_type>
    class has_std_size_function {
        template <typename t_c, typename = decltype(std::declval<t_c>().size())>
        static std::true_type test(int) {}

        template <typename t_c>
        static std::false_type test(...) {}

    public:
        constexpr static bool value = decltype(test<t_type>(0))::value;
    };

    template <typename t_type>
    class has_std_data_function {
        template <typename t_c, typename = decltype(std::declval<t_c>().data())>
        static std::true_type test(int) {}

        template <typename t_c>
        static std::false_type test(...) {}

    public:
        constexpr static bool value = decltype(test<t_type>(0))::value;
    };

    template <typename t_destination, typename t_source>
    t_destination bitcast(t_source a_source) {
        static_assert(sizeof(t_destination) == sizeof(t_source));

        t_destination dest;
        memcpy_s(&dest, sizeof(t_destination), &a_source, sizeof(t_source));
        return dest;
    }
}

#endif //REBAR_UTILITY_HPP
