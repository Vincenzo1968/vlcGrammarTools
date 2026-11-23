/*
   Copyright (C) 2013 Vincenzo Lo Cicero

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

#ifndef LALR_H
#define LALR_H

#include "ast.h"

/* --- AZIONI DELLA TABELLA LALR(1) --- */
typedef enum {
    ACT_ERROR = 0,
    ACT_SHIFT,
    ACT_REDUCE,
    ACT_ACCEPT
} LalrActionType;

typedef struct tagLalrAction {
    LalrActionType type;
    int value; /* Numero stato (per SHIFT) o ID produzione (per REDUCE) */
    struct tagLalrAction *next; /* Lista per rilevare conflitti S/R o R/R */
} LalrAction;

/* --- ITEM LR(1) / LALR(1): [A -> alpha . beta, lookaheads] --- */
typedef struct {
    int prodId;       /* Indice in pTables->aProds (-1 per S' -> start) */
    int dotPos;       /* Posizione del pallino (da 1 a count) */
    int *lookaheads;  /* Array booleano di dimensione countTerminals + 1 ($ = countTerminals) */
} LalrItem;

/* --- STATO DELL'AUTOMA LALR(1) --- */
typedef struct {
    int id;
    int numItems;
    int capItems;
    LalrItem *items;
    int *transitions; /* Dimensione: countNonTerminals + countTerminals (-1 = nessuna) */
} LalrState;

/* --- LOG DEI PASSAGGI DI CONVERSIONE (per report HTML) --- */
typedef struct tagConversionStep {
    char title[256];
    char description[512];
    char *grammarSnapshot; /* Testo formattato della grammatica in questo step */
    struct tagConversionStep *next;
} ConversionStep;

typedef struct {
    int converted;          /* 1 se è stata effettuata una conversione */
    int success;            /* 1 se la conversione ha prodotto una grammatica valida */
    char sourceType[32];    /* "LL(1)" o "LALR(1)" */
    char targetType[32];    /* "LALR(1)" o "LL(1)" */
    ConversionStep *steps;
} ConversionReport;

/* --- TABELLE E AUTOMA LALR(1) --- */
typedef struct {
    int numStates;
    int capStates;
    LalrState *states;

    /* Matrice ACTION[numStates][countTerminals + 1] */
    LalrAction ***actionTable;
    /* Matrice GOTO[numStates][countNonTerminals] */
    int **gotoTable;

    int numConflicts;
    int isLALR1;
} LalrTables;

/* API Principale LALR(1) */
void InitLalrTables(LalrTables *pLalr);
void FreeLalrTables(LalrTables *pLalr, const Tables *pTables);
int  BuildLALR1(Tables *pTables, LalrTables *pLalr);

/* API Report di conversione */
void InitConversionReport(ConversionReport *pRep);
void AddConversionStep(ConversionReport *pRep, const char *title, const char *desc, const Tables *pTables);
void FreeConversionReport(ConversionReport *pRep);

/* Conversione LALR(1) -> LL(1) (Ricorsione sinistra + Left Factoring) */
int ConvertLALRtoLL1(Tables *pTables, ConversionReport *pRep);

/* Generatori Output per LALR(1) e Report */
void MakeLalrHtmlFile(Tables *pTables, LalrTables *pLalr, ConversionReport *pRep, const char *szFileName);
void MakeLalrCFile(Tables *pTables, LalrTables *pLalr, const char *szFileName);
void MakeHtmlFileWithReport(Tables *pTables, ConversionReport *pRep, const char *szFileName);

#endif /* LALR_H */
