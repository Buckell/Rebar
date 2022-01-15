#pragma once

#include <rebar.hpp>

namespace rebar {
    namespace standard {
        enum class usage {
            implicit_include,
            explicit_include
        };

        struct library {
            usage include;

            library(usage a_include) noexcept : include(a_include) {}

            virtual object load(environment&) = 0;
        };

        std::map<std::string_view, library*> registry;

        template <const char* v_identifier, class t_library>
        struct define_library {
            define_library() {
                registry[v_identifier] = new t_library;
            }
        };

        object load_library(environment& a_environment, const std::string_view a_identifier) {
            return registry[a_identifier]->load(a_environment);
        }

        void load_implicit_libraries(environment& a_environment) {
            for (auto& lib : registry) {
                if (lib.second->include == usage::implicit_include) {
                    lib.second->load(a_environment);
                }
            }
        }

        struct base : public library {
            base() : library(usage::implicit_include) {}

            static rebar::object PrintLn(rebar::environment* env) {
                if (env->arg_count() == 0) {
                    std::cout << '\n';
                }

                std::cout << env->arg(0);

                for (size_t i = 1; i < env->arg_count(); i++) {
                    std::cout << "    " << env->arg(i);
                }

                std::cout << '\n';

                return rebar::null;
            }

            static rebar::object Print(rebar::environment* env) {
                std::cout << env->arg(0);

                for (size_t i = 1; i < env->arg_count(); i++) {
                    std::cout << "    " << env->arg(i);
                }

                return rebar::null;
            }

            static rebar::object Include(rebar::environment* env) {
                std::string_view sv = env->arg(0).get_string().to_string_view();
                return load_library(*env, sv);
            }

            static rebar::object Input(rebar::environment* env) {
                std::string input;
                std::getline(std::cin, input);
                return env->str(input);
            }

            object load(environment& a_environment) override {
                auto& global_table = a_environment.global_table();

                const auto define_global_function = [&a_environment, &global_table](const std::string_view a_identifier, callable a_function) noexcept {
                    global_table[a_environment.str(a_identifier)] = a_environment.bind(a_function);
                };

                define_global_function("PrintLn", PrintLn);
                define_global_function("Print", Print);
                define_global_function("Include", Include);
                define_global_function("Input", Input);

                return null;
            }
        };

        const char library_base[] = "Base";
        define_library<library_base, base> d_base;


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

                std::transform(self.begin(), self.end(), output.begin(), std::tolower);

                return a_environment->str(output);
            }

            static object ToUpperCase(environment* a_environment) {
                std::string_view self = a_environment->arg(0).get_string().to_string_view();

                std::string output;
                output.reserve(self.length());

                std::transform(self.begin(), self.end(), output.begin(), std::toupper);

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
                    string_table[a_environment.str(a_identifier)] = a_environment.bind(a_function);
                };

                define_string_function("contains",         Contains);
                define_string_function("endsWith",         EndsWith);
                define_string_function("equalsIgnoreCase", EqualsIgnoreCase);
                define_string_function("indexOf",          IndexOf);
                define_string_function("isEmpty",          IsEmpty);
                define_string_function("lastIndexOf",      LastIndexOf);
                define_string_function("length",           Length);
                define_string_function("matches",          Matches);
                define_string_function("replace",          Replace);
                define_string_function("split",            Split);
                define_string_function("startsWith",       StartsWith);
                define_string_function("toCharArray",      ToCharArray);
                define_string_function("toLowerCase",      ToLowerCase);
                define_string_function("toUpperCase",      ToUpperCase);
                define_string_function("trim",             Trim);
                define_string_function("trimLeft",         TrimLeft);
                define_string_function("trimRight",        TrimRight);

                return null;
            }
        };

        const char library_string_base[] = "StringBase";
        define_library<library_string_base, string_base> d_string_base;


        struct string_utility : public library {
            string_utility() : library(usage::explicit_include) {}

            object load(environment& a_environment) override {
                rebar::virtual_table& string_builder_virtual_table = a_environment.register_native_class("REBAR::STD::STRING_BUILDER");

                string_builder_virtual_table.overload_new = [](environment* env, native_object) -> rebar::object {
                    return env->create_native_object<std::vector<std::string>>("REBAR::STD::STRING_BUILDER");
                };

                string_builder_virtual_table.overload_addition_assignment = [](environment* env, native_object a_this, object rhs) {
                    a_this.get_object<std::vector<std::string>>().push_back(rhs.to_string());
                };

                string_builder_virtual_table[a_environment.str("toString")] = a_environment.bind([](environment* env) -> object {
                    object this_object = env->arg(0);

                    auto& strings = this_object.get_native_object().get_object<std::vector<std::string>>();

                    size_t size = 0;

                    for (const auto& str : strings) {
                        size += str.length();
                    }

                    std::string final;
                    final.reserve(size);

                    for (const auto& str : strings) {
                        final += str;
                    }

                    return env->str(final);
                });

                string_builder_virtual_table[a_environment.str("append")] = a_environment.bind([](environment* env) -> object {
                    object this_object = env->arg(0);
                    object obj = env->arg(1);

                    this_object.get_native_object().get_object<std::vector<std::string>>().push_back(obj.to_string());

                    return null;
                });

                native_object n_obj = a_environment.create_native_object<std::vector<std::string>>(string_builder_virtual_table);

                table* tbl = new table;

                (*tbl)[a_environment.str("StringBuilder")] = n_obj;

                return tbl;
            }
        };

        const char library_string_utility[] = "StringUtility";
        define_library<library_string_utility, string_utility> d_string_utility;
    }
}