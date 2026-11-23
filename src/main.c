#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifndef PACKAGE_NAME
#define PACKAGE_NAME "vlcGrammarTools"
#endif
#ifndef PACKAGE_VERSION
#define PACKAGE_VERSION "2.0"
#endif

/*
   Copyright (C) 2013-2026 Vincenzo Lo Cicero

   Author: Vincenzo Lo Cicero.
   e-mail: vincenzolocicero68@gmail.com
         
   This file is part of vlcGrammarTools.

   vlcGrammarTools is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   myoptParserLib is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with myoptParserLib.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/types.h>
#ifdef _WIN32
  #include <direct.h>
  #define MKDIR(d) _mkdir(d)
#else
  #include <unistd.h>
  #define MKDIR(d) mkdir(d, 0755)
#endif

#include "scanner.h"
#include "parser.h"
#include "symtab.h"
#include "ast.h"
#include "lalr.h"
#include "myoptParser.h"

static void EnsureDirExists(const char *dirPath)
{
    struct stat st = {0};
    if (stat(dirPath, &st) == -1) {
        MKDIR(dirPath);
    }
}

static int FileExists(const char *path)
{
    struct stat st;
    return (stat(path, &st) == 0);
}

static void MakeOutputNames(const char *szFileInput, const char *customBase,
                            char *szOutHtml, char *szOutC)
{
    const char *outDir = "../output";
    EnsureDirExists(outDir);

    char baseName[512];
    if (customBase && *customBase != '\0') {
        strncpy(baseName, customBase, sizeof(baseName) - 1);
        baseName[sizeof(baseName) - 1] = '\0';
    } else {
        /* Estrae solo il nome file senza il percorso (es. ../grammar_src/grammar_001.grm -> grammar_001) */
        const char *slash1 = strrchr(szFileInput, '/');
        const char *slash2 = strrchr(szFileInput, '\\');
        const char *fname = szFileInput;
        if (slash1 && slash2) fname = (slash1 > slash2 ? slash1 : slash2) + 1;
        else if (slash1) fname = slash1 + 1;
        else if (slash2) fname = slash2 + 1;

        strncpy(baseName, fname, sizeof(baseName) - 1);
        baseName[sizeof(baseName) - 1] = '\0';
        char *dot = strrchr(baseName, '.');
        if (dot) *dot = '\0';
    }

    snprintf(szOutHtml, 1024, "%s/%s.html", outDir, baseName);
    snprintf(szOutC, 1024, "%s/%s.c", outDir, baseName);

    int ver = 2;
    while (FileExists(szOutHtml) || FileExists(szOutC)) {
        snprintf(szOutHtml, 1024, "%s/%s(%d).html", outDir, baseName, ver);
        snprintf(szOutC, 1024, "%s/%s(%d).c", outDir, baseName, ver);
        ver++;
    }
}

static void PrintHelpInfo(myopt_Parser_t optParser)
{
    printf("%s %s - Analizzatore e Convertitore di Grammatiche LL(1) / LALR(1)\n\n",
           PACKAGE_NAME, PACKAGE_VERSION);
    PrintHelpInfo(optParser);
    printf("Esempi:\n");
    printf("  %s grammar_001.grm\n", PACKAGE_NAME);
    printf("  %s -c -o out_converted grammar_022.grm\n\n", PACKAGE_NAME);
}

static void PrintLicenseInfo(void)
{
	printf("Copyright (C) 2013-2026 Vincenzo Lo Cicero.\n");
	printf("\n");
	printf("Author: Vincenzo Lo Cicero.\n");
	printf("e-mail: vincenzolocicero68@gmail.com\n");
	printf("\n");
	printf("This file is part of vlcGrammarTools.\n");
	printf("\n");
	printf("vlcGrammarTools is free software: you can redistribute it and/or modify\n");
	printf("it under the terms of the GNU General Public License as published by\n");
	printf("the Free Software Foundation, either version 3 of the License, or\n");
	printf("(at your option) any later version.\n");
	printf("\n");
	printf("myoptParserLib is distributed in the hope that it will be useful,\n");
	printf("but WITHOUT ANY WARRANTY; without even the implied warranty of\n");
	printf("MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n");
	printf("GNU General Public License for more details.\n");
	printf("\n");	
	printf("You should have received a copy of the GNU General Public License\n");
	printf("along with myoptParserLib.  If not, see <http://www.gnu.org/licenses/>.\n");
}

static void PrintVersionInfo(void)
{
    printf("%s %s\n", PACKAGE_NAME, PACKAGE_VERSION);
    printf("Copyright (C) 2013-2026 Vincenzo Lo Cicero.\n");
}

int main(int argc, char **argv)
{
    myopt_Parser_t optParser;
    int32_t idGroupGeneral, idGroupMode;
    int32_t idHelp, idInfo, idVersion, idConvert, idToLL1, idToLALR1, idOutput;
    bool bParseOpt;

    const char *szFileInput = NULL;
    const char *szCustomOut = NULL;
    char szFileOutput_Html[1024];
    char szFileOutput_C[1024];

    optParser = myopt_InitParser();
    if (!optParser) {
        fprintf(stderr, "Errore di inizializzazione di myoptParser.\n");
        return -1;
    }

    /* Gruppo 0: Opzioni generali */
    idGroupGeneral = myopt_AddGroup(optParser, "Opzioni generali", false, false);
    myopt_AddOption(optParser, idGroupGeneral, 'h', "help",
                    "Mostra questo messaggio di aiuto ed esce",
                    false, MOB_OVERRIDE, 0, 0, "");
    myopt_AddOption(optParser, idGroupGeneral, 'i', "info",
                    "Mostra informazioni sulla licenza ed esce",
                    false, MOB_OVERRIDE, 0, 0, "");
    myopt_AddOption(optParser, idGroupGeneral, 'v', "version",
                    "Mostra la versione del programma ed esce",
                    false, MOB_OVERRIDE, 0, 0, "");
    myopt_AddOption(optParser, idGroupGeneral, 'o', "output",
                    "Prefisso nome file di output (senza estensione)",
                    false, MOB_OVERRIDE, 1, 1, "string");

    /* Gruppo 1: Modalita' di conversione (mutuamente esclusive, opzionali) */
    idGroupMode = myopt_AddGroup(optParser, "Conversione grammatica", true, false);
    myopt_AddOption(optParser, idGroupMode, 'c', "convert",
                    "Converte automaticamente LL(1) <-> LALR(1) nell'altro formato",
                    false, MOB_OVERRIDE, 0, 0, "");
    myopt_AddOption(optParser, idGroupMode, 0, "to-ll1",
                    "Forza la conversione e generazione per parser LL(1)",
                    false, MOB_OVERRIDE, 0, 0, "");
    myopt_AddOption(optParser, idGroupMode, 0, "to-lalr1",
                    "Forza la conversione e generazione per parser LALR(1)",
                    false, MOB_OVERRIDE, 0, 0, "");

    /* Argomento posizionale: 1 file di grammatica .txt */
    myopt_SetPositionalArgsParams(optParser, "FILE_GRAMMATICA", 0, 1, "string");

    bParseOpt = myopt_ParseArray(optParser, argc, argv);
    if (optParser->countInternalErrors > 0) {
        fprintf(stderr, "Errore interno opzioni: %s\n", optParser->strInternalErrors);
        myopt_FreeParser(optParser);
        return -1;
    }

    idHelp = myopt_LookupLong(optParser, "help");
    if (idHelp >= 0 && optParser->arrayOptArgs[idHelp].countOccurrences > 0) {
        PrintHelpInfo(optParser);
        myopt_FreeParser(optParser);
        return 0;
    }

    idInfo = myopt_LookupLong(optParser, "info");
    if (idInfo >= 0 && optParser->arrayOptArgs[idInfo].countOccurrences > 0) {
        PrintLicenseInfo();
        myopt_FreeParser(optParser);
        return 0;
    }

    idVersion = myopt_LookupLong(optParser, "version");
    if (idVersion >= 0 && optParser->arrayOptArgs[idVersion].countOccurrences > 0) {
        PrintVersionInfo();
        myopt_FreeParser(optParser);
        return 0;
    }

    if (!bParseOpt || optParser->countPosArgs < 1) {
        PrintHelpInfo(optParser);
        if (optParser->strErrors[0] != '\0')
            fprintf(stderr, "%s\n", optParser->strErrors);
        myopt_FreeParser(optParser);
        return -1;
    }

    szFileInput = optParser->arrayPosArgs[0].strValue;

    idOutput = myopt_LookupLong(optParser, "output");
    if (idOutput >= 0 && optParser->arrayOptArgs[idOutput].countOccurrences > 0) {
        szCustomOut = optParser->arrayOptArgs[idOutput].arrayArgs[0].strValue;
    }

    idConvert = myopt_LookupLong(optParser, "convert");
    idToLL1   = myopt_LookupLong(optParser, "to-ll1");
    idToLALR1 = myopt_LookupLong(optParser, "to-lalr1");

    bool optConvert = (idConvert >= 0 && optParser->arrayOptArgs[idConvert].countOccurrences > 0);
    bool optToLL1   = (idToLL1   >= 0 && optParser->arrayOptArgs[idToLL1].countOccurrences > 0);
    bool optToLALR1 = (idToLALR1 >= 0 && optParser->arrayOptArgs[idToLALR1].countOccurrences > 0);

    MakeOutputNames(szFileInput, szCustomOut, szFileOutput_Html, szFileOutput_C);

    /* --- 1. PARSING DELLA GRAMMATICA --- */
    Tables tables;
    InitTables(&tables);
    MakeArrays(&tables);

    int ret = Parse((char *)szFileInput, &tables);
    if (ret) ReallocArrays(&tables);

    if (!ret || tables.countProds == 0) {
        FreeArrays(&tables);
        myopt_FreeParser(optParser);
        return -1;
    }

    /* --- 2. ANALISI LL(1) E LALR(1) --- */
    Ordina(&tables);
    MakeNullableSet(&tables);
    MakeFirstSets(&tables);
    MakeFollowSets(&tables);
    MakeTableLL(&tables);

    bool isLL1 = (tables.problemList == NULL);

    LalrTables lalr;
    InitLalrTables(&lalr);
    bool isLALR1 = (BuildLALR1(&tables, &lalr) && lalr.isLALR1);

    if (!isLL1 && !isLALR1) {
        printf("Spiacente, la grammatica nel file %s non e' supportata\n", szFileInput);
        FreeLalrTables(&lalr, &tables);
        FreeArrays(&tables);
        myopt_FreeParser(optParser);
        return 0;
    }

    /* Determina il tipo rilevato (LL(1) e' un sottoinsieme proprio di LALR(1)) */
    const char *detectedType = isLL1 ? "LL(1)" : "LALR(1)";
    printf("Grammatica rilevata nel file '%s': %s\n", szFileInput, detectedType);

    /* --- 3. DECISIONE TARGET & CONVERSIONE --- */
    bool targetIsLALR1 = !isLL1; /* Default: mantieni il tipo rilevato */
    if (optConvert) {
        targetIsLALR1 = isLL1;   /* Inverti: LL(1)->LALR(1), LALR(1)->LL(1) */
    } else if (optToLALR1) {
        targetIsLALR1 = true;
    } else if (optToLL1) {
        targetIsLALR1 = false;
    }

    ConversionReport report;
    InitConversionReport(&report);
    strcpy(report.sourceType, detectedType);
    strcpy(report.targetType, targetIsLALR1 ? "LALR(1)" : "LL(1)");

    if (!targetIsLALR1) {
        /* Target: LL(1) */
        if (!isLL1) {
            printf("Conversione da LALR(1) a LL(1) in corso...\n");
            report.converted = 1;
            if (!ConvertLALRtoLL1(&tables, &report)) {
                printf("Attenzione: la grammatica non e' convertibile in LL(1) senza conflitti.\n");
                MakeHtmlFileWithReport(&tables, &report, szFileOutput_Html);
                printf("Report con analisi conflitti salvato in '%s'\n", szFileOutput_Html);
                FreeConversionReport(&report);
                FreeLalrTables(&lalr, &tables);
                FreeArrays(&tables);
                myopt_FreeParser(optParser);
                return 0;
            }
            printf("Conversione in LL(1) completata con successo!\n");
        }
        PrintTables(&tables);
        MakeHtmlFileWithReport(&tables, &report, szFileOutput_Html);
        MakeCFile(&tables, szFileOutput_C);
        printf("File '%s' e '%s' generati (Modalita' LL(1)).\n", szFileOutput_Html, szFileOutput_C);
    } else {
        /* Target: LALR(1) */
        if (isLL1 && (optConvert || optToLALR1)) {
            printf("Conversione da LL(1) a LALR(1) (costruzione automa e tabelle Action/Goto)...\n");
            report.converted = 1;
            report.success = 1;
            AddConversionStep(&report, "Grammatica LL(1) di partenza",
                              "Ogni grammatica LL(1) e' gia' compatibile con l'analisi bottom-up LALR(1). Si procede alla generazione degli stati LR(0) e alla propagazione dei lookahead.",
                              &tables);
        }
        MakeLalrHtmlFile(&tables, &lalr, &report, szFileOutput_Html);
        MakeLalrCFile(&tables, &lalr, szFileOutput_C);
        printf("File '%s' e '%s' generati (Modalita' LALR(1)).\n", szFileOutput_Html, szFileOutput_C);
    }

    FreeConversionReport(&report);
    FreeLalrTables(&lalr, &tables);
    FreeArrays(&tables);
    myopt_FreeParser(optParser);

    return 0;
}
