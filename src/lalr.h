/*
   Copyright (C) 2013 Vincenzo Lo Cicero

   Author: Vincenzo Lo Cicero.
   e-mail: vincenzolocicero68@gmail.com
         
   This file is part of vlcGrammarTools.

   vlcGrammarTools is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   vlcGrammarTools is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with myoptParserLib.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef LALR_H
#define LALR_H

#include "ast.h"

/* --- ACTIONS IN THE LALR(1) TABLE --- */
typedef enum {
    ACT_ERROR = 0,
    ACT_SHIFT,
    ACT_REDUCE,
    ACT_ACCEPT
} LalrActionType;

typedef struct tagLalrAction {
    LalrActionType type;
    int value; /* Status number (for SHIFT) or production ID (for REDUCE) */
    struct tagLalrAction *next; /* List for detecting S/R or R/R conflicts */
} LalrAction;

/* --- ITEM LR(1) / LALR(1): [A -> alpha . beta, lookaheads] --- */
typedef struct {
    int prodId;       /* Index in pTables->aProds (-1 for S' -> start) */
    int dotPos;       /* Position of the dot (from 1 to count) */
    int *lookaheads;  /* Boolean array of size countTerminals + 1 ($ = countTerminals) */
} LalrItem;

/* --- STATUS OF THE LALR(1) AUTOMATON --- */
typedef struct {
    int id;
    int numItems;
    int capItems;
    LalrItem *items;
    int *transitions; /* Size: countNonTerminals + countTerminals (-1 = none) */
} LalrState;

/* --- CONVERSION STEP LOG (for HTML report) --- */
typedef struct tagConversionStep {
    char title[256];
    char description[512];
    char *grammarSnapshot; /* Formatted grammar text in this step */
    struct tagConversionStep *next;
} ConversionStep;

typedef struct {
    int converted;          /* 1 if a conversion has been carried out */
    int success;            /* 1 if the conversion produced a valid grammar */
    char sourceType[32];    /* "LL(1)" or "LALR(1)" */
    char targetType[32];    /* "LALR(1)" or "LL(1)" */
    ConversionStep *steps;
} ConversionReport;

/* --- TABLES AND LALR(1) PARSER --- */
typedef struct {
    int numStates;
    int capStates;
    LalrState *states;

    /* ACTION[numStates][countTerminals + 1] matrix */
    LalrAction ***actionTable;
    /* GOTO Matrix[numStates][countNonTerminals] */
    int **gotoTable;

    int numConflicts;
    int isLALR1;
} LalrTables;

/* LALR(1) Main API */
void InitLalrTables(LalrTables *pLalr);
void FreeLalrTables(LalrTables *pLalr, const Tables *pTables);
int  BuildLALR1(Tables *pTables, LalrTables *pLalr);

/* Conversion report */
void InitConversionReport(ConversionReport *pRep);
void AddConversionStep(ConversionReport *pRep, const char *title, const char *desc, const Tables *pTables);
void FreeConversionReport(ConversionReport *pRep);

/* LALR(1) -> LL(1) conversion (Left recursion + Left factoring) */
int ConvertLALRtoLL1(Tables *pTables, ConversionReport *pRep);

/* Output Generators for LALR(1) and Reports */
void MakeLalrHtmlFile(Tables *pTables, LalrTables *pLalr, ConversionReport *pRep, const char *szFileName);
void MakeLalrCFile(Tables *pTables, LalrTables *pLalr, const char *szFileName);
void MakeHtmlFileWithReport(Tables *pTables, ConversionReport *pRep, const char *szFileName);

#endif /* LALR_H */
