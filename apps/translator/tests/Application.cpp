#include "Application.h"

#include "TestAssert.h"

#include <filesystem>
#include <fstream>
#include <sstream>

using LexiBridge::Internal::Apps::Translator::Application;

class ApplicationTests {
private:
    std::ostringstream m_output;
    std::ostringstream m_errors;

public:
    int synonyms() {
        const std::string data = LEXIBRIDGE_TEST_DATA_DIR;
        CPPTEST_ASSERT(invoke({"synonyms", "--word", " \tBaNK\n", "--data-dir", data}) == 0);
        CPPTEST_ASSERT(m_output.str().find("[noun]") != std::string::npos);
        CPPTEST_ASSERT(m_output.str().find("[verb]") != std::string::npos);
        CPPTEST_ASSERT(m_output.str().find("financial institution") != std::string::npos);
        CPPTEST_ASSERT(m_output.str().find("banking company") != std::string::npos);
        CPPTEST_ASSERT(m_output.str().find("money box") != std::string::npos);
        CPPTEST_ASSERT(m_errors.str().empty());

        CPPTEST_ASSERT(invoke({"synonyms", "--word", "lexibridgezzzzunknown", "--data-dir", data}) == 0);
        CPPTEST_ASSERT(m_output.str() == "No WordNet results.\n");
        CPPTEST_ASSERT(m_errors.str().empty());

        CPPTEST_ASSERT(invoke({"synonyms", "--help"}) == 0);
        CPPTEST_ASSERT(m_output.str().find("--word") != std::string::npos);
        CPPTEST_ASSERT(m_output.str().find("--data-dir") != std::string::npos);

        CPPTEST_ASSERT(fails({"synonyms", "--data-dir", data}));
        CPPTEST_ASSERT(fails({"synonyms", "--word", "bank"}));
        CPPTEST_ASSERT(fails({"synonyms", "--word", "   ", "--data-dir", data}));
        CPPTEST_ASSERT(fails({"synonyms", "--word", "bank", "--data-dir", ""}));
        CPPTEST_ASSERT(fails({"synonyms", "--word", "bank", "--data-dir", LEXIBRIDGE_TEST_INPUT}));
        CPPTEST_ASSERT(fails({"synonyms", "--word", "lexibridgezzzzunknown", "--data-dir", LEXIBRIDGE_TEST_INPUT}));
        CPPTEST_ASSERT(fails({"synonyms", "--word", "bank", "--data-dir", data, "--from", "nl"}));
        CPPTEST_ASSERT(fails({"synonyms", "--word", "bank", "--data-dir", data, "translate"}));
        return 0;
    }

    int models() {
        const std::string data = LEXIBRIDGE_TEST_DATA_DIR;
        CPPTEST_ASSERT(
            invoke({"translate", "--from", "nl", "--to", "en", "--text", "De kat slaapt.", "--data-dir", data}) == 0);
        CPPTEST_ASSERT(m_output.str().find("cat") != std::string::npos);
        CPPTEST_ASSERT(m_output.str().find("sleep") != std::string::npos);
        CPPTEST_ASSERT(m_errors.str().empty());

        const auto input = std::filesystem::path(LEXIBRIDGE_TEST_INPUT);
        std::ofstream(input, std::ios::binary) << "\xEF\xBB\xBFThe cat is sleeping.\nWhere is the café?\n";
        CPPTEST_ASSERT(
            invoke({"translate", "--from", "en", "--to", "nl", "--input", input.string(), "--data-dir", data}) == 0);
        CPPTEST_ASSERT(m_output.str().find("kat") != std::string::npos);
        CPPTEST_ASSERT(m_output.str().find("slaap") != std::string::npos);
        CPPTEST_ASSERT(m_output.str().find("café") != std::string::npos);
        CPPTEST_ASSERT(m_output.str().find('\n') < m_output.str().find("café"));
        CPPTEST_ASSERT(m_errors.str().empty());
        std::filesystem::remove(input);
        return 0;
    }

    int errors() {
        const std::string data = LEXIBRIDGE_TEST_DATA_DIR;
        const auto input = std::filesystem::path(LEXIBRIDGE_TEST_INPUT);
        std::filesystem::remove(input);
        CPPTEST_ASSERT(fails({}));
        CPPTEST_ASSERT(fails({"translate", "--from", "de", "--to", "en", "--text", "Text", "--data-dir", data}));
        CPPTEST_ASSERT(fails({"translate", "--from", "nl", "--to", "nl", "--text", "tekst", "--data-dir", data}));
        CPPTEST_ASSERT(fails({"translate", "--from", "nl", "--to", "en", "--data-dir", data}));
        CPPTEST_ASSERT(fails({"translate", "--from", "nl", "--to", "en", "--text", "", "--data-dir", data}));
        CPPTEST_ASSERT(fails({"translate", "--from", "nl", "--to", "en", "--text", "tekst", "--input", input.string(),
                              "--data-dir", data}));
        CPPTEST_ASSERT(
            fails({"translate", "--from", "nl", "--to", "en", "--input", input.string(), "--data-dir", data}));
        CPPTEST_ASSERT(fails({"translate", "--from", "nl", "--to", "en", "--text", "tekst", "--data-dir", ""}));
        CPPTEST_ASSERT(
            fails({"translate", "--from", "nl", "--to", "en", "--text", "tekst", "--data-dir", input.string()}));
        return 0;
    }

private:
    int invoke(std::vector<std::string> arguments) {
        arguments.insert(arguments.begin(), "translator");
        std::vector<const char*> argv;
        for (const auto& argument : arguments)
            argv.push_back(argument.c_str());

        m_output.str("");
        m_errors.str("");
        return Application::run(static_cast<int>(argv.size()), argv.data(), m_output, m_errors);
    }

    bool fails(std::vector<std::string> arguments) {
        return invoke(std::move(arguments)) != 0 && m_output.str().empty() && !m_errors.str().empty();
    }
};

int main() {
    ApplicationTests tests;

    CPPTEST_RUN(tests.errors);
    CPPTEST_RUN(tests.synonyms);

    return tests.models();
}
