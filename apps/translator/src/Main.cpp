#include "Application.h"

#include <CLI/CLI.hpp>
#include <cstdlib>
#include <exception>
#include <iostream>

int main(int argc, char** argv) {

    LexiBridge::Apps::Translator::Options options;


    CLI11_PARSE(cli, argc, argv);
    try {
        return LexiBridge::Apps::Translator::Application::run(options);
    } catch (const std::exception& error) {
        std::cerr << "translator error: " << error.what() << '\n';
        return EXIT_FAILURE;
    }
}
