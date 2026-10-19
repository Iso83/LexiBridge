#pragma once

#include <ostream>

namespace LexiBridge::Internal::Apps::Translator {

class Application {
public:
    static int run(int argc, const char* const* argv, std::ostream& output, std::ostream& errors);
};

} // namespace LexiBridge::Internal::Apps::Translator
