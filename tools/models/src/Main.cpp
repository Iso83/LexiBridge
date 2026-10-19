#include "Unpacker.h"

#include <filesystem>
#include <iostream>

#ifdef _WIN32
int wmain(int argc, wchar_t** argv) {
#else
int main(int argc, char** argv) {
#endif

    if (argc != 3) {
        std::cerr << "Usage: model_unpack input.gz output\n";
        return 1;
    }

    return LexiBridge::Internal::Models::Unpacker::run(std::filesystem::path(argv[1]), std::filesystem::path(argv[2]),
                                                       std::cerr);
}
