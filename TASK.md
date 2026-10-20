# TASK.md — LexiBridge

## Doel

Een CLI-hulpmiddel voor offline vertaling Nederlands ↔ Engels en Engelse synoniemen, om preciezere woorden te vinden.

## Accepted — uitgangspunten

- Alleen C/C++ voor applicatie, libraries, tests en eventuele hulptools. Alleen CMake voor buildautomatisering, downloaden en voorbereiden van taaldata. Geen Python, Java, JavaScript, shell-, batch- of PowerShell-scripts als onderdeel van de oplossing of vereiste voorbereidingsroute. Dit geldt ook voor benodigde dependency-buildstappen en modelconversie.
- Behoud het skeleton: `core/translator` voor verwerking, `apps/translator` voor de CLI. Executablenaam: `translator`.
- Behoud C++20, CMake 4.0, CppCMake, CLI11 en de bestaande testopzet. Volg `AGENTS.md`; wijzig niets onder `extern/`.
- Bergamot is de vertaalengine. De dependency hoort in CppDependencies; de modelhelper blijft projectlokaal.
- Dependencies en taaldata mogen via CMake worden opgehaald. Vertalen en opzoeken werken daarna volledig offline op CPU.
- Windows/MSVC is de gebruikersomgeving. Bouw en test ook met de beschikbare sandbox-toolchain; vermeld wat daadwerkelijk is getest.
- Huidige scope: CLI, vertaling en Engelse synoniemen. GUI en andere functies vallen buiten deze taken.

## Fase 1 — offline vertalen

Voer fase 1 uit en lever op voor review. Start fase 2 pas na opdracht.

> Status 18 oktober 2026: fase 1 opgeleverd voor review. Release-build en alle
> vier CTest-suites slagen met de echte Mozilla-modellen in beide richtingen.
> Beide CLI-richtingen zijn gedemonstreerd met lokale data, UTF-8 en meerdere regels.
> De zes door de gebruiker aangeleverde archieven zijn via CMake in een lege
> datamap voorbereid; alle uitgepakte assets worden op SHA-256 gecontroleerd.
> Automatische HTTP-downloads zijn hier niet uitgevoerd door de proxybeperking;
> de ondersteunde lokale-archiefroute is volledig uitgevoerd. Zie README voor
> reproduceerbare commando's en beperkingen. Fase 2 is niet gestart.
> De gebruiker bevestigt ook de volledige Windows/MSVC Debug-build en 4/4 groene
> CTests. Datavoorbereiding loopt automatisch mee in de normale build.

### 1. Bergamot integreren

- [x] Oorspronkelijke Bergamot-integratie opgeleverd; de migratie naar CppDependencies staat hieronder apart ter validatie.
- [x] Vervang de Bergamot-TODO in root-CMake door include en resolveraanroep; link de teruggegeven target aan `core/translator`.
- [x] Leg de gebruikte revisie vast, hergebruik beschikbare targets/packages en behoud de source-fetch/buildroute. Houd dependencydetails uit root-CMake en schakel onnodige upstream-voorbeelden/tools uit.
- [x] Bouw de engine met de echte Bergamot-library en benodigde dependencies via CMake, zonder aanvullende programmeertalen of scripts.

### 2. Modellen beschikbaar maken

- [x] Selecteer werkende Bergamot-modellen voor NL→EN en EN→NL, inclusief bijbehorende vocabulaire/tokenizer en configuratie.
- [x] Voeg een CMake-voorbereidingsstap toe die de benodigde bestanden ophaalt en in een configureerbare lokale datamap klaarzet. Hergebruik reeds aanwezige data.
- [x] Gebruik direct bruikbare modellen; eventuele noodzakelijke conversie gebeurt met C/C++-tools aangestuurd door CMake.
- [x] Noteer bron, versie en licentie van de gebruikte modellen in README.md; houd grote databestanden en gegenereerde binaries buiten de brondiff.

### 3. Engine en CLI opleveren

- [x] Implementeer in `core/translator` een publieke API voor vertaling met taalrichting, invoertekst en lokaal datapad.
- [x] Implementeer met CLI11 de opdracht `translator translate`, met `--from`, `--to`, `--text` of `--input`, en `--data-dir`.
- [x] Verbind de CLI met de echte engine. Ondersteun UTF-8 en meerregelige tekst zonder stil afkappen.
- [x] Schrijf vertalingen naar stdout en fouten naar stderr. Geef een niet-nul exitcode bij ongeldige argumenten, onleesbare invoer, ontbrekende/onbruikbare modellen en vertaalfouten.
- [x] Voeg helptekst en korte README-instructies toe voor build, datavoorbereiding en gebruik in beide richtingen.

### 4. Testen en accepteren

- [x] Vervang skeleton-placeholdertests door betekenisvolle tests via `cppcmake_test_add()`; behoud de bestaande INTERFACE-supporttarget voor app-tests.
- [x] Test beide vertaalrichtingen via de publieke engine-API met echte modellen en inhoudelijke verwachte resultaten.
- [x] Test CLI-invoerverwerking, taalrichting en foutafhandeling; neem accenten, meerregelige tekst en ontbrekende modellen mee.
- [x] Demonstreer beide richtingen via de echte CLI met lokale data en zonder netwerkgebruik tijdens vertaling.
- [x] Bouw productie- en testtargets en voer CTest uit na de laatste codewijziging. Rond fase 1 alleen af bij een groene relevante suite met werkelijk uitgevoerde modeltests.

## Fase 2 — Engelse synoniemen

> Status 20 oktober 2026: uitgevoerd op basis van de gereviewde commit d124fd4.
> OEWN 2025 WNDB wordt rechtstreeks verwerkt. Automatische download, lokale
> archiefvoorbereiding, hergebruik en herstel zijn uitgevoerd. De Release-build
> met GCC 14.2.0/CMake 4.0.3 en alle zes CTest-suites zijn groen, inclusief echte
> WordNet-data en de vertaaltests uit fase 1. Offline CLI-lookup is gedemonstreerd.
> Windows/MSVC is in deze fase niet uitgevoerd. Zie doc/phase2-validation.md.

- [x] Gebruik Open English WordNet als uitgangspunt en kies een dataformaat dat rechtstreeks met C/C++ verwerkt kan worden.
- [x] Voeg datavoorbereiding via CMake toe; gebruik alleen indien nodig een kleine C/C++-hulptool voor conversie.
- [x] Implementeer lokale lookup in de engine en voeg `translator synonyms --word <woord> --data-dir <pad>` toe aan de bestaande CLI.
- [x] Toon synoniemen per betekenis, met woordsoort en definitie waar beschikbaar. Normaliseer hoofdletters en omliggende whitespace; verwijder duplicaten binnen dezelfde betekenis.
- [x] Onderscheid 'geen resultaten' van ontbrekende of onbruikbare woordenboekdata. Verzin geen labels zoals 'formeler' wanneer de bron die niet levert.
- [x] Voeg inhoudelijke tests met echte woordenboekdata toe voor lookup, meerdere betekenissen, normalisatie, onbekende woorden en foutafhandeling.
- [x] Werk helptekst en README bij met datavoorbereiding, voorbeelden, bron, versie en vereiste attributies.
- [x] Demonstreer offline lookup via de CLI en voer CTest uit, inclusief de vertaaltests uit fase 1. Rond fase 2 alleen af bij een groene relevante suite.

## Oplevering per fase

- Vink alleen uitgevoerde en geverifieerde taken af.
- Rapporteer kort wat werkt, de build-/CTest-commando's met resultaat en eventuele blokkades.
- Ontbrekende modellen, overgeslagen essentiële tests of dummyresultaten gelden niet als voltooiing.
- Los routineproblemen binnen de fase op. Meld een concrete blokkade wanneer een dependency of model niet binnen de C/C++- en CMake-afspraak bruikbaar is; voeg niet stil een andere taal, script of backend toe.

## Wijziging

18 oktober 2026: teruggebracht tot concrete CLI-taken; C/C++ en CMake expliciet als enige implementatie- en voorbereidingsroute vastgelegd.
