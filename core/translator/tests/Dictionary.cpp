#include "TestAssert.h"

#include <LexiBridge/core/translator/synonyms/Dictionary.h>
#include <fstream>
#include <unordered_set>

using LexiBridge::Core::Translator::Synonyms::Dictionary;
using LexiBridge::Core::Translator::Synonyms::Sense;

namespace {
static bool contains(const std::vector<Sense>& senses, const std::string& type, const std::string& definition,
                     const std::string& alternative) {
    return std::any_of(senses.begin(), senses.end(), [&](const Sense& sense) {
        return sense.partOfSpeech == type && sense.definition.find(definition) != std::string::npos &&
               std::find(sense.synonyms.begin(), sense.synonyms.end(), alternative) != sense.synonyms.end();
    });
}

static bool dataError(const std::filesystem::path& directory) {
    for (const auto* word : {"bank", "lexibridgezzzzunknown"}) {
        try {
            Dictionary::lookup(word, directory);
            return false;
        } catch (const std::runtime_error& error) {
            if (std::string(error.what()).find("WordNet data") == std::string::npos)
                return false;
        }
    }
    return true;
}
} // namespace

static int meanings() {
    const auto data = std::filesystem::u8path(LEXIBRIDGE_TEST_DATA_DIR);
    const auto car = Dictionary::lookup("car", data);
    CPPTEST_ASSERT(contains(car, "noun", "motor vehicle", "automobile"));

    const auto bank = Dictionary::lookup("bank", data);
    CPPTEST_ASSERT(contains(bank, "noun", "financial institution", "banking company"));
    CPPTEST_ASSERT(contains(bank, "noun", "container", "money box"));
    CPPTEST_ASSERT(
        std::any_of(bank.begin(), bank.end(), [](const Sense& sense) { return sense.partOfSpeech == "verb"; }));
    CPPTEST_ASSERT(contains(Dictionary::lookup("quick", data), "adjective", "moving quickly", "nimble"));
    CPPTEST_ASSERT(contains(Dictionary::lookup("quickly", data), "adverb", "with speed", "rapidly"));

    // Source synsets may contain only the queried word. Preserve that meaning.
    const auto able = Dictionary::lookup("able", data);
    CPPTEST_ASSERT(std::any_of(able.begin(), able.end(), [](const Sense& sense) {
        return sense.definition.find("necessary means") != std::string::npos && sense.synonyms.empty();
    }));
    return 0;
}

static int normalization() {
    const auto data = std::filesystem::u8path(LEXIBRIDGE_TEST_DATA_DIR);
    const auto expected = Dictionary::lookup("bank", data);
    const auto normalized = Dictionary::lookup(" \tBaNK\r\n", data);
    CPPTEST_ASSERT(normalized.size() == expected.size());
    for (std::size_t index = 0; index < expected.size(); ++index) {
        CPPTEST_ASSERT(normalized[index].id == expected[index].id);
        CPPTEST_ASSERT(normalized[index].definition == expected[index].definition);
        CPPTEST_ASSERT(normalized[index].partOfSpeech == expected[index].partOfSpeech);
        CPPTEST_ASSERT(normalized[index].synonyms == expected[index].synonyms);
        std::unordered_set<std::string> seen;
        for (const auto& synonym : normalized[index].synonyms) {
            CPPTEST_ASSERT(synonym != "bank");
            CPPTEST_ASSERT(seen.insert(synonym).second);
        }
    }

    const auto multiword = Dictionary::lookup(" ICE CREAM ", data);
    CPPTEST_ASSERT(contains(multiword, "noun", "frozen dessert", "icecream"));
    const auto underscored = Dictionary::lookup("ice_cream", data);
    CPPTEST_ASSERT(multiword.size() == underscored.size());
    CPPTEST_ASSERT(multiword.front().id == underscored.front().id);

    // The real source has A.M./a.m. and am/AM in this same synset.
    const auto morning = Dictionary::lookup("ante meridiem", data);
    CPPTEST_ASSERT(morning.size() == 1);
    CPPTEST_ASSERT(morning.front().partOfSpeech == "adverb");
    CPPTEST_ASSERT(morning.front().synonyms == std::vector<std::string>({"A.M.", "am"}));

    // Position markers such as (p) must not hide an adjective lemma.
    CPPTEST_ASSERT(contains(Dictionary::lookup("ready to hand", data), "adjective", "easy to reach", "handy"));
    CPPTEST_ASSERT(Dictionary::lookup("lexibridgezzzzunknown", data).empty());
    return 0;
}

static int errors() {
    const auto data = std::filesystem::u8path(LEXIBRIDGE_TEST_DATA_DIR);
    const auto rejects = [](const std::string& word, const std::filesystem::path& directory) {
        try {
            Dictionary::lookup(word, directory);
        } catch (const std::invalid_argument&) {
            return true;
        }
        return false;
    };
    CPPTEST_ASSERT(rejects("", data));
    CPPTEST_ASSERT(rejects(" \r\n\t", data));
    CPPTEST_ASSERT(rejects(std::string("a\0b", 3), data));
    CPPTEST_ASSERT(rejects("bank", {}));

    const auto scratch = std::filesystem::u8path(LEXIBRIDGE_WORDNET_TEST_SCRATCH);
    std::filesystem::remove_all(scratch);
    CPPTEST_ASSERT(dataError(scratch));
    std::filesystem::create_directories(scratch / "wordnet");
    for (const auto* name : {"data.noun", "data.verb", "data.adj", "data.adv"})
        std::filesystem::copy_file(data / "wordnet" / name, scratch / "wordnet" / name);

    // An absent/empty/truncated final file must fail, even if the first file
    // contains the requested lemma, and also for an unknown word.
    const auto damaged = scratch / "wordnet" / "data.adv";
    std::filesystem::remove(damaged);
    CPPTEST_ASSERT(dataError(scratch));
    std::ofstream(damaged, std::ios::binary);
    CPPTEST_ASSERT(dataError(scratch));
    std::ofstream(damaged, std::ios::binary) << "not WordNet\n";
    CPPTEST_ASSERT(dataError(scratch));
    std::filesystem::copy_file(data / "wordnet" / "data.adv", damaged,
                               std::filesystem::copy_options::overwrite_existing);
    std::filesystem::resize_file(damaged, std::filesystem::file_size(damaged) - 1);
    CPPTEST_ASSERT(dataError(scratch));
    std::filesystem::copy_file(data / "wordnet" / "data.adv", damaged,
                               std::filesystem::copy_options::overwrite_existing);
    {
        std::fstream output(damaged, std::ios::binary | std::ios::in | std::ios::out);
        output.put('!');
    }
    CPPTEST_ASSERT(dataError(scratch));
    std::filesystem::remove_all(scratch);
    return 0;
}

int main() {
    try {
        CPPTEST_RUN(meanings);
        CPPTEST_RUN(normalization);
        CPPTEST_RUN(errors);
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
