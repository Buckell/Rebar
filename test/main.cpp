#include <iostream>
#include <filesystem>

#include "rebar/rebar.hpp"
#include "rebar/standard.hpp"

#include "nlohmann/json.hpp"

int main() {
    //rebar::environment env(rebar::use_provider<rebar::interpreter>);
//
    //std::string test = R"(
    //function TestFunc(p2, p3) {
//
    //}
//
    //if (var == 4) {
    //    PrintLn("Hello, world!" + 5, "Goodbye, world!");
    //} else if (var == 5)
    //    PrintLn(4);
    //else {
    //    PrintLn(4);
    //}
//
    //Print(2);
//
    //)";
//
    //rebar::lexer& code_lexer = env.code_lexer();
//
    //rebar::lex_unit l_uint = code_lexer.lex(test);
//
    //// code_parser.print_tokens(l_uint.tokens());
//
    ////rebar::parse_unit p_unit = code_parser.parse(test);
//
    //// std::cout << p_unit.string_representation() << std::endl;
//
    ////std::cout << rebar::object(env.create_string("PrintLn")).data() << " " << env.global_table().index(env.create_string("PrintLn")) << std::endl;
//
    //std::string input1 = R"(
//
    //    v = "Hello, world!";
//
    //    PrintLn(v);
//
    //    local dd = "Goodbye, world!";
//
    //    if (v == "Hi!") {
    //        PrintLn("Francais");
    //    } else if (v == "Hello, world!") {
    //        PrintLn("Espagnol", dd);
    //    } else {
    //        PrintLn("Anglais");
    //    }
//
    //    PrintLn(dd);
//
    //)";
//
    //std::string input2 = R"(
//
    //    PrintLn(v);
//
    //)";
//
    ////rebar::parse_unit p_unit = code_parser.parse(input1);
    ////std::cout << p_unit.string_representation() << std::endl;
//
    ////auto i1_func = env.compile_string(input1);
    ////i1_func();
//
    ////auto i2_func = env.compile_string(input2);
    ////i2_func();
//
    //rebar::library::load_implicit_libraries(env);
//
    ////std::cout << n_obj.get_virtual_table()[env.create_string("ToString")].call(env, n_obj) << std::endl;
//
    //std::string file_contents{ rebar::read_file("../test/test.rbr") };
//
    //try {
    //    rebar::parse_unit p_unit = rebar::parse(code_lexer, file_contents);
    //    std::cout << p_unit.string_representation() << std::endl;
//
    //    auto file_func = env.compile_string(file_contents, "TEST FILE MAIN");
    //    file_func();
    //} catch (rebar::exception& e) {
    //    std::cout << e.what() << std::endl;
    //}

    rebar::environment renv(rebar::use_provider<rebar::compiler>);

    rebar::library::load_implicit_libraries(renv);

    std::string file_contents{ rebar::read_file("../test/test.rbr") };

    rebar::lexer& code_lexer = renv.code_lexer();
    rebar::parse_unit p_unit = rebar::parse(code_lexer, file_contents);

    for (auto& tok : p_unit.m_lex_unit.tokens()) {
        std::cout << tok.to_string() << std::endl;
    }
    std::cout << p_unit.string_representation() << std::endl;

    auto f = renv.compile_string(file_contents);

    rebar::object o = f();

    std::cout << o << '\n' << std::endl;

    if (rebar::debug_mode) {
        ((rebar::compiler&)renv.execution_provider()).enable_assembly_debug_output(false);
    }

    std::filesystem::path test_case_directory("../test/cases");

    for (const auto& file : std::filesystem::directory_iterator(test_case_directory)) {
        std::ifstream file_stream(file.path());
        nlohmann::json file_json = nlohmann::json::parse(file_stream);

        std::string name = file_json["name"];

        const auto& code_data = file_json["code"];

        std::string code;

        if (code_data.is_array()) {
            code += code_data[0];

            for (size_t i = 1; i < code_data.size(); ++i) {
                code += '\n';
                code += code_data[i];
            }
        } else {
            code = code_data;
        }

        auto func = renv.compile_string(code, file.path().stem().string(), "TEST CASE");

        std::stringstream out_stream;
        renv.set_out_stream(out_stream);

        auto ret = func();

        const auto& return_target = file_json["return"];

        bool return_test = true;

        if (return_target.is_string()) {
            return_test = ret.is_string() && return_target.get<std::string_view>() == ret.get_string().to_string_view();
        } else if (return_target.is_number_integer()) {
            return_test = ret.is_integer() && return_target.get<rebar::integer>() == ret.get_integer();
        } else if (return_target.is_number_float()) {
            return_test = ret.is_number() && return_target.get<rebar::number>() == ret.get_number();
        } else if (return_target.is_boolean()) {
            return_test = ret.is_boolean() && return_target.get<bool>() == ret.get_boolean();
        }

        bool output_test = true;

        const auto& output_data = file_json["output"];

        std::string output;
        std::string actual_output;

        if (!output_data.is_null()) {
            if (output_data.is_array()) {
                for (size_t i = 0; i < output_data.size(); ++i) {
                    output += output_data[i];
                    output += "\n";
                }
            } else {
                output = output_data;
            }

            output.erase(std::remove_if(output.begin(), output.end(), [] (char c) noexcept -> bool {
                return c == '\r';
            }), output.cend());

            actual_output = out_stream.str();
            actual_output.erase(std::remove_if(actual_output.begin(), actual_output.end(), [] (char c) noexcept -> bool {
                return c == '\r';
            }), actual_output.cend());

            if (actual_output.size() == output.size()) {
                for (size_t i = 0; i < output.size(); ++i) {
                    if (actual_output[i] != output[i]) {
                        output_test = false;
                        break;
                    }
                }
            } else {
                output_test = false;
            }
        }

        std::cout << "[TEST] " << (return_test && output_test ? "PASS" : "FAIL") << " | " << name << " | RESULT-CHECK: " << (return_test ? "PASS" : "FAIL") << ", OUTPUT-CHECK: " << (output_test ? "PASS" : "FAIL") << std::endl;

        if (!output_test) {
            std::cout << "--- Expected ---\n";
            std::cout << output;
            std::cout << "----------------" << std::endl;
            std::cout << "--- Actual ---\n";
            std::cout << actual_output;
            std::cout << "----------------" << std::endl;
        }

        if (!return_test) {
            std::cout << "RETURNED " << ret << " (EXPECTED " << return_target << ")" << std::endl;
        }
    }

    return 0;
}