#include <iostream>

#include <rebar.hpp>

int main() {
    Rebar::Token t(Rebar::Token::Type::StringLiteral, "Hello, world!");

    std::cout << static_cast<size_t>(t.type) << ' ' << t.GetStringLiteral() << '\n';
}