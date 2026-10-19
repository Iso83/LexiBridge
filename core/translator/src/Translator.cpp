#include <LexiBridge/core/translator/Translator.h>
#include <translator/parser.h>
#include <translator/service.h>
#include <zlib.h>

namespace LexiBridge::Core::Translator {

std::mutex Translator::s_mutex;

std::string Translator::translate(const Request& request) {
    validate(request);
    const auto direction = request.from == Language::Dutch ? "nl-en" : "en-nl";
    const auto directory = request.dataDirectory / direction;
    for (const auto* name : {"config.yml", "model.bin", "vocab.spm", "lex.bin"}) {
        const auto path = directory / name;
        std::ifstream input(path, std::ios::binary | std::ios::ate);
        if (!input || input.tellg() < (std::string_view(name) == "model.bin" ? 16 : 1))
            throw std::runtime_error("Missing, unreadable or empty model asset: " + path.string());

        if (std::string_view(name) != "config.yml")
            verifyAsset(path);
    }

    // BlockingService keeps model failures on the calling thread.
    const std::lock_guard lock(s_mutex);
    const bool previousAbortPolicy = marian::getThrowExceptionOnAbort();
    marian::setThrowExceptionOnAbort(true);
    try {
        marian::bergamot::BlockingService service(marian::bergamot::BlockingService::Config{});
        const auto options = marian::bergamot::parseOptionsFromFilePath((directory / "config.yml").string());
        auto model = std::make_shared<marian::bergamot::TranslationModel>(options);
        auto responses = service.translateMultiple(model, {request.text}, {marian::bergamot::ResponseOptions{}});
        if (responses.size() != 1 || responses.front().target.text.empty())
            throw std::runtime_error("Bergamot returned no translation");

        marian::setThrowExceptionOnAbort(previousAbortPolicy);
        return std::move(responses.front().target.text);
    } catch (...) {
        marian::setThrowExceptionOnAbort(previousAbortPolicy);
        throw;
    }
}

void Translator::validate(const Request& request) {
    const auto supported = [](Language language) {
        return language == Language::Dutch || language == Language::English;
    };
    if (!supported(request.from) || !supported(request.to) || request.from == request.to)
        throw std::invalid_argument("Supported directions are nl -> en and en -> nl");

    if (request.text.empty())
        throw std::invalid_argument("Input text is empty");

    if (request.text.find('\0') != std::string::npos)
        throw std::invalid_argument("Input contains a NUL byte; expected UTF-8 text");

    if (request.dataDirectory.empty())
        throw std::invalid_argument("A local data directory is required");

    validateText(request.text);
}

void Translator::validateText(std::string_view text) {
    for (std::size_t offset = 0; offset < text.size();) {
        const auto first = static_cast<unsigned char>(text[offset]);
        if (first < 0x80) {
            ++offset;
            continue;
        }

        const std::size_t length = first >= 0xC2 && first <= 0xDF   ? 2
                                   : first >= 0xE0 && first <= 0xEF ? 3
                                   : first >= 0xF0 && first <= 0xF4 ? 4
                                                                    : 0;
        if (length == 0 || length > text.size() - offset)
            throw std::invalid_argument("Input is not valid UTF-8");

        for (std::size_t index = 1; index < length; ++index)
            if ((static_cast<unsigned char>(text[offset + index]) & 0xC0) != 0x80)
                throw std::invalid_argument("Input is not valid UTF-8");

        const auto second = static_cast<unsigned char>(text[offset + 1]);
        if ((first == 0xE0 && second < 0xA0) || (first == 0xED && second >= 0xA0) || (first == 0xF0 && second < 0x90) ||
            (first == 0xF4 && second >= 0x90))
            throw std::invalid_argument("Input is not valid UTF-8");

        offset += length;
    }
}

void Translator::verifyAsset(const std::filesystem::path& path) {
    auto checksumPath = path;
    checksumPath += ".crc32";
    std::ifstream checksumFile(checksumPath);
    unsigned long expected;
    if (!(checksumFile >> std::hex >> expected))
        throw std::runtime_error("Missing asset checksum, run prepare_translation_data: " + path.string());

    std::ifstream input(path, std::ios::binary);
    std::array<char, 65536> buffer;
    auto actual = crc32(0, Z_NULL, 0);
    while (input.read(buffer.data(), buffer.size()) || input.gcount() > 0)
        actual = crc32(actual, reinterpret_cast<const Bytef*>(buffer.data()), static_cast<uInt>(input.gcount()));

    if (input.bad() || actual != expected)
        throw std::runtime_error("Corrupt model asset, run prepare_translation_data: " + path.string());
}

} // namespace LexiBridge::Core::Translator
