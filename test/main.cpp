#include <iostream>
#include <filesystem>

#include "rebar/rebar.hpp"
#include "rebar/standard.hpp"

#include "nlohmann/json.hpp"

std::pair<size_t, size_t> count_file_lines(const std::filesystem::path& a_path, bool a_recursive = true) {
    size_t total_lines = 0;
    size_t content_lines = 0;

    for (auto& item : std::filesystem::directory_iterator(a_path)) {
        if (a_recursive && item.is_directory()) {
            auto counts = count_file_lines(item.path());

            total_lines += counts.first;
            content_lines += counts.second;
        } else {
            std::ifstream file_stream(item.path());

            for (std::string str; std::getline(file_stream, str);) {
                ++total_lines;
                content_lines += str.size() > 0;
            }
        }
    }

    return { total_lines, content_lines };
}

void run_test_cases(rebar::environment& env) {
    std::filesystem::path test_case_directory("../test/cases");

    size_t max_name_size = 0;

    for (const auto& file : std::filesystem::directory_iterator(test_case_directory)) {
        std::ifstream file_stream(file.path() / "case.json");
        nlohmann::json file_json = nlohmann::json::parse(file_stream);
        file_stream.close();

        max_name_size = std::max(max_name_size, file_json["name"].get<std::string_view>().size());
    }

    for (const auto& file : std::filesystem::directory_iterator(test_case_directory)) {
        std::ifstream file_stream(file.path() / "case.json");
        nlohmann::json file_json = nlohmann::json::parse(file_stream);
        file_stream.close();

        std::string name = file_json["name"];

        const auto& code_data = file_json["code"];

        auto func = env.compile_file(file.path() / "main.bar", file.path().stem().string());

        std::stringstream out_stream;
        env.set_out_stream(out_stream);
        env.set_error_stream(out_stream);

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
            }), output.end());

            actual_output = out_stream.str();
            actual_output.erase(std::remove_if(actual_output.begin(), actual_output.end(), [] (char c) noexcept -> bool {
                return c == '\r';
            }), actual_output.end());

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

        std::cout << "[TEST] " << (return_test && output_test ? "PASS" : "FAIL") << " | " << name << std::string(max_name_size - name.size(), ' ') << " | RESULT-CHECK: " << (return_test ? "PASS" : "FAIL") << ", OUTPUT-CHECK: " << (output_test ? "PASS" : "FAIL") << std::endl;

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
}

void run_test_file(rebar::environment& env) {
    std::string file_contents{ rebar::read_file("../test/test.bar") };

    rebar::parse_unit p_unit = rebar::parse(env.code_lexer(), file_contents);
    std::cout << p_unit.string_representation() << std::endl;

    auto test_file = env.compile_file("../test/test.bar", "main");

    try {
        std::cout << test_file() << '\n' << std::endl;
    } catch (rebar::runtime_exception& e) {
        std::cout << e.what() << std::endl;
    }
}

void run_test_compile() {
    asmjit::JitRuntime runtime;
    asmjit::CodeHolder holder;
    holder.init(runtime.environment());
    asmjit::FileLogger logger(stdout);

    asmjit::x86::Compiler cc(&holder);
    cc.addDiagnosticOptions(asmjit::DiagnosticOptions::kRADebugAll);
    cc.setLogger(&logger);

    auto* func_node = cc.addFunc(asmjit::FuncSignatureT<void, rebar::object*, rebar::environment*>(rebar::compiler::platform_call_convention));

    auto ret = cc.newGpq("ret");
    auto env = cc.newGpq("env");

    func_node->setArg(0, ret);
    func_node->setArg(1, env);

    cc.mov(asmjit::x86::qword_ptr(ret), rebar::type::null);
    cc.mov(asmjit::x86::qword_ptr(ret), rebar::type::integer);
    cc.mov(asmjit::x86::qword_ptr(ret), rebar::type::number);

    //cc._code = nullptr;
    cc.mov(asmjit::x86::qword_ptr(ret), rebar::type::null);
    cc.commentf("Test");
    //cc._code = &holder;

    cc.mov(asmjit::x86::qword_ptr(ret, rebar::object_data_offset), 0);

    cc.ret();

    cc.endFunc();
    cc.finalize();

    rebar::callable func;
    asmjit::Error err = runtime.add(&func, &holder);

    rebar::object ob;
    //func(&ob, &renv);

    std::cout << ob << std::endl;
}

REBAR_FUNCTION(TestFunction) {
    auto& comp = env->execution_provider<rebar::compiler>();

    REBAR_RETURN(rebar::null);
}

int main() {
    std::filesystem::path include_directory("../include/rebar");
    auto sloc = count_file_lines(include_directory);
    std::cout << "SLOC - TOTAL LINES: " << sloc.first << " - NON-EMPTY LINES: " << sloc.second << std::endl;

    rebar::environment compiler_environment(rebar::use_provider<rebar::compiler>);
    rebar::library::load_implicit_libraries(compiler_environment);

    compiler_environment.global_table()[compiler_environment.str("TestFunction")] = compiler_environment.bind(TestFunction);

    rebar::environment interpreter_environment(rebar::use_provider<rebar::interpreter>);
    rebar::library::load_implicit_libraries(interpreter_environment);

    std::cout << "========== [ MAIN TEST ] ==========" << std::endl;
    run_test_file(compiler_environment);

    compiler_environment.execution_provider<rebar::compiler>().enable_assembly_debug_output(false);
    std::cout << "========== [ COMPILER ] ==========" << std::endl;
    run_test_cases(compiler_environment);
    std::cout << std::endl;

    std::cout << "========== [ INTERPRETER ] ==========" << std::endl;
    run_test_cases(interpreter_environment);
    std::cout << std::endl;

    return 0;
}