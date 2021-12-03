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
                std::cout << env->arg(0);

                for (size_t i = 1; i < env->arg_count(); i++) {
                    std::cout << "    " << env->arg(i);
                }

                std::cout << std::endl; // '/n' or stream flush (std::endl)

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

            static object Length(environment* a_environment) {
                return static_cast<integer>(a_environment->arg(0).get_string().length());
            }

            object load(environment& a_environment) override {
                auto& string_table = a_environment.get_string_virtual_table();

                const auto define_string_function = [&a_environment, &string_table](const std::string_view a_identifier, callable a_function) noexcept {
                    string_table[a_environment.str(a_identifier)] = a_environment.bind(a_function);
                };

                define_string_function("length", Length);

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

                string_builder_virtual_table.overload_addition_assignment = [](environment* env, native_object a_this, const object rhs) {
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