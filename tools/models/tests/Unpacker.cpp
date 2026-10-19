#include "Unpacker.h"

#include "TestAssert.h"

#include <fstream>
#include <zlib.h>

using LexiBridge::Internal::Models::Unpacker;

namespace {

static void writeGzip(const std::filesystem::path& path, const std::string& bytes) {
#ifdef _WIN32
    gzFile output = gzopen_w(path.c_str(), "wb");
#else
    gzFile output = gzopen(path.c_str(), "wb");
#endif
    if (!output)
        throw std::runtime_error("Cannot create gzip fixture");

    const auto count = gzwrite(output, bytes.data(), static_cast<unsigned>(bytes.size()));
    const auto status = gzclose(output);
    if (count != static_cast<int>(bytes.size()) || status != Z_OK)
        throw std::runtime_error("Cannot write gzip fixture");
}
} // namespace

static int run() {
    const auto directory = std::filesystem::u8path(LEXIBRIDGE_UNPACK_TEST_DIR);
    std::filesystem::remove_all(directory);
    std::filesystem::create_directories(directory);
    const auto compressed = directory / "fixture.gz";
    const auto output = directory / "output.bin";
    std::string fixture(131073, '\0');
    fixture.front() = 'A';
    fixture[65535] = 'B';
    fixture[65536] = 'C';
    fixture.back() = 'D';
    writeGzip(compressed, fixture);

    std::ostringstream errors;
    CPPTEST_ASSERT(Unpacker::run(compressed, output, errors) == 0);
    std::ifstream input(output, std::ios::binary);
    const std::string decoded(std::istreambuf_iterator<char>(input), {});
    CPPTEST_ASSERT(decoded == fixture);
    CPPTEST_ASSERT(errors.str().empty());
    input.close();

    // Standard CRC-32 check vector, independent of the decoder's loop.
    writeGzip(compressed, "123456789");
    CPPTEST_ASSERT(Unpacker::run(compressed, output, errors) == 0);
    std::ifstream checksumFile(directory / "output.bin.crc32");
    unsigned long checksum = 0;
    checksumFile >> std::hex >> checksum;
    CPPTEST_ASSERT(checksum == 0xCBF43926);
    checksumFile.close();

    std::filesystem::resize_file(compressed, std::filesystem::file_size(compressed) - 4);
    CPPTEST_ASSERT(Unpacker::run(compressed, directory / "invalid.bin", errors) != 0);
    CPPTEST_ASSERT(errors.str().find("gzip integrity") != std::string::npos);

    // gzread otherwise accepts plain text through its transparent mode.
    std::ofstream plain(compressed, std::ios::binary | std::ios::trunc);
    plain << "<Error>Missing model</Error>";
    plain.close();
    CPPTEST_ASSERT(Unpacker::run(compressed, directory / "plain.bin", errors) != 0);
    CPPTEST_ASSERT(errors.str().find("not a gzip stream") != std::string::npos);
    CPPTEST_ASSERT(!std::filesystem::exists(directory / "plain.bin"));
    std::filesystem::remove_all(directory);
    return 0;
}

int main() {
    try {
        return run();
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
