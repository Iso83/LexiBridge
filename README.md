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
- **Synonym assistance** — planned to help discover more suitable and expressive words.

## Getting started

LexiBridge currently provides a command-line translator.

### Translate offline

With the required translation models installed in the `data` directory, translate Dutch text into English using:

```bash
apps/translator/translator translate --from nl --to en --text "De kat slaapt." --data-dir data
```

Use `--from` and `--to` to select the source and target languages.

Translation models are stored locally and must be available before translating.

## Project status

LexiBridge is being developed in stages, with additional features planned as time permits.

- **Completed — Translator engine:** Offline neural translation between Dutch and English using Bergamot.
- **Next — Synonym dictionary:** Find alternative words and expressions to improve vocabulary and choose more natural wording.
- **Planned — Graphical interface:** Make translation and word lookup accessible without the command line.
