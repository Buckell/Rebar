//
// Created by maxng on 6/25/2022.
//

#ifndef REBAR_STRING_BASE_HPP
#define REBAR_STRING_BASE_HPP

#include <algorithm>

#include "../rebar.hpp"

namespace rebar::library::standard {
    struct string_base : public library {
        string_base() : library(usage::implicit_include) {}

        static object Contains(environment* a_environment) {
            return null;
        }

        static object EndsWith(environment* a_environment) {
            std::string_view self = a_environment->arg(0).get_string().to_string_view();
            std::string_view compare = a_environment->arg(1).get_string().to_string_view();

            if (compare.length() > self.length()) {
                return false;
            }

            return self.substr(self.length() - compare.length(), compare.length()) == compare;
        }

        static object EqualsIgnoreCase(environment* a_environment) {
            std::string_view self = a_environment->arg(0).get_string().to_string_view();
            std::string_view compare = a_environment->arg(1).get_string().to_string_view();

            if (self.length() != compare.length()) {
                return false;
            }

            for (size_t i = 0; i < self.length(); ++i) {
                if (std::tolower(self[i]) != std::tolower(compare[i])) {
                    return false;
                }
            }

            return true;
        }

        static object IndexOf(environment* a_environment) {
            return null;
        }

        static object IsEmpty(environment* a_environment) {
            return null;
        }

        static object LastIndexOf(environment* a_environment) {
            return null;
        }

        static object Length(environment* a_environment) {
            return static_cast<integer>(a_environment->arg(0).get_string().length());
        }

        static object Matches(environment* a_environment) {
            return null;
        }

        static object Replace(environment* a_environment) {
            return null;
        }

        static object Split(environment* a_environment) {
            return null;
        }

        static object StartsWith(environment* a_environment) {
            std::string_view self = a_environment->arg(0).get_string().to_string_view();
            std::string_view compare = a_environment->arg(1).get_string().to_string_view();

            if (compare.length() > self.length()) {
                return false;
            }

            return self.substr(0, compare.length()) == compare;
        }

        static object ToCharArray(environment* a_environment) {
            return null;
        }

        static object ToLowerCase(environment* a_environment) {
            std::string_view self = a_environment->arg(0).get_string().to_string_view();

            std::string output;
            output.reserve(self.length());

            std::transform(self.begin(), self.end(), output.begin(), static_cast<int(*)(int)>(std::tolower));

            return a_environment->str(output);
        }

        static object ToUpperCase(environment* a_environment) {
            std::string_view self = a_environment->arg(0).get_string().to_string_view();

            std::string output;
            output.reserve(self.length());

            std::transform(self.begin(), self.end(), output.begin(), static_cast<int(*)(int)>(std::tolower));

            return a_environment->str(output);
        }

        static object Trim(environment* a_environment) {
            std::string_view self = a_environment->arg(0).get_string().to_string_view();

            auto begin_it = std::find_if(self.begin(), self.end(), [](const auto ch) noexcept -> bool {
                return !std::isspace(ch);
            });

            auto end_it = std::find_if(self.rbegin(), self.rend(), [](const auto ch) noexcept -> bool {
                return !std::isspace(ch);
            });

            const auto begin_difference = begin_it - self.cbegin();
            const auto end_difference = end_it - self.crbegin();

            return a_environment->str(self.substr(begin_difference, self.length() - (begin_difference + end_difference)));
        }

        static object TrimLeft(environment* a_environment) {
            std::string_view self = a_environment->arg(0).get_string().to_string_view();

            auto begin_it = std::find_if(self.begin(), self.end(), [](const auto ch) noexcept -> bool {
                return !std::isspace(ch);
            });

            return a_environment->str(self.substr(begin_it - self.begin()));
        }

        static object TrimRight(environment* a_environment) {
            std::string_view self = a_environment->arg(0).get_string().to_string_view();

            auto end_it = std::find_if(self.rbegin(), self.rend(), [](const auto ch) noexcept -> bool {
                return !std::isspace(ch);
            });

            return a_environment->str(self.substr(0, self.length() - (end_it - self.rbegin())));
        }

        object load(environment& a_environment) override {
            auto& string_table = a_environment.get_string_virtual_table();

            const auto define_string_function = [&a_environment, &string_table](const std::string_view a_identifier, callable a_function) noexcept {
                string_table[a_environment.str(a_identifier)] = a_environment.bind(a_function, std::string(a_identifier), "REBAR::STD::STRING_BASE");
            };

            define_string_function("Contains",         Contains);
            define_string_function("EndsWith",         EndsWith);
            define_string_function("EqualsIgnoreCase", EqualsIgnoreCase);
            define_string_function("IndexOf",          IndexOf);
            define_string_function("IsEmpty",          IsEmpty);
            define_string_function("LastIndexOf",      LastIndexOf);
            define_string_function("Length",           Length);
            define_string_function("Matches",          Matches);
            define_string_function("Replace",          Replace);
            define_string_function("Split",            Split);
            define_string_function("StartsWith",       StartsWith);
            define_string_function("ToCharArray",      ToCharArray);
            define_string_function("ToLowerCase",      ToLowerCase);
            define_string_function("ToUpperCase",      ToUpperCase);
            define_string_function("Trim",             Trim);
            define_string_function("TrimLeft",         TrimLeft);
            define_string_function("TrimRight",        TrimRight);

            return null;
        }
    };

    const char library_string_base[] = "StringBase";
    define_library<library_string_base, string_base> d_string_base;
}

#endif //REBAR_STRING_BASE_HPP
