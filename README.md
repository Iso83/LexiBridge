# LexiBridge

**A small offline language companion for English and Dutch.**

LexiBridge is designed for people who already understand and speak English, but occasionally struggle to find the right words.

Sometimes you know exactly what you want to say, but the English word that comes to mind feels too basic. You want to express yourself naturally, without sounding like a tourist relying on a handful of common phrases.

That's where LexiBridge comes in.

The goal isn't to learn English from scratch, but to help you find better words, improve your vocabulary, and express yourself with more confidence.

## Features

- **Offline translation** between Dutch and English.
- **Neural machine translation** powered by [Bergamot](https://github.com/browsermt/bergamot-translator).
- **Privacy-friendly** — translations happen locally, without sending text to online services.
- **Lightweight CLI** for quick translations.
- **English synonyms** grouped by meaning, with part of speech and the original WordNet gloss.

## Getting started

LexiBridge provides command-line translation and English synonym lookup. Both run offline after preparing the data.

### Build and prepare data

Requirements: Git, CMake 4.0 or newer, a C++20 compiler (MSVC on Windows, GCC on Linux).

```sh
git clone https://github.com/Iso83/LexiBridge.Sandbox.git
cd LexiBridge.Sandbox
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --parallel 4
ctest --test-dir build -C Release -N
ctest --test-dir build -C Release --output-on-failure
```

The first configure/build needs network access for dependencies and data. The normal
build prepares both translation models and WordNet under `build/data` and verifies
existing files before reusing them. Set `-DLEXIBRIDGE_DATA_DIR=<path>` at configure
time to choose another location. Keep generated data outside the source tree.

For data preparation alone, use the `LexiBridge_tools_models_prepare` or
`LexiBridge_tools_wordnet_prepare` build target. The WordNet target has no model dependency.
WordNet can also be prepared without configuring or compiling the project:

```sh
cmake -DLEXIBRIDGE_SOURCE_DIR=. -DLEXIBRIDGE_DATA_DIR=build/data -P tools/wordnet/cmake/PrepareWordNet.cmake
```

For machines without download access, configure with `-DLEXIBRIDGE_MODEL_ARCHIVE_DIR=<directory>` pointing to a directory containing the six extracted .gz files from the [Mozilla Bergamot translation models (nl-en and en-nl)](https://mozilla.github.io/translations/final-evals), and `-DLEXIBRIDGE_WORDNET_ARCHIVE=<path/to/english-wordnet-2025-index.sense-fixed.zip>`.
The WordNet option also works with the standalone preparation command above. Local
WordNet archives must match the pinned SHA-256, just like downloads. Prepared data
can be copied to another machine, retain the WordNet license and attribution files.
No downloading occurs during `translate`, `synonyms`, or CTest.
The preparation regression test uses the verified WordNet archive retained in
`build/data/cache/wordnet-2025`, retain that cache when moving a build used for testing.

### Translate offline

With the required translation models prepared, translate Dutch text into English using:

```bash
build/apps/translator/translator translate --from nl --to en --text "De kat slaapt." --data-dir build/data
build/apps/translator/translator translate --from en --to nl --text "The cat is sleeping." --data-dir build/data
```

Use `--from` and `--to` to select the source and target languages.

Translation models are stored locally and must be available before translating.
Use `--input <file>` instead of `--text` to read an entire UTF-8 file.
With a multi-configuration generator such as Visual Studio, the executable is
`build/apps/translator/Release/translator.exe` for the Release configuration.

### Find English synonyms offline

```sh
build/apps/translator/translator synonyms --word bank --data-dir build/data
build/apps/translator/translator synonyms --word "  QUICK  " --data-dir build/data
build/apps/translator/translator synonyms --word "ice cream" --data-dir build/data
build/apps/translator/translator synonyms --help
```

Each meaning is shown separately with its part of speech, source definition and
usage examples, and synonyms from the same synset. For example, `bank` includes
the financial institution meaning with `banking company` and the money-container
meaning with `money box`. Meanings with no alternatives are explicitly shown.
The queried lemma is excluded and alternatives are deduplicated within each meaning.
Adjective satellites are shown as adjectives, WNDB adjective-position markers are
removed and underscores in expressions are displayed as spaces.

Lookup ignores ASCII case and surrounding whitespace. Spaces and underscores in
multiword lemmas are equivalent. Lookup uses exact dictionary lemmas, without
stemming, spelling correction or inferred labels such as “more formal”.

An unknown lemma prints `No WordNet results.` to stdout and exits with code 0.
Invalid arguments and missing, corrupt or unsupported dictionary data print an error
to stderr and exit with a nonzero code. `--data-dir` names the shared data root,
which contains the `wordnet` subdirectory, not the subdirectory itself.

### Dictionary source and attribution

The dictionary is [Open English WordNet 2025](https://github.com/globalwordnet/english-wordnet/releases/tag/2025-edition),
using the corrected WNDB release `english-wordnet-2025-index.sense-fixed.zip`
published on 26 June 2026. The archive SHA-256 is
`38b16326159f51853626b7d24a44c453fa88ab33f06fce5ec8fc5996d1c2be93`.
The four `data.*` text files are read directly by C++, no conversion tool or
generated database is needed. CMake verifies SHA-256 on download and preparation,
runtime CRC-32 checks reject damaged or different-edition data before returning results.

Copyright 2019–present, the Open English WordNet Team. Licensed under
[Creative Commons Attribution 4.0 International](https://creativecommons.org/licenses/by/4.0/).
Derived from Princeton University WordNet, subject to the WordNet License.
The original data files and their notices are unmodified. Display changes are
described above, complete upstream notices are retained in
[LICENSE.md](doc/wordnet/LICENSE.md), [WNDB_License.txt](doc/wordnet/WNDB_License.txt)
and [ATTRIBUTION.txt](doc/wordnet/ATTRIBUTION.txt), and copied into the prepared data directory.

## Project status

LexiBridge is being developed in stages, with additional features planned as time permits.

- **Completed — Translator engine:** Offline neural translation between Dutch and English using Bergamot.
- **Completed — Synonym dictionary:** Offline English meanings and alternatives from Open English WordNet 2025.
- **Planned — Graphical interface:** Make translation and word lookup accessible without the command line.
