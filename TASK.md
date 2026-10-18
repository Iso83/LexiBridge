# TASK.md — LexiBridge

## Doel

Een CLI-hulpmiddel voor offline vertaling Nederlands ↔ Engels en Engelse synoniemen, om preciezere woorden te vinden.

## Accepted — uitgangspunten

- Alleen C/C++ voor applicatie, libraries, tests en eventuele hulptools. Alleen CMake voor buildautomatisering, downloaden en voorbereiden van taaldata. Geen Python, Java, JavaScript, shell-, batch- of PowerShell-scripts als onderdeel van de oplossing of vereiste voorbereidingsroute. Dit geldt ook voor benodigde dependency-buildstappen en modelconversie.
- Behoud het skeleton: `engine/translator` voor verwerking, `apps/translator` voor de CLI. Executablenaam: `translator`.
- Behoud C++20, CMake 4.0, CppCMake, CLI11 en de bestaande testopzet. Volg `AGENTS.md`; wijzig niets onder `extern/`.
- Bergamot is de vertaalengine. Voeg de dependency projectlokaal toe in CppDependencies-stijl.
- Dependencies en taaldata mogen via CMake worden opgehaald. Vertalen en opzoeken werken daarna volledig offline op CPU.
- Windows/MSVC is de gebruikersomgeving. Bouw en test ook met de beschikbare sandbox-toolchain; vermeld wat daadwerkelijk is getest.
- Huidige scope: CLI, vertaling en Engelse synoniemen. GUI en andere functies vallen buiten deze taken.

## Fase 1 — offline vertalen

Voer fase 1 uit en lever op voor review. Start fase 2 pas na opdracht.

### 1. Bergamot integreren

- [ ] Voeg één projectlokale `bergamot.cmake` toe onder `cmake/`, met een resolverfunctie en korte toelichting volgens de bestaande CppDependencies-stijl.
- [ ] Vervang de Bergamot-TODO in root-CMake door include en resolveraanroep; link de teruggegeven target aan `engine/translator`.
- [ ] Leg de gebruikte revisie vast, hergebruik beschikbare targets/packages en behoud de source-fetch/buildroute. Houd dependencydetails uit root-CMake en schakel onnodige upstream-voorbeelden/tools uit.
- [ ] Bouw de engine met de echte Bergamot-library en benodigde dependencies via CMake, zonder aanvullende programmeertalen of scripts.

### 2. Modellen beschikbaar maken

- [ ] Selecteer werkende Bergamot-modellen voor NL→EN en EN→NL, inclusief bijbehorende vocabulaire/tokenizer en configuratie.
- [ ] Voeg een CMake-voorbereidingsstap toe die de benodigde bestanden ophaalt en in een configureerbare lokale datamap klaarzet. Hergebruik reeds aanwezige data.
- [ ] Gebruik direct bruikbare modellen; eventuele noodzakelijke conversie gebeurt met C/C++-tools aangestuurd door CMake.
- [ ] Noteer bron, versie en licentie van de gebruikte modellen in README.md; houd grote databestanden en gegenereerde binaries buiten de brondiff.

### 3. Engine en CLI opleveren

- [ ] Implementeer in `engine/translator` een publieke API voor vertaling met taalrichting, invoertekst en lokaal datapad.
- [ ] Implementeer met CLI11 de opdracht `translator translate`, met `--from`, `--to`, `--text` of `--input`, en `--data-dir`.
- [ ] Verbind de CLI met de echte engine. Ondersteun UTF-8 en meerregelige tekst zonder stil afkappen.
- [ ] Schrijf vertalingen naar stdout en fouten naar stderr. Geef een niet-nul exitcode bij ongeldige argumenten, onleesbare invoer, ontbrekende/onbruikbare modellen en vertaalfouten.
- [ ] Voeg helptekst en korte README-instructies toe voor build, datavoorbereiding en gebruik in beide richtingen.

### 4. Testen en accepteren

- [ ] Vervang skeleton-placeholdertests door betekenisvolle tests via `cppcmake_test_add()`; behoud de bestaande INTERFACE-supporttarget voor app-tests.
- [ ] Test beide vertaalrichtingen via de publieke engine-API met echte modellen en inhoudelijke verwachte resultaten.
- [ ] Test CLI-invoerverwerking, taalrichting en foutafhandeling; neem accenten, meerregelige tekst en ontbrekende modellen mee.
- [ ] Demonstreer beide richtingen via de echte CLI met lokale data en zonder netwerkgebruik tijdens vertaling.
- [ ] Bouw productie- en testtargets en voer CTest uit na de laatste codewijziging. Rond fase 1 alleen af bij een groene relevante suite met werkelijk uitgevoerde modeltests.

## Fase 2 — Engelse synoniemen

- [ ] Gebruik Open English WordNet als uitgangspunt en kies een dataformaat dat rechtstreeks met C/C++ verwerkt kan worden.
- [ ] Voeg datavoorbereiding via CMake toe; gebruik alleen indien nodig een kleine C/C++-hulptool voor conversie.
- [ ] Implementeer lokale lookup in de engine en voeg `translator synonyms --word <woord> --data-dir <pad>` toe aan de bestaande CLI.
- [ ] Toon synoniemen per betekenis, met woordsoort en definitie waar beschikbaar. Normaliseer hoofdletters en omliggende whitespace; verwijder duplicaten binnen dezelfde betekenis.
- [ ] Onderscheid 'geen resultaten' van ontbrekende of onbruikbare woordenboekdata. Verzin geen labels zoals 'formeler' wanneer de bron die niet levert.
- [ ] Voeg inhoudelijke tests met echte woordenboekdata toe voor lookup, meerdere betekenissen, normalisatie, onbekende woorden en foutafhandeling.
- [ ] Werk helptekst en README bij met datavoorbereiding, voorbeelden, bron, versie en vereiste attributies.
- [ ] Demonstreer offline lookup via de CLI en voer CTest uit, inclusief de vertaaltests uit fase 1. Rond fase 2 alleen af bij een groene relevante suite.

## Oplevering per fase

- Vink alleen uitgevoerde en geverifieerde taken af.
- Rapporteer kort wat werkt, de build-/CTest-commando's met resultaat en eventuele blokkades.
- Ontbrekende modellen, overgeslagen essentiële tests of dummyresultaten gelden niet als voltooiing.
- Los routineproblemen binnen de fase op. Meld een concrete blokkade wanneer een dependency of model niet binnen de C/C++- en CMake-afspraak bruikbaar is; voeg niet stil een andere taal, script of backend toe.

## Wijziging

18 oktober 2026: teruggebracht tot concrete CLI-taken; C/C++ en CMake expliciet als enige implementatie- en voorbereidingsroute vastgelegd.
