#pragma once

#include <string>
#include <vector>

namespace LexiBridge::Core::Translator::Synonyms {

struct Sense {
    std::string id;
    std::string partOfSpeech;
    std::string definition;
    std::vector<std::string> synonyms;
};

} // namespace LexiBridge::Core::Translator::Synonyms
