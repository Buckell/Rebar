#include <iostream>

#include "rebar/rebar.hpp"
#include "rebar/standard.hpp"

#include <emmintrin.h>

int main() {
    rebar::environment env(rebar::use_provider<rebar::interpreter>);

    std::string test = R"(
    function TestFunc(p2, p3) {

    }

    if (var == 4) {
        PrintLn("Hello, world!" + 5, "Goodbye, world!");
    } else if (var == 5)
        PrintLn(4);
    else {
        PrintLn(4);
    }

    Print(2);

    )";

    rebar::lexer& code_lexer = env.code_lexer();

    rebar::lex_unit l_uint = code_lexer.lex(test);

    // code_parser.print_tokens(l_uint.tokens());

    //rebar::parse_unit p_unit = code_parser.parse(test);

    // std::cout << p_unit.string_representation() << std::endl;

    //std::cout << rebar::object(env.create_string("PrintLn")).data() << " " << env.global_table().index(env.create_string("PrintLn")) << std::endl;

    std::string input1 = R"(

        v = "Hello, world!";

        PrintLn(v);

        local dd = "Goodbye, world!";

        if (v == "Hi!") {
            PrintLn("Francais");
        } else if (v == "Hello, world!") {
            PrintLn("Espagnol", dd);
        } else {
            PrintLn("Anglais");
        }

        PrintLn(dd);

    )";

    std::string input2 = R"(

        PrintLn(v);

    )";

    //rebar::parse_unit p_unit = code_parser.parse(input1);
    //std::cout << p_unit.string_representation() << std::endl;

    //auto i1_func = env.compile_string(input1);
    //i1_func();

    //auto i2_func = env.compile_string(input2);
    //i2_func();

    rebar::library::load_implicit_libraries(env);

    //std::cout << n_obj.get_virtual_table()[env.create_string("ToString")].call(env, n_obj) << std::endl;

    std::string file_contents{ rebar::read_file("../test/test.rbr") };

    try {
        rebar::parse_unit p_unit = rebar::parse(code_lexer, file_contents);
        std::cout << p_unit.string_representation() << std::endl;

        auto file_func = env.compile_string(file_contents, "TEST FILE MAIN");
        file_func();
    } catch (rebar::exception& e) {
        std::cout << e.what() << std::endl;
    }

    return 0;
}