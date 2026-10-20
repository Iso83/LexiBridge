#include <LexiBridge/core/translator/synonyms/Dictionary.h>
#include <array>
#include <fstream>
#include <unordered_set>
#include <zlib.h>

namespace LexiBridge::Core::Translator::Synonyms {

std::vector<Sense> Dictionary::lookup(std::string_view word, const std::filesystem::path& dataDirectory) {
    const auto normalized = normalize(word);
    if (normalized.empty())
        throw std::invalid_argument("A non-empty English word is required");

    if (dataDirectory.empty())
        throw std::invalid_argument("A local data directory is required");

    // CRCs pin the unmodified WNDB files from the verified OEWN 2025 archive.
    // Validate every part of speech, even for unknown words or early matches.
    const std::array<std::pair<const char*, unsigned long>, 4> assets{{{"data.noun", 0x2891a8beUL},
                                                                       {"data.verb", 0x1da768d6UL},
                                                                       {"data.adj", 0x4689545eUL},
                                                                       {"data.adv", 0x093911a4UL}}};
    std::vector<Sense> result;
    for (const auto& [name, checksum] : assets)
        appendSenses(read(dataDirectory / "wordnet" / name, checksum), normalized, result);

    return result;
}

std::string Dictionary::normalize(std::string_view word) {
    const auto first = word.find_first_not_of(" \t\r\n\f\v");
    if (first == std::string_view::npos)
        return {};

    const auto last = word.find_last_not_of(" \t\r\n\f\v");
    std::string result(word.substr(first, last - first + 1));
    for (char& character : result) {
        if (static_cast<unsigned char>(character) < 0x20 || character == 0x7f)
            throw std::invalid_argument("English word contains a control character");

        if (character >= 'A' && character <= 'Z')
            character = static_cast<char>(character + ('a' - 'A'));
        else if (character == ' ')
            character = '_';
    }

    return result;
}

std::string Dictionary::display(std::string word) {
    // Adjective position markers are WNDB metadata, not part of the lemma.
    for (const auto* marker : {"(a)", "(p)", "(ip)"})
        if (word.ends_with(marker)) {
            word.resize(word.size() - std::string_view(marker).size());
            break;
        }

    std::replace(word.begin(), word.end(), '_', ' ');
    return word;
}

std::string Dictionary::read(const std::filesystem::path& path, unsigned long expectedChecksum) {
    std::ifstream input(path, std::ios::binary | std::ios::ate);
    if (!input || input.tellg() <= 0 || input.tellg() > 32 * 1024 * 1024)
        throw std::runtime_error("Missing, unreadable or unusable WordNet data: " + path.string() +
                                 "; build LexiBridge_tools_wordnet_prepare");

    const auto size = static_cast<std::size_t>(input.tellg());
    input.seekg(0);
    std::string data(size, '\0');
    input.read(data.data(), static_cast<std::streamsize>(size));
    const auto checksum =
        crc32(crc32(0, Z_NULL, 0), reinterpret_cast<const Bytef*>(data.data()), static_cast<uInt>(data.size()));
    if (!input || checksum != expectedChecksum)
        throw std::runtime_error("Corrupt or unsupported WordNet data: " + path.string() +
                                 "; build LexiBridge_tools_wordnet_prepare");

    return data;
}

void Dictionary::appendSenses(std::string_view data, const std::string& word, std::vector<Sense>& result) {
    // Scan synsets directly; index offsets are unnecessary and file encoding/newline
    // differences cannot redirect a lookup into a different sense.
    while (!data.empty()) {
        const auto end = data.find('\n');
        const auto line = data.substr(0, end);
        data = end == std::string_view::npos ? std::string_view{} : data.substr(end + 1);
        if (line.empty() || line.front() == ' ')
            continue;

        std::istringstream fields{std::string(line)};
        std::string offset;
        unsigned lexicographerFile;
        char type;
        unsigned count;
        if (!(fields >> offset >> lexicographerFile >> type >> std::hex >> count) || count == 0)
            throw std::runtime_error("Invalid WordNet synset");

        bool matches = false;
        std::vector<std::string> alternatives;
        std::unordered_set<std::string> seen;
        for (unsigned index = 0; index < count; ++index) {
            std::string lemma;
            unsigned lexicalId;
            if (!(fields >> lemma >> lexicalId))
                throw std::runtime_error("Invalid WordNet lemma");

            auto label = display(std::move(lemma));
            auto key = normalize(label);
            if (key == word)
                matches = true;
            else if (seen.insert(std::move(key)).second)
                alternatives.push_back(std::move(label));
        }

        if (!matches)
            continue;

        std::string partOfSpeech;
        switch (type) {
            case 'n':
                partOfSpeech = "noun";
                break;
            case 'v':
                partOfSpeech = "verb";
                break;
            case 'a':
            case 's':
                partOfSpeech = "adjective";
                break;
            case 'r':
                partOfSpeech = "adverb";
                break;
            default:
                throw std::runtime_error("Invalid WordNet part of speech");
        }

        const auto separator = line.find('|');
        if (separator == std::string_view::npos)
            throw std::runtime_error("Missing WordNet gloss");

        const auto gloss = line.substr(separator + 1);
        const auto first = gloss.find_first_not_of(" \r\t");
        const auto last = gloss.find_last_not_of(" \r\t");
        const auto definition =
            first == std::string_view::npos ? std::string{} : std::string(gloss.substr(first, last - first + 1));
        result.push_back({"oewn-" + offset + "-" + type, std::move(partOfSpeech), definition, std::move(alternatives)});
    }
}

} // namespace LexiBridge::Core::Translator::Synonyms
