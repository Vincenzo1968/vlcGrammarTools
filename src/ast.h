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

#ifndef AST_H
#define AST_H

#include "symtab.h"
#include "scanner.h"

#define T_EMPTY -1
#define T_DOLLAR -2 // $

#define TYPE_CONSTRAINT_FIRST 1
#define TYPE_CONSTRAINT_FOLLOW 2

#define ARRAY_SIZE(Array) \
   (sizeof(Array) / sizeof((Array)[0]))

typedef struct tagTerminalList
{
	int idSymbol;
	struct tagTerminalList *next;
} TerminalList;

typedef struct tagProdsList
{
	int idProd;
	struct tagProdsList *next;
} ProdsList;

typedef struct tagProblemList
{
	int idNonTerminal;
	int idTerminal;
	struct tagProblemList *next;
} ProblemList;

typedef struct tagConstraintsList
{
	int idNT1;
	int idNT2;
	int type;
	struct tagConstraintsList *next;
} ConstraintsList;

typedef struct tagSymbol
{
	TokenTypeEnum Type;
	int id;
	char *strLexeme;
	int bNullable;
	TerminalList *pFirstSet;
	TerminalList *pFollowSet;
}Symbol;

typedef struct tagArrayProduction
{
	int count;
	Symbol *aSymbols;
}ArrayProduction;

typedef ArrayProduction Item; // for QuickSort

typedef struct tagTables
{
	int countTempNonTerminals;
	int countNonTerminals;
	Symbol *aNonTerminals;

	int countTempTerminals;
	int countTerminals;
	Symbol *aTerminals;

	int countTempProds;
	int countProds;
	ArrayProduction *aProds;

	ProdsList * **tableLL;

	ProblemList *problemList;

	ConstraintsList *constraintsList;
} Tables, *pTables;

void FreeConstraintsList(ConstraintsList *first);
ConstraintsList* NewNodeConstraintsList(ConstraintsList *Elem);
ConstraintsList* AppendConstraintsList(ConstraintsList *Elem, ConstraintsList *first);

void FreeProblemList(ProblemList *first);
ProblemList* NewNodeProblemList(ProblemList *Elem);
ProblemList* AppendProblemList(ProblemList *Elem, ProblemList *first);

void FreeTerminalList(TerminalList *first);
TerminalList* NewNodeTerminalList(TerminalList *Elem);
TerminalList* AppendTerminalList(TerminalList *Elem, TerminalList *first, int bNonEmpty);

void FreeProdsList(ProdsList *first);
ProdsList* NewNodeProdsList(ProdsList *Elem);
ProdsList* AppendProdsList(ProdsList *Elem, ProdsList *first);


void InitTables(Tables *pTables);
void PrintTables(Tables *pTables);
void ReallocArrays(Tables *pTables);
void MakeArrays(Tables *pTables);
void FreeArrays(Tables *pTables);

void Raggruppa(Tables *pTables);
extern int (*pfnCompare)(Item, Item); // Pointer to the comparison function for QuickSort
int Compare(Item a, Item b); // Comparison function for QuickSort
void QuickSortNonRecursive(Item *a, int start, int stop);
int FindInStack(int id, int *pStack, int top);
void Ordina(Tables *pTables);

int IsNullable(Tables *pTables, char *strSym);
void MakeNullableSet(Tables *pTables);
int MutuallyRecursive(Tables *pTables);
void MakeFirstSets(Tables *pTables);
void MakeFollowSets(Tables *pTables);

void MakeTableLL(Tables *pTables);

void MakeHtmlFile(Tables *pTables, const char *szFileName);

void MakeCFile(Tables *pTables, const char *szFileName);

int SimulateLL1(Tables *pTables, int *tokens, int numTokens);

#endif // AST_H
