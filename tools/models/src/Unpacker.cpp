#include "Unpacker.h"

#include <array>
#include <fstream>
#include <ostream>
#include <zlib.h>

namespace LexiBridge::Internal::Models {

int Unpacker::run(const std::filesystem::path& inputPath, const std::filesystem::path& outputPath,
                  std::ostream& errors) {
#ifdef _WIN32
    gzFile input = gzopen_w(inputPath.c_str(), "rb");
#else
    gzFile input = gzopen(inputPath.c_str(), "rb");
#endif

    if (!input) {
        errors << "Cannot open gzip input\n";
        return 1;
    }

    if (gzdirect(input)) {
        errors << "Model asset is not a gzip stream\n";
        gzclose(input);
        return 1;
    }

    std::ofstream output(outputPath, std::ios::binary | std::ios::trunc);
    std::array<char, 65536> buffer;
    std::size_t total = 0;
    auto checksum = crc32(0, Z_NULL, 0);
    int readStatus = Z_OK;
    int count;
    while ((count = gzread(input, buffer.data(), static_cast<unsigned>(buffer.size()))) > 0) {
        // A partial payload can accompany a deferred gzip error. Another
        // gzread can clear that error, so inspect every read immediately.
        gzerror(input, &readStatus);
        if (readStatus != Z_OK)
            break;

        total += static_cast<std::size_t>(count);
        if (total > 512 * 1024 * 1024) {
            errors << "Uncompressed model asset exceeds 512 MiB\n";
            gzclose(input);
            return 1;
        }

        output.write(buffer.data(), count);
        checksum = crc32(checksum, reinterpret_cast<const Bytef*>(buffer.data()), static_cast<uInt>(count));
        if (!output)
            break;
    }

    gzerror(input, &readStatus);
    const int closeStatus = gzclose(input);
    output.close();
    if (count < 0 || readStatus != Z_OK || closeStatus != Z_OK || !output || total == 0) {
        errors << "Cannot decompress model asset (gzip integrity or output error)\n";
        return 1;
    }

    auto checksumPath = outputPath;
    checksumPath += ".crc32";
    std::ofstream checksumFile(checksumPath);
    checksumFile << std::hex << checksum << '\n';
    checksumFile.close();
    return checksumFile ? 0 : 1;
}

} // namespace LexiBridge::Internal::Models
