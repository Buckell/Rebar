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

        static REBAR_FUNCTION(Contains) {
            std::string_view self = env->arg(0).get_string().to_string_view();
            std::string_view compare = env->arg(1).get_string().to_string_view();

            size_t self_size = self.size();
            size_t compare_size = compare.size();

            if (self_size < compare_size) {
                REBAR_RETURN(boolean_false);
            }

            size_t compare_index = 0;

            for (size_t i = 0; i <= self_size; ++i) {
                if (self[i] == compare[compare_index]) {
                    ++compare_index;

                    if (compare_index >= compare_size) {
                        REBAR_RETURN(boolean_true);
                    }
                } else {
                    compare_index = 0;

                    if (i > (self_size - compare_size)) {
                        REBAR_RETURN(boolean_false);
                    }
                }
            }

            REBAR_RETURN(boolean_false);
        }

        static REBAR_FUNCTION(EndsWith) {
            std::string_view self = env->arg(0).get_string().to_string_view();
            std::string_view compare = env->arg(1).get_string().to_string_view();

            if (compare.length() > self.length()) {
                REBAR_RETURN(boolean_false);
            }

            REBAR_RETURN(object::from_bool(self.substr(self.length() - compare.length(), compare.length()) == compare));
        }

        static REBAR_FUNCTION(EqualsIgnoreCase) {
            std::string_view self = env->arg(0).get_string().to_string_view();
            std::string_view compare = env->arg(1).get_string().to_string_view();

            if (self.length() != compare.length()) {
                REBAR_RETURN(boolean_false);
            }

            for (size_t i = 0; i < self.length(); ++i) {
                if (std::tolower(self[i]) != std::tolower(compare[i])) {
                    REBAR_RETURN(boolean_false);
                }
            }

            REBAR_RETURN(boolean_true);
        }

        static REBAR_FUNCTION(IndexOf) {
            std::string_view self = env->arg(0).get_string().to_string_view();
            std::string_view compare = env->arg(1).get_string().to_string_view();

            size_t self_size = self.size();
            size_t compare_size = compare.size();

            if (self_size < compare_size) {
                REBAR_RETURN(null);
            }

            size_t start_index = 0;
            size_t compare_index = 0;

            for (size_t i = 0; i <= self_size; ++i) {
                if (self[i] == compare[compare_index]) {
                    if (compare_index == 0) {
                        start_index = i;
                    }

                    ++compare_index;

                    if (compare_index >= compare_size) {
                        REBAR_RETURN(static_cast<integer>(start_index));
                    }
                } else {
                    compare_index = 0;

                    if (i > (self_size - compare_size)) {
                        REBAR_RETURN(null);
                    }
                }
            }

            REBAR_RETURN(null);
        }

        static REBAR_FUNCTION(IsEmpty) {
            std::string_view self = env->arg(0).get_string().to_string_view();
            REBAR_RETURN(object::from_bool(self.empty()));
        }

        static REBAR_FUNCTION(LastIndexOf) {
            std::string_view self = env->arg(0).get_string().to_string_view();
            std::string_view compare = env->arg(1).get_string().to_string_view();

            size_t self_size = self.size();
            size_t compare_size = compare.size();

            if (self_size < compare_size) {
                REBAR_RETURN(null);
            }

            size_t compare_index = 0;

            for (size_t i = self_size - 1; i > 0; --i) {
                if (self[i] == compare[compare_size - compare_index - 1]) {
                    ++compare_index;

                    if (compare_index >= compare_size) {
                        REBAR_RETURN(static_cast<integer>(i));
                    }
                } else {
                    compare_index = 0;

                    if (i < compare_size) {
                        REBAR_RETURN(null);
                    }
                }
            }

            REBAR_RETURN(null);
        }

        static REBAR_FUNCTION(Length) {
            REBAR_RETURN(static_cast<integer>(env->arg(0).get_string().length()));
        }

        static REBAR_FUNCTION(Matches) {
            // TODO: Implement REGEX.
            REBAR_RETURN(null);
        }

        static REBAR_FUNCTION(Replace) {
            std::string_view self = env->arg(0).get_string().to_string_view();
            std::string_view search = env->arg(1).get_string().to_string_view();
            std::string_view replace = env->arg(2).get_string().to_string_view();

            size_t self_size = self.size();
            size_t search_size = search.size();
            size_t replace_size = replace.size();

            size_t start_index = self_size + 1;
            size_t compare_index = 0;

            if (search_size == replace_size) {
                std::string buffer{ self };

                for (size_t i = 0; i <= self_size; ++i) {
                    if (self[i] == search[compare_index]) {
                        if (compare_index == 0) {
                            start_index = i;
                        }

                        ++compare_index;

                        if (compare_index >= search_size) {
                            compare_index = 0;
                            memcpy(buffer.data() + start_index, replace.data(), replace_size);
                        }
                    } else {
                        compare_index = 0;

                        if (i > (self_size - search_size)) {
                            REBAR_RETURN(env->str(buffer));
                        }
                    }
                }
            } else {
                std::string buffer;
                buffer.reserve(self_size);

                for (size_t i = 0; i <= self_size; ++i) {
                    if (self[i] == search[compare_index]) {
                        ++compare_index;

                        if (compare_index >= search_size) {
                            compare_index = 0;
                            buffer += replace;
                        }
                    } else {
                        compare_index = 0;

                        if (i > (self_size - search_size)) {
                            buffer += self.substr(i);
                            REBAR_RETURN(env->str(buffer));
                        }

                        buffer += self[i];
                    }
                }

                REBAR_RETURN(env->str(buffer));
            }
        }

        static REBAR_FUNCTION(Split) {
            std::string_view self = env->arg(0).get_string().to_string_view();
            std::string_view delimiter = env->arg(1).get_string().to_string_view();

            size_t self_size = self.size();
            size_t delimiter_size = delimiter.size();

            array arr(4);

            size_t begin_index = 0;

            if (delimiter_size == 1) {
                char delimiter_character = delimiter[0];

                for (size_t i = 0; i < self_size; ++i) {
                    char c = self[i];

                    if (c == delimiter_character) {
                        if (i - begin_index > 0) {
                            arr.push_back(env->str(self.substr(begin_index, i - begin_index)));
                        }

                        begin_index = i + 1;
                    }
                }

                if (begin_index != self_size - 1) {
                    std::string_view str = self.substr(begin_index);

                    if (!str.empty()) {
                        arr.push_back(env->str(self.substr(begin_index)));
                    }
                }
            } else {
                size_t compare_index = 0;

                for (size_t i = 0; i < self_size; ++i) {
                    if (self[i] == delimiter[compare_index]) {
                        ++compare_index;

                        if (compare_index >= delimiter_size) {
                            compare_index = 0;

                            if (i - begin_index - delimiter_size + 1 > 0) {
                                arr.push_back(env->str(self.substr(begin_index, i - begin_index - delimiter_size + 1)));
                            }

                            begin_index = i + 1;
                        }
                    } else {
                        compare_index = 0;

                        if (i > (self_size - delimiter_size)) {
                            break;
                        }
                    }
                }

                std::string_view str = self.substr(begin_index);
                if (!str.empty()) {
                    arr.push_back(env->str(str));
                }
            }

            REBAR_RETURN(arr);
        }

        static REBAR_FUNCTION(StartsWith) {
            std::string_view self = env->arg(0).get_string().to_string_view();
            std::string_view compare = env->arg(1).get_string().to_string_view();

            if (compare.length() > self.length()) {
                REBAR_RETURN(boolean_false);
            }

            REBAR_RETURN(object::from_bool(self.substr(0, compare.length()) == compare));
        }

        static REBAR_FUNCTION(ToCharArray) {
            std::string_view self = env->arg(0).get_string().to_string_view();

            array arr(self.size());

            for (char c : self) {
                arr.push_back(static_cast<integer>(c));
            }

            REBAR_RETURN(arr);
        }

        static REBAR_FUNCTION(ToLowerCase) {
            std::string_view self = env->arg(0).get_string().to_string_view();

            std::string output;
            output.reserve(self.length());

            std::transform(self.begin(), self.end(), std::back_inserter(output), static_cast<int(*)(int)>(std::tolower));

            *ret = env->str(output);
        }

        static REBAR_FUNCTION(ToUpperCase) {
            std::string_view self = env->arg(0).get_string().to_string_view();

            std::string output;
            output.reserve(self.length());

            std::transform(self.begin(), self.end(), std::back_inserter(output), static_cast<int(*)(int)>(std::toupper));

            *ret = env->str(output);
        }

        static REBAR_FUNCTION(Trim) {
            std::string_view self = env->arg(0).get_string().to_string_view();

            auto begin_it = std::find_if(self.begin(), self.end(), [](const auto ch) noexcept -> bool {
                return !std::isspace(ch);
            });

            auto end_it = std::find_if(self.rbegin(), self.rend(), [](const auto ch) noexcept -> bool {
                return !std::isspace(ch);
            });

            const auto begin_difference = begin_it - self.cbegin();
            const auto end_difference = end_it - self.crbegin();

            *ret = env->str(self.substr(begin_difference, self.length() - (begin_difference + end_difference)));
        }

        static REBAR_FUNCTION(TrimLeft) {
            std::string_view self = env->arg(0).get_string().to_string_view();

            auto begin_it = std::find_if(self.begin(), self.end(), [](const auto ch) noexcept -> bool {
                return !std::isspace(ch);
            });

            *ret = env->str(self.substr(begin_it - self.begin()));
        }

        static REBAR_FUNCTION(TrimRight) {
            std::string_view self = env->arg(0).get_string().to_string_view();

            auto end_it = std::find_if(self.rbegin(), self.rend(), [](const auto ch) noexcept -> bool {
                return !std::isspace(ch);
            });

            *ret = env->str(self.substr(0, self.length() - (end_it - self.rbegin())));
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

    REBAR_DEFINE_LIBRARY("StringBase", string_base);
}

#endif //REBAR_STRING_BASE_HPP
