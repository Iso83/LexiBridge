#pragma once

#include <filesystem>

namespace LexiBridge::Internal::Models {

class Unpacker {
public:
    static int run(const std::filesystem::path& inputPath, const std::filesystem::path& outputPath,
                   std::ostream& errors);
};

} // namespace LexiBridge::Internal::Models
