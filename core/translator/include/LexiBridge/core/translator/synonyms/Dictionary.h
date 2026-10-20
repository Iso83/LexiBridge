#pragma once

#include <LexiBridge/core/translator/synonyms/Sense.h>
#include <filesystem>
#include <string_view>

namespace LexiBridge::Core::Translator::Synonyms {

class Dictionary {
public:
    static std::vector<Sense> lookup(std::string_view word, const std::filesystem::path& dataDirectory);

private:
    static std::string normalize(std::string_view word);
    static std::string display(std::string word);
    static std::string read(const std::filesystem::path& path, unsigned long expectedChecksum);
    static void appendSenses(std::string_view data, const std::string& word, std::vector<Sense>& result);
};

} // namespace LexiBridge::Core::Translator::Synonyms
