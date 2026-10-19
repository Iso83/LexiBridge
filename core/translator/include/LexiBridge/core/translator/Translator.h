#pragma once

#include <LexiBridge/core/translator/Request.h>
#include <mutex>
#include <string_view>

namespace LexiBridge::Core::Translator {

class Translator {
private:
    static std::mutex s_mutex;

public:
    // UTF-8 in/out; preserves paragraph breaks. Throws std::exception on errors.
    static std::string translate(const Request& request);

private:
    static void validate(const Request& request);
    static void validateText(std::string_view text);
    static void verifyAsset(const std::filesystem::path& path);
};

} // namespace LexiBridge::Core::Translator
