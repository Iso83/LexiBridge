#include "Application.h"

#include <CLI/CLI.hpp>
#include <LexiBridge/core/translator/Translator.h>

namespace LexiBridge::Internal::Apps::Translator {

using Translator = LexiBridge::Core::Translator::Translator;
using Language = LexiBridge::Core::Translator::Language;

int Application::run(int argc, const char* const* argv, std::ostream& output, std::ostream& errors) {
    CLI::App cli{"Offline Dutch/English translation with local Bergamot models"};
    cli.require_subcommand(1);
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

    try {
        cli.parse(argc, argv);
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
