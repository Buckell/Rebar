#include <iostream>

#include <rebar.hpp>

#include <emmintrin.h>

int main() {
    rebar::parser parser;

    std::vector<rebar::token> tokens = parser.lex("PrintLn(\"Hello, world!\");");

    parser.print_tokens(tokens);

    rebar::parse_unit unit = parser.parse("PrintLn(\"Hello, world!\");");

   std::cout << unit.string_representation() << std::endl;

    return 0;
}