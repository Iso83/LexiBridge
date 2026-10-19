#include "Application.h"

#include <CLI/CLI.hpp>
#include <cstdio>
#include <exception>
#include <iostream>

#ifdef _WIN32

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <fcntl.h>
#include <io.h>
#include <windows.h>

#endif

int main(int argc, char** argv) {
#ifdef _WIN32
    // Keep UTF-8 bytes and existing CRLFs intact, including redirected output.
    _setmode(_fileno(stdout), _O_BINARY);
    const auto previousCodePage = GetConsoleOutputCP();
    if (previousCodePage != 0)
        SetConsoleOutputCP(CP_UTF8);
#endif

    int result = 1;
    try {
        CLI::App encoding;
        result = LexiBridge::Internal::Apps::Translator::Application::run(argc, encoding.ensure_utf8(argv), std::cout,
                                                                          std::cerr);
    } catch (const std::exception& error) {
        std::cerr << "translator error: " << error.what() << '\n';
    }

#ifdef _WIN32
    std::cout.flush();
    std::cerr.flush();
    if (previousCodePage != 0)
        SetConsoleOutputCP(previousCodePage);
#endif
    return result;
}
