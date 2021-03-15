#include <iostream>

#include <rebar.hpp>

int main() {
    Rebar::Optional<double> opt;

    opt = new double{ 3 };

    std::cout << *opt << std::endl;

    std::vector<char> data;

    return 0;
}
