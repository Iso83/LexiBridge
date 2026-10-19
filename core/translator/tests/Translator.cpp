#include "TestAssert.h"

#include <LexiBridge/core/translator/Translator.h>
#include <fstream>

using namespace LexiBridge::Core::Translator;

class TranslationTests {
public:
    static int models() {
        const auto data = std::filesystem::path(LEXIBRIDGE_TEST_DATA_DIR);
        const auto english =
            Translator::translate({Language::Dutch, Language::English, "De kat slaapt.\nWaar is het café?", data});
        CPPTEST_ASSERT(english.find("cat") != std::string::npos);
        CPPTEST_ASSERT(english.find("sleep") != std::string::npos);
        CPPTEST_ASSERT(english.find('\n') != std::string::npos);
        CPPTEST_ASSERT(english.find("caf") != std::string::npos);

        const auto dutch = Translator::translate(
            {Language::English, Language::Dutch, "The cat is sleeping.\nWhere is the café?", data});
        CPPTEST_ASSERT(dutch.find("kat") != std::string::npos);
        CPPTEST_ASSERT(dutch.find("slaap") != std::string::npos);
        CPPTEST_ASSERT(dutch.find('\n') != std::string::npos);
        CPPTEST_ASSERT(dutch.find("café") != std::string::npos);
        return 0;
    }

    static int errors() {
        const auto rejects = [](const Request& request) {
            try {
                Translator::translate(request);
            } catch (const std::exception&) {
                return true;
            }
            return false;
        };
        CPPTEST_ASSERT(rejects({Language::Dutch, Language::Dutch, "tekst", "."}));
        CPPTEST_ASSERT(rejects({static_cast<Language>(99), Language::English, "tekst", "."}));
        CPPTEST_ASSERT(rejects({Language::Dutch, Language::English, "", "."}));
        CPPTEST_ASSERT(rejects({Language::Dutch, Language::English, std::string("a\0b", 3), "."}));
        CPPTEST_ASSERT(rejects({Language::Dutch, Language::English, "\xC3", "."}));
        CPPTEST_ASSERT(rejects({Language::Dutch, Language::English, "\xED\xA0\x80", "."}));
        CPPTEST_ASSERT(rejects({Language::Dutch, Language::English, "tekst", ""}));

        const auto directory = std::filesystem::path(LEXIBRIDGE_TEST_SCRATCH_DIR);
        std::filesystem::remove_all(directory);
        std::filesystem::create_directories(directory / "nl-en");
        CPPTEST_ASSERT(rejects({Language::Dutch, Language::English, "tekst", directory}));
        for (const auto* name : {"config.yml", "model.bin", "vocab.spm", "lex.bin"})
            std::ofstream(directory / "nl-en" / name) << "broken";

        CPPTEST_ASSERT(rejects({Language::Dutch, Language::English, "tekst", directory}));
        std::ofstream(directory / "nl-en" / "model.bin") << "deliberately corrupt model payload";
        std::ofstream(directory / "nl-en" / "model.bin.crc32") << "0\n";
        bool corruptReported = false;
        try {
            Translator::translate({Language::Dutch, Language::English, "tekst", directory});
        } catch (const std::runtime_error& error) {
            corruptReported = std::string(error.what()).find("Corrupt model asset") != std::string::npos;
        }
        CPPTEST_ASSERT(corruptReported);
        std::filesystem::remove_all(directory);
        return 0;
    }
};

int main() {
    try {
        CPPTEST_RUN(TranslationTests::errors);
        CPPTEST_RUN(TranslationTests::models);
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
