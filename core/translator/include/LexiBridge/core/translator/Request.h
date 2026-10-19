#pragma once

#include <filesystem>
#include <string>

namespace LexiBridge::Core::Translator {

enum class Language { Dutch, English };

struct Request {
    Language from;
    Language to;
    std::string text;
    std::filesystem::path dataDirectory;
};

} // namespace LexiBridge::Core::Translator
