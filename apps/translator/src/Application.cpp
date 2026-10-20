#include "Application.h"

#include <CLI/CLI.hpp>
#include <LexiBridge/core/translator/Translator.h>
#include <LexiBridge/core/translator/synonyms/Dictionary.h>

namespace LexiBridge::Internal::Apps::Translator {

using Translator = LexiBridge::Core::Translator::Translator;
using Language = LexiBridge::Core::Translator::Language;

int Application::run(int argc, const char* const* argv, std::ostream& output, std::ostream& errors) {
    CLI::App cli{"Offline Dutch/English translation and English synonyms"};
    cli.require_subcommand(1, 1);
    auto* translate = cli.add_subcommand("translate", "Translate UTF-8 text using local CPU models");
    std::string from;
    std::string to;
    std::string text;
    std::string inputPath;
    std::string dataDirectory;
    translate->add_option("--from", from, "Source language: nl or en")->required()->check(CLI::IsMember({"nl", "en"}));
    translate->add_option("--to", to, "Target language: nl or en")->required()->check(CLI::IsMember({"nl", "en"}));
    auto* textOption = translate->add_option("--text", text, "UTF-8 text to translate");
    auto* inputOption = translate->add_option("--input", inputPath, "Read the entire UTF-8 file");
    textOption->excludes(inputOption);
    translate->add_option("--data-dir", dataDirectory, "Prepared local model directory")->required();

    auto* synonyms = cli.add_subcommand("synonyms", "Look up English meanings and synonyms in local WordNet data");
    std::string word;
    std::string dictionaryDirectory;
    synonyms->add_option("--word", word, "English lemma (case-insensitive); quote multiword expressions")->required();
    synonyms->add_option("--data-dir", dictionaryDirectory, "Prepared local data directory containing wordnet/")
        ->required();

    try {
        cli.parse(argc, argv);
        if (*synonyms) {
            const auto senses = LexiBridge::Core::Translator::Synonyms::Dictionary::lookup(
                word, std::filesystem::u8path(dictionaryDirectory));
            if (senses.empty()) {
                output << "No WordNet results.\n";
                return 0;
            }

            for (std::size_t index = 0; index < senses.size(); ++index) {
                const auto& sense = senses[index];
                output << index + 1 << ". [" << sense.partOfSpeech << "] " << sense.definition << '\n';
                output << "   Synonyms: ";
                if (sense.synonyms.empty())
                    output << "(no alternatives in this meaning)";

                for (std::size_t alternative = 0; alternative < sense.synonyms.size(); ++alternative) {
                    if (alternative != 0)
                        output << ", ";

                    output << sense.synonyms[alternative];
                }
                output << '\n';
            }

            return 0;
        }

        if (textOption->count() + inputOption->count() != 1)
            throw std::invalid_argument("Specify exactly one of --text and --input");

        if (from == to)
            throw std::invalid_argument("Source and target languages must differ");

        if (inputOption->count()) {
            std::ifstream input(std::filesystem::u8path(inputPath), std::ios::binary);
            if (!input)
                throw std::runtime_error("Cannot open input file: " + inputPath);

            text.assign(std::istreambuf_iterator<char>(input), {});
            if (input.bad())
                throw std::runtime_error("Cannot read input file: " + inputPath);
        }

        if (text.starts_with("\xEF\xBB\xBF"))
            text.erase(0, 3);

        const auto result = Translator::translate({from == "nl" ? Language::Dutch : Language::English,
                                                   to == "nl" ? Language::Dutch : Language::English, text,
                                                   std::filesystem::u8path(dataDirectory)});
        output << result;
        if (result.empty() || result.back() != '\n')
            output << '\n';

        return 0;
    } catch (const CLI::ParseError& error) {
        return cli.exit(error, output, errors);
    } catch (const std::exception& error) {
        errors << "translator error: " << error.what() << '\n';
        return 1;
    }
}

} // namespace LexiBridge::Internal::Apps::Translator
