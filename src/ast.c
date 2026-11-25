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

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "ast.h"


int (*pfnCompare)(Item, Item) = NULL;

void InitTables(Tables *pTables)
{
	pTables->startSymbolId = 0;
	pTables->strPrologue = NULL;
	pTables->strEpilogue = NULL;
	pTables->bIgnoreActions = 0;
	
	pTables->aNonTerminals = NULL;
	pTables->aTerminals = NULL;
	pTables->aProds = NULL;
	pTables->tableLL = NULL;

	pTables->countTempNonTerminals = 21;
	pTables->countTempTerminals = 21;
	pTables->countTempProds = 21;

	pTables->countNonTerminals = 0;
	pTables->countTerminals = 0;
	pTables->countProds = 0;

	pTables->problemList = NULL;

	pTables->constraintsList = NULL;
}

void PrintHtmlEscaped(FILE *fp, const char *s)
{
	if (!s) return;
	for (const char *p = s; *p != '\0'; p++) {
		switch (*p) {
			case '&': fputs("&amp;", fp); break;
			case '<': fputs("&lt;", fp); break;
			case '>': fputs("&gt;", fp); break;
			case '"': fputs("&quot;", fp); break;
			default:  fputc(*p, fp); break;
		}
	}
}

int IsValidCIdent(const char *s)
{
	if (!s || !((*s >= 'A' && *s <= 'Z') || (*s >= 'a' && *s <= 'z') || *s == '_'))
		return 0;
	for (const char *p = s + 1; *p != '\0'; p++) {
		if (!((*p >= 'A' && *p <= 'Z') || (*p >= 'a' && *p <= 'z') ||
		      (*p >= '0' && *p <= '9') || *p == '_'))
			return 0;
	}
	return 1;
}

void PrintCSanitizedComment(FILE *fp, const char *s)
{
	if (!s) return;
	for (const char *p = s; *p != '\0'; p++) {
		unsigned char c = (unsigned char)*p;
		if (c == '*' && *(p + 1) == '/') {
			fputs("* /", fp);
			p++;
		} else if (c < 0x20 || c == 0x7f) {
			fprintf(fp, "\\x%02x", c);
		} else {
			fputc(c, fp);
		}
	}
}

// *********************************** Begin ConstraintsList ****************************************

void FreeConstraintsList(ConstraintsList *pList)
{
	ConstraintsList *pCurr = pList;
	while (pCurr != NULL)
	{
		ConstraintsList *pNext = pCurr->next;
		free(pCurr);
		pCurr = pNext;
	}
}

ConstraintsList* NewNodeConstraintsList(ConstraintsList *Elem)
{
	ConstraintsList *n;

	n = (ConstraintsList*)calloc(1, sizeof(ConstraintsList));

	if( n == NULL )
		return NULL;

	n->idNT1 = Elem->idNT1;
	n->idNT2 = Elem->idNT2;
	n->next = NULL;

	return n;
}

ConstraintsList* AppendConstraintsList(ConstraintsList *Elem, ConstraintsList *first)
{
	ConstraintsList *n = first, *nuovo;

	// empty chain
	if ( first == NULL )
		return NewNodeConstraintsList(Elem);

	n = first;
	while( n->next != NULL )
	{
		n = n->next;
	}

	nuovo = NewNodeConstraintsList(Elem);
	
	n->next = nuovo;

	return first;
}

// *********************************** End ConstraintsList ****************************************

// *********************************** Begin ProblemList ****************************************

void FreeProblemList(ProblemList *first)
{
	ProblemList *n1 = first, *n2;
	while ( n1 != NULL )
	{
		n2 = n1->next;
		free(n1);
		n1 = n2;
	}
}

ProblemList* NewNodeProblemList(ProblemList *Elem)
{
	ProblemList *n;

	n = (ProblemList *)malloc(sizeof(ProblemList));

	if( n == NULL )
		return NULL;

	n->idNonTerminal = Elem->idNonTerminal;
	n->idTerminal = Elem->idTerminal;
	n->next = NULL;

	return n;
}

ProblemList* AppendProblemList(ProblemList *Elem, ProblemList *first)
{
	ProblemList *n = first, *nuovo;

	// empty chain
	if ( first == NULL )
		return NewNodeProblemList(Elem);

	n = first;
	while( n->next != NULL )
	{
		n = n->next;
	}

	nuovo = NewNodeProblemList(Elem);
	
	n->next = nuovo;

	return first;
}

// *********************************** End ProblemList ****************************************

// *********************************** Begin TerminalList ****************************************

void FreeTerminalList(TerminalList *first)
{
	TerminalList *n1 = first, *n2;
	while ( n1 != NULL )
	{
		n2 = n1->next;
		free(n1);
		n1 = n2;
	}
}

TerminalList* NewNodeTerminalList(TerminalList *Elem)
{
	TerminalList *n;

	n = (TerminalList *)malloc(sizeof(TerminalList));

	if( n == NULL )
		return NULL;

	n->idSymbol = Elem->idSymbol;
	n->next = NULL;

	return n;
}

TerminalList* AppendTerminalList(TerminalList *Elem, TerminalList *first, int bNonEmpty)
{
	TerminalList *n = first, *nuovo;

	if ( bNonEmpty && Elem->idSymbol == T_EMPTY )
		return n;

	// empty chain
	if ( first == NULL )
		return NewNodeTerminalList(Elem);

	n = first;
	while( n->next != NULL )
	{
		n = n->next;
	}

	nuovo = NewNodeTerminalList(Elem);
	
	n->next = nuovo;

	return first;
}

// *********************************** End TerminalList ****************************************

// *********************************** Begin ProdsList ****************************************

void FreeProdsList(ProdsList *first)
{
	ProdsList *n1 = first, *n2;
	while ( n1 != NULL )
	{
		n2 = n1->next;
		free(n1);
		n1 = n2;
	}
}

ProdsList* NewNodeProdsList(ProdsList *Elem)
{
	ProdsList *n;

	n = (ProdsList *)malloc(sizeof(ProdsList));

	if( n == NULL )
		return NULL;

	n->idProd = Elem->idProd;
	n->next = NULL;

	return n;
}

ProdsList* AppendProdsList(ProdsList *Elem, ProdsList *first)
{
	ProdsList *n = first, *nuovo;

	// empty chain
	if ( first == NULL )
		return NewNodeProdsList(Elem);

	n = first;
	while( n->next != NULL )
	{
		n = n->next;
	}

	nuovo = NewNodeProdsList(Elem);
	
	n->next = nuovo;

	return first;
}

// *********************************** End ProdsList ****************************************

void FreeArrays(Tables *pTables)
{
	int x;
	int y;

	if ( pTables->aProds )
	{
		x = 0;
		for ( x = 0; x < pTables->countProds; x++ )
		{
			if ( pTables->aProds[x].aSymbols )
				free(pTables->aProds[x].aSymbols);
			if ( pTables->aProds[x].strAction )
				free(pTables->aProds[x].strAction);
		}
		free(pTables->aProds);
	}
	free(pTables->strPrologue);
	free(pTables->strEpilogue);

	if ( pTables->aTerminals )
	{
		for ( x = 0; x < pTables->countTerminals; x++ )
		{
			if ( pTables->aTerminals[x].strLexeme )
				free(pTables->aTerminals[x].strLexeme);
		}
		free(pTables->aTerminals);
	}

	if ( pTables->aNonTerminals )
	{
		for ( x = 0; x < pTables->countNonTerminals; x++ )
		{
			FreeTerminalList(pTables->aNonTerminals[x].pFirstSet);
			FreeTerminalList(pTables->aNonTerminals[x].pFollowSet);
			if ( pTables->aNonTerminals[x].strLexeme )
				free(pTables->aNonTerminals[x].strLexeme);
		}
		free(pTables->aNonTerminals);
	}


	if ( pTables->tableLL )
	{
		for ( x = 0; x < pTables->countNonTerminals; x++ )
		{
			for ( y = 0; y < pTables->countTerminals + 1; y++ )
			{
				if ( pTables->tableLL[x][y] )
					FreeProdsList(pTables->tableLL[x][y]);
			}
			if ( pTables->tableLL[x] )
				free(pTables->tableLL[x]);
		}
		free(pTables->tableLL);
	}

	FreeProblemList(pTables->problemList);

	FreeConstraintsList(pTables->constraintsList);
}

void MakeArrays(Tables *pTables)
{
	pTables->aNonTerminals = (Symbol*)calloc(pTables->countTempNonTerminals, sizeof(Symbol));
	if ( !pTables->aNonTerminals )
	{
		printf("Insufficient memory to allocate non-terminal arrays!\n");
		return;
	}

	pTables->aTerminals = (Symbol*)calloc(pTables->countTempTerminals, sizeof(Symbol));
	if ( !pTables->aTerminals )
	{
		printf("Insufficient memory to allocate terminal arrays!\n");
		return;
	}

	pTables->aProds = (ArrayProduction*)calloc(pTables->countTempProds, sizeof(ArrayProduction));	
	if ( !pTables->aProds )
	{
		printf("Insufficient memory to allocate production arrays!\n");
		return;
	}
	
	pTables->tableLL = NULL;
	pTables->problemList = NULL;
	pTables->constraintsList = NULL;		
}

void ReallocArrays(Tables *pTables)
{
	Symbol *pOldTerminals;
	Symbol *pOldNonTerminals;
	ArrayProduction *pOldProds;

	if ( pTables->countTerminals != pTables->countTempTerminals )
	{
		pOldTerminals = pTables->aTerminals;
		pTables->aTerminals = (Symbol*)realloc(pTables->aTerminals, (sizeof(Symbol))*pTables->countTerminals);
		if ( !pTables->aTerminals )
		{
			free(pOldTerminals);
			printf("Insufficient memory to reallocate terminals arrays!\n");
			return;
		}
	}

	if ( pTables->countNonTerminals != pTables->countTempNonTerminals )
	{
		pOldNonTerminals = pTables->aNonTerminals;
		pTables->aNonTerminals = (Symbol*)realloc(pTables->aNonTerminals, (sizeof(Symbol))*pTables->countNonTerminals);
		if ( !pTables->aNonTerminals )
		{
			free(pOldNonTerminals);
			printf("Insufficient memory to reallocate non-terminal arrays!\n");
			return;
		}
	}

	if ( pTables->countProds != pTables->countTempProds )
	{
		pOldProds = pTables->aProds;
		pTables->aProds = (ArrayProduction*)realloc(pTables->aProds, (sizeof(ArrayProduction))*pTables->countProds);
		if ( !pTables->aProds )
		{
			free(pOldProds);
			printf("Insufficient memory to reallocate productions array!\n");
			return;
		}
	}
}

void PrintTables(Tables *pTables)
{
	int x, y, z;

	int bDollar, bEmpty;

	TerminalList *pTermListTemp;
	ProdsList *pProdList;
	ProblemList *pProblemList;

	printf("\n");

	printf("%d Non-terminal Symbols:\n", pTables->countNonTerminals);
	for (x = 0; x < pTables->countNonTerminals; x++)
	{
		printf("%d %s\n", pTables->aNonTerminals[x].id, pTables->aNonTerminals[x].strLexeme);
	}

	printf("\n");

	// -1 Why don't we print the last symbol $
	printf("%d Terminal symbols:\n", pTables->countTerminals);
	for (x = 0; x < pTables->countTerminals; x++)
	{
		printf("%d %s\n", pTables->aTerminals[x].id, pTables->aTerminals[x].strLexeme);
	}

	printf("\n");

	printf("%d Productions:\n", pTables->countProds);
	//x = 0;
	for ( x = 0; x < pTables->countProds; x++ )
	{
		for ( y = 0; y < pTables->aProds[x].count; y++ )
		{
			printf("%s ", pTables->aProds[x].aSymbols[y].strLexeme);
			if ( y == 0 )
			{
				printf("-> ");
			}
		}
		printf("\n");
	}

	printf("\n");

	printf("Nullable Set:\n");
	for ( x = 0; x < pTables->countNonTerminals; x++ )
	{
		if ( pTables->aNonTerminals[x].bNullable )
			printf("%s\n", pTables->aNonTerminals[x].strLexeme);
	}

	printf("\n");

	printf("First Set:\n");
	for ( x = 0; x < pTables->countNonTerminals; x++ )
	{
		printf("FIRST(%s) = { ", pTables->aNonTerminals[x].strLexeme);

		bEmpty = 0;
		pTermListTemp = pTables->aNonTerminals[x].pFirstSet;
		while ( pTermListTemp != NULL )
		{
			//printf("%d ", pTermListTemp->idSymbol);
			if ( pTermListTemp->idSymbol == T_EMPTY )
			{
				//printf("empty ");
				bEmpty = 1;
			}
			else
			{
				printf("%s ", pTables->aTerminals[pTermListTemp->idSymbol].strLexeme);
			}
			pTermListTemp = pTermListTemp->next;
		}
		if ( bEmpty )
		{
			printf("empty ");
		}
		printf("}\n");
	}

	printf("\n");

	printf("Follow Set:\n");
	for ( x = 0; x < pTables->countNonTerminals; x++ )
	{
		printf("FOLLOW(%s) = { ", pTables->aNonTerminals[x].strLexeme);

		bDollar = 0;
		pTermListTemp = pTables->aNonTerminals[x].pFollowSet;
		while ( pTermListTemp != NULL )
		{
			if ( pTermListTemp->idSymbol == T_EMPTY )
			{
				printf("empty ");
			}
			else if ( pTermListTemp->idSymbol == T_DOLLAR )
			{
				bDollar = 1;
				//printf("$ ");
			}
			else
			{
				printf("%s ", pTables->aTerminals[pTermListTemp->idSymbol].strLexeme);
			}
			pTermListTemp = pTermListTemp->next;
		}
		if ( bDollar )
		{
			printf("$ ");
		}
		printf("}\n");
	}

	printf("\n");

	if ( pTables->problemList )
		printf("LL Table:\n");
	else
		printf("LL(1) Table:\n");
	for ( x = 0; x < pTables->countNonTerminals; x++ )
	{
		for ( y = 0; y < pTables->countTerminals + 1; y++ )
		{
			if ( pTables->tableLL[x][y] != NULL )
			{
				pProdList = pTables->tableLL[x][y];
				if ( y < pTables->countTerminals )
					printf("Table[%s, %s] = \n\t", pTables->aNonTerminals[x].strLexeme, pTables->aTerminals[y].strLexeme);
				else
					printf("Table[%s, $] = \n\t", pTables->aNonTerminals[x].strLexeme);

				while ( pProdList != NULL )
				{
					for ( z = 0; z < pTables->aProds[pProdList->idProd].count; z++ )
					{
						printf("%s ", pTables->aProds[pProdList->idProd].aSymbols[z].strLexeme);
						if ( z == 0 )
						{
							printf("-> ");
						}
					}
					if ( pProdList->next != NULL )
						printf("\n\t");
					pProdList = pProdList->next;
				}
				printf("\n");
			}
		}
	}

	printf("\n");

	if ( pTables->problemList )
	{
		printf("The grammar is not LL(1).\n");
		printf("The following table entries contain more than one production:\n\n");

		pProblemList = pTables->problemList;
		while ( pProblemList != NULL )
		{
			if ( pProblemList->idTerminal != T_DOLLAR )
			{
				if ( pProblemList->idTerminal >= pTables->countTerminals )
				{
					printf("Table[%s, $]\n", pTables->aNonTerminals[pProblemList->idNonTerminal].strLexeme);
				}
				else
				{
					printf("Table[%s, %s]\n",
						pTables->aNonTerminals[pProblemList->idNonTerminal].strLexeme,
						pTables->aTerminals[pProblemList->idTerminal].strLexeme);
				}
			}
			else
			{
				printf("Table[%s, $]\n",
					pTables->aNonTerminals[pProblemList->idNonTerminal].strLexeme);
			}
			pProblemList = pProblemList->next;
		}
	}

	printf("\n");
}

void Raggruppa(Tables *pTables)
{
	int x, y, z;
	char *strSym;

	ArrayProduction tempProd;

	int temp1, temp2;

	for(x = 0; x < pTables->countProds - 1; x++)
	{
		strSym = pTables->aProds[x].aSymbols[0].strLexeme;
		if ( strcmp(pTables->aProds[x + 1].aSymbols[0].strLexeme, strSym) == 0 )
		{
			x++;
			continue;
		}

		temp1 = x + 1;

		for(y = temp1; y < pTables->countProds; y++)
		{
			if ( strcmp(pTables->aProds[y].aSymbols[0].strLexeme, strSym) == 0 )
			{
				temp2 = y;

				tempProd = pTables->aProds[y];
				
				for(z = temp2; z > temp1; z--)
				{
					pTables->aProds[z] = pTables->aProds[z - 1];
				}

				pTables->aProds[temp1] = tempProd;
			}
		}
	}
}

int FindInStack(int id, int *pStack, int top)
{
	int x;

	for ( x = 0; x <= top; x++ )
	{
		if ( id == pStack[x] )
			return 1;
	}

	return 0;
}

int Compare(Item a, Item b) // Comparison function for QuickSort
{
	// The comparison function must return a value
	// less than zero if a < b,
	// greater than zero if a > b,
	// and zero if a and b are equal

	if ( a.aSymbols[0].id < b.aSymbols[0].id )
		return -1;
	else if ( a.aSymbols[0].id > b.aSymbols[0].id )
		return 1;
	else
		return 0;
}

void QuickSortNonRecursive(Item *a, int start, int stop)
{
	int i, s = 0, stack[128];
	int up, down;
	Item part, tmp;

	if (!a || start >= stop)
		return;

	stack[s++] = start;
	stack[s++] = stop;

	while (s > 0)
	{
		stop = stack[--s];
		start = stack[--s];

		if (start >= stop)
			continue;

		up = start;
		down = stop - 1;
		part = a[stop];

		if (stop <= start)
		{
			i = start;
			continue;
		}

		while (1)
		{
			while ( (*pfnCompare)(a[up], part) < 0 )
				up++;

			while ( (*pfnCompare)(part, a[down]) < 0 && (up < down) )
				down--;

			if (up >= down)
				break;

			if ( a[down].aSymbols[0].id != a[up].aSymbols[0].id )
			{
				tmp = a[down];
				a[down] = a[up];
				a[up] = tmp;
			}

			up++;
			down--;
		}

		if ( a[stop].aSymbols[0].id != a[up].aSymbols[0].id )
		{
			tmp = a[stop];
			a[stop] = a[up];
			a[up] = tmp;
		}

		i = up;

		// END OF PARTITIONING

		if (i - start > stop - i)
		{
			stack[s++] = start;
			stack[s++] = i - 1;
			stack[s++] = i + 1;
			stack[s++] = stop;
		}
		else
		{
			stack[s++] = i + 1;
			stack[s++] = stop;
			stack[s++] = start;
			stack[s++] = i - 1;
		}
	}
}

void QSort(Item * a, int nCount)
{
	QuickSortNonRecursive(a, 0, nCount - 1);
}

void Ordina(Tables *pTables)
{
	int x, y, z;
	int *pQueue;
	int *pVisited;
	int *pOldToNew;
	Symbol *pSortedNT;
	ArrayProduction *pSortedProds;

	if (!pTables || pTables->countProds <= 0 || pTables->countNonTerminals <= 0)
		return;

	int nNT = pTables->countNonTerminals;
	pQueue = (int *)malloc((size_t)nNT * sizeof(int));
	pVisited = (int *)calloc((size_t)nNT, sizeof(int));
	pOldToNew = (int *)malloc((size_t)nNT * sizeof(int));
	pSortedNT = (Symbol *)malloc((size_t)nNT * sizeof(Symbol));
	if (!pQueue || !pVisited || !pOldToNew || !pSortedNT)
	{
		free(pQueue); free(pVisited); free(pOldToNew); free(pSortedNT);
		printf("Sort function: Insufficient memory!\n");
		return;
	}

	int qHead = 0, qTail = 0;
	int startId = pTables->aProds[0].aSymbols[0].id;
	pQueue[qTail++] = startId;
	pVisited[startId] = 1;

	while (qHead < qTail)
	{
		int curId = pQueue[qHead++];
		for (x = 0; x < pTables->countProds; x++)
		{
			if (pTables->aProds[x].aSymbols[0].id == curId)
			{
				for (y = 1; y < pTables->aProds[x].count; y++)
				{
					if (pTables->aProds[x].aSymbols[y].Type == T_NONTERMINAL)
					{
						int ntId = pTables->aProds[x].aSymbols[y].id;
						if (ntId >= 0 && ntId < nNT && !pVisited[ntId])
						{
							pVisited[ntId] = 1;
							pQueue[qTail++] = ntId;
						}
					}
				}
			}
		}
	}

	for (x = 0; x < nNT; x++)
	{
		if (!pVisited[x])
		{
			pVisited[x] = 1;
			pQueue[qTail++] = x;
		}
	}

	for (x = 0; x < nNT; x++)
	{
		int oldId = pQueue[x];
		pOldToNew[oldId] = x;
		pSortedNT[x] = pTables->aNonTerminals[oldId];
		pSortedNT[x].id = x;
	}
	memcpy(pTables->aNonTerminals, pSortedNT, (size_t)nNT * sizeof(Symbol));

	for (y = 0; y < pTables->countProds; y++)
	{
		for (z = 0; z < pTables->aProds[y].count; z++)
		{
			if (pTables->aProds[y].aSymbols[z].Type == T_NONTERMINAL)
			{
				pTables->aProds[y].aSymbols[z].id =
					pOldToNew[pTables->aProds[y].aSymbols[z].id];
			}
		}
	}

	pSortedProds = (ArrayProduction *)malloc((size_t)pTables->countProds * sizeof(ArrayProduction));
	if (pSortedProds)
	{
		int pos = 0;
		for (x = 0; x < nNT; x++)
		{
			for (y = 0; y < pTables->countProds; y++)
			{
				if (pTables->aProds[y].aSymbols[0].id == x)
					pSortedProds[pos++] = pTables->aProds[y];
			}
		}
		memcpy(pTables->aProds, pSortedProds, (size_t)pTables->countProds * sizeof(ArrayProduction));
		free(pSortedProds);
	}

	pTables->startSymbolId = 0;

	free(pSortedNT);
	free(pOldToNew);
	free(pVisited);
	free(pQueue);
}

int IsNullable(Tables *pTables, char *strSym)
{
	int x;

	x = 0;
	while ( x < pTables->countNonTerminals )
	{
		if ( strcmp(pTables->aNonTerminals[x].strLexeme, strSym) == 0 )
		{
			return pTables->aNonTerminals[x].bNullable;
		}
		x++;
	}

	return 0;
}

void MakeNullableSet(Tables *pTables)
{
	int x, y;
	int changed;

	for (x = 0; x < pTables->countTerminals; x++)
		pTables->aTerminals[x].bNullable = 0;

	for (x = 0; x < pTables->countNonTerminals; x++)
		pTables->aNonTerminals[x].bNullable = 0;

	do {
		changed = 0;
		for (x = 0; x < pTables->countProds; x++)
		{
			int lhsId = pTables->aProds[x].aSymbols[0].id;
			if (pTables->aNonTerminals[lhsId].bNullable)
				continue;

			int allNullable = 1;
			for (y = 1; y < pTables->aProds[x].count; y++)
			{
				if (pTables->aProds[x].aSymbols[y].Type == T_TERMINAL ||
				    !pTables->aNonTerminals[pTables->aProds[x].aSymbols[y].id].bNullable)
				{
					allNullable = 0;
					break;
				}
			}
			if (allNullable)
			{
				pTables->aNonTerminals[lhsId].bNullable = 1;
				changed = 1;
			}
		}
	} while (changed);

	for (x = 0; x < pTables->countProds; x++)
	{
		for (y = 0; y < pTables->aProds[x].count; y++)
		{
			if (pTables->aProds[x].aSymbols[y].Type == T_TERMINAL)
				pTables->aProds[x].aSymbols[y].bNullable = 0;
			else
				pTables->aProds[x].aSymbols[y].bNullable =
					pTables->aNonTerminals[pTables->aProds[x].aSymbols[y].id].bNullable;
		}
	}
}

int MutuallyRecursive(Tables *pTables)
{
	// A : B A a
	//  | ;

	// B : b B c
	//  | A A ;

    // Mutual recursion complicates our task... 

	int index1, index2;
	int count;

	count = 0;

	for ( index1 = 0; index1 < pTables->countProds - 1; index1++ )
	{
		if ( pTables->aProds[index1].count == 1 )
			continue;

		if ( pTables->aProds[index1].aSymbols[1].Type != T_NONTERMINAL )
			continue;

		for ( index2 = index1 + 1; index2 < pTables->countProds; index2++ )
		{
			if ( pTables->aProds[index2].count == 1 )
				continue;

			if ( pTables->aProds[index2].aSymbols[1].Type != T_NONTERMINAL )
				continue;

			if ( pTables->aProds[index1].aSymbols[0].id == pTables->aProds[index2].aSymbols[1].id 
				&&
				pTables->aProds[index2].aSymbols[0].id == pTables->aProds[index1].aSymbols[1].id )
			{
				count++;
			}
		}
	}

	return count;
}

void MakeFirstSets(Tables *pTables)
{
	int x, y;
	int nNT = pTables->countNonTerminals;
	int nT = pTables->countTerminals;
	if (nNT <= 0) return;

	size_t stride = (size_t)nT + 2; /* [0..nT-1]=terminals, nT=EMPTY, nT+1=DOLLAR */
	int emptyCol = nT;
	unsigned char *hasFirst = (unsigned char *)calloc((size_t)nNT * stride, sizeof(unsigned char));
	if (!hasFirst)
	{
		printf("Insufficient memory to allocate FIRST matrix!\n");
		return;
	}

	int changed;
	do {
		changed = 0;
		for (x = 0; x < pTables->countProds; x++)
		{
			int lhs = pTables->aProds[x].aSymbols[0].id;
			if (pTables->aProds[x].count == 1)
			{
				if (!hasFirst[(size_t)lhs * stride + (size_t)emptyCol])
				{
					TerminalList tl = { T_EMPTY, NULL };
					pTables->aNonTerminals[lhs].pFirstSet =
						AppendTerminalList(&tl, pTables->aNonTerminals[lhs].pFirstSet, 0);
					hasFirst[(size_t)lhs * stride + (size_t)emptyCol] = 1;
					changed = 1;
				}
				continue;
			}

			int allNullable = 1;
			for (y = 1; y < pTables->aProds[x].count; y++)
			{
				Symbol sym = pTables->aProds[x].aSymbols[y];
				if (sym.Type == T_TERMINAL)
				{
					if (!hasFirst[(size_t)lhs * stride + (size_t)sym.id])
					{
						TerminalList tl = { sym.id, NULL };
						pTables->aNonTerminals[lhs].pFirstSet =
							AppendTerminalList(&tl, pTables->aNonTerminals[lhs].pFirstSet, 1);
						hasFirst[(size_t)lhs * stride + (size_t)sym.id] = 1;
						changed = 1;
					}
					allNullable = 0;
					break;
				}
				else
				{
					TerminalList *src = pTables->aNonTerminals[sym.id].pFirstSet;
					while (src != NULL)
					{
						if (src->idSymbol >= 0 && src->idSymbol < nT &&
						    !hasFirst[(size_t)lhs * stride + (size_t)src->idSymbol])
						{
							pTables->aNonTerminals[lhs].pFirstSet =
								AppendTerminalList(src, pTables->aNonTerminals[lhs].pFirstSet, 1);
							hasFirst[(size_t)lhs * stride + (size_t)src->idSymbol] = 1;
							changed = 1;
						}
						src = src->next;
					}
					if (!pTables->aNonTerminals[sym.id].bNullable)
					{
						allNullable = 0;
						break;
					}
				}
			}

			if (allNullable && !hasFirst[(size_t)lhs * stride + (size_t)emptyCol])
			{
				TerminalList tl = { T_EMPTY, NULL };
				pTables->aNonTerminals[lhs].pFirstSet =
					AppendTerminalList(&tl, pTables->aNonTerminals[lhs].pFirstSet, 0);
				hasFirst[(size_t)lhs * stride + (size_t)emptyCol] = 1;
				changed = 1;
			}
		}
	} while (changed);

	free(hasFirst);
}

void MakeFollowSets(Tables *pTables)
{
	int x, z, w;
	int nNT = pTables->countNonTerminals;
	int nT = pTables->countTerminals;
	if (nNT <= 0) return;

	size_t stride = (size_t)nT + 1; /* [0..nT-1]=terminals, nT=DOLLAR */
	int dollarCol = nT;
	unsigned char *hasFollow = (unsigned char *)calloc((size_t)nNT * stride, sizeof(unsigned char));
	if (!hasFollow)
	{
		printf("Insufficient memory to allocate FOLLOW matrix!\n");
		return;
	}

	int startId = (pTables->startSymbolId >= 0 && pTables->startSymbolId < nNT)
	              ? pTables->startSymbolId : 0;
	if (pTables->countProds > 0)
	{
		TerminalList tl = { T_DOLLAR, NULL };
		pTables->aNonTerminals[startId].pFollowSet =
			AppendTerminalList(&tl, pTables->aNonTerminals[startId].pFollowSet, 1);
		hasFollow[(size_t)startId * stride + (size_t)dollarCol] = 1;
	}

	int changed;
	do {
		changed = 0;
		for (x = 0; x < pTables->countProds; x++)
		{
			int lhs = pTables->aProds[x].aSymbols[0].id;
			for (z = 1; z < pTables->aProds[x].count; z++)
			{
				if (pTables->aProds[x].aSymbols[z].Type != T_NONTERMINAL)
					continue;

				int B = pTables->aProds[x].aSymbols[z].id;
				int suffixNullable = 1;

				for (w = z + 1; w < pTables->aProds[x].count; w++)
				{
					Symbol sym = pTables->aProds[x].aSymbols[w];
					if (sym.Type == T_TERMINAL)
					{
						if (!hasFollow[(size_t)B * stride + (size_t)sym.id])
						{
							TerminalList tl = { sym.id, NULL };
							pTables->aNonTerminals[B].pFollowSet =
								AppendTerminalList(&tl, pTables->aNonTerminals[B].pFollowSet, 1);
							hasFollow[(size_t)B * stride + (size_t)sym.id] = 1;
							changed = 1;
						}
						suffixNullable = 0;
						break;
					}
					else
					{
						TerminalList *src = pTables->aNonTerminals[sym.id].pFirstSet;
						while (src != NULL)
						{
							if (src->idSymbol >= 0 && src->idSymbol < nT &&
							    !hasFollow[(size_t)B * stride + (size_t)src->idSymbol])
							{
								pTables->aNonTerminals[B].pFollowSet =
									AppendTerminalList(src, pTables->aNonTerminals[B].pFollowSet, 1);
								hasFollow[(size_t)B * stride + (size_t)src->idSymbol] = 1;
								changed = 1;
							}
							src = src->next;
						}
						if (!pTables->aNonTerminals[sym.id].bNullable)
						{
							suffixNullable = 0;
							break;
						}
					}
				}

				if (suffixNullable && lhs != B)
				{
					TerminalList *src = pTables->aNonTerminals[lhs].pFollowSet;
					while (src != NULL)
					{
						int col = (src->idSymbol == T_DOLLAR) ? dollarCol : src->idSymbol;
						if (col >= 0 && col <= nT &&
						    !hasFollow[(size_t)B * stride + (size_t)col])
						{
							pTables->aNonTerminals[B].pFollowSet =
								AppendTerminalList(src, pTables->aNonTerminals[B].pFollowSet, 1);
							hasFollow[(size_t)B * stride + (size_t)col] = 1;
							changed = 1;
						}
						src = src->next;
					}
				}
			}
		}
	} while (changed);

	free(hasFollow);
}

static void AddLLTableEntry(Tables *pTables, int ntId, int termCol, int prodId)
{
	ProdsList *cur = pTables->tableLL[ntId][termCol];
	while (cur != NULL)
	{
		if (cur->idProd == prodId)
			return;
		cur = cur->next;
	}

	ProdsList pl = { prodId, NULL };
	pTables->tableLL[ntId][termCol] = AppendProdsList(&pl, pTables->tableLL[ntId][termCol]);
	ProdsList *head = pTables->tableLL[ntId][termCol];
	if (head && head->next != NULL && head->next->next == NULL)
	{
		ProblemList prob = { ntId, termCol, NULL };
		pTables->problemList = AppendProblemList(&prob, pTables->problemList);
	}
}

void MakeTableLL(Tables *pTables)
{
	int x, y;
	int rows = pTables->countNonTerminals;
	int columns = pTables->countTerminals + 1;

	pTables->problemList = NULL;
	pTables->tableLL = (ProdsList ***)malloc(sizeof(ProdsList **) * (size_t)rows);
	if (!pTables->tableLL)
	{
		printf("Insufficient memory to allocate the table rows LL!\n");
		return;
	}

	for (x = 0; x < rows; x++)
	{
		pTables->tableLL[x] = (ProdsList **)calloc((size_t)columns, sizeof(ProdsList *));
		if (!pTables->tableLL[x])
		{
			for (int k = 0; k < x; k++) free(pTables->tableLL[k]);
			free(pTables->tableLL);
			pTables->tableLL = NULL;
			printf("Insufficient memory to allocate the columns from Table LL!\n");
			return;
		}
	}

	for (x = 0; x < pTables->countProds; x++)
	{
		int lhs = pTables->aProds[x].aSymbols[0].id;
		int bNullable = 1;

		for (y = 1; y < pTables->aProds[x].count; y++)
		{
			Symbol sym = pTables->aProds[x].aSymbols[y];
			if (sym.Type == T_TERMINAL)
			{
				AddLLTableEntry(pTables, lhs, sym.id, x);
				bNullable = 0;
				break;
			}
			else
			{
				TerminalList *fl = pTables->aNonTerminals[sym.id].pFirstSet;
				while (fl != NULL)
				{
					if (fl->idSymbol >= 0 && fl->idSymbol < pTables->countTerminals)
						AddLLTableEntry(pTables, lhs, fl->idSymbol, x);
					fl = fl->next;
				}
				if (!pTables->aNonTerminals[sym.id].bNullable)
				{
					bNullable = 0;
					break;
				}
			}
		}

		if (bNullable)
		{
			TerminalList *flw = pTables->aNonTerminals[lhs].pFollowSet;
			while (flw != NULL)
			{
				int idT = flw->idSymbol;
				if (idT == T_DOLLAR)
					AddLLTableEntry(pTables, lhs, pTables->countTerminals, x);
				else if (idT >= 0 && idT < pTables->countTerminals)
					AddLLTableEntry(pTables, lhs, idT, x);
				flw = flw->next;
			}
		}
	}
}

void MakeHtmlFile(Tables *pTables, const char *szFileName)
{
	int x, y, z;
	int bDollar, bEmpty;

	ProdsList *pProdList;
	TerminalList *pTermList;
	ProblemList *pProblemList;

	FILE *fp = fopen(szFileName, "wt");
	if ( fp == NULL )
	{
		printf("Error opening file %s.\n", szFileName);
		return;
	}

	fprintf(fp,
		"<!DOCTYPE html>\n"
		"<html lang=\"en\">\n"
		"<head>\n"
		"<meta charset=\"UTF-8\">\n"
		"<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
		"<title>LL(1) Grammar Tools - Vincenzo Lo Cicero</title>\n"
		"<style>\n"
		"body { font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, Helvetica, Arial, sans-serif; background-color: #f4f6f9; color: #1e293b; margin: 0; padding: 24px; line-height: 1.5; }\n"
		".container { max-width: 1200px; margin: 0 auto; }\n"
		"h1 { font-size: 1.6rem; color: #0f172a; margin-bottom: 24px; border-bottom: 2px solid #cbd5e1; padding-bottom: 10px; }\n"
		".card { background: #ffffff; border-radius: 8px; box-shadow: 0 1px 3px rgba(0,0,0,0.1); padding: 20px 24px; margin-bottom: 24px; overflow-x: auto; }\n"
		".card h2 { font-size: 1.15rem; color: #334155; margin-top: 0; margin-bottom: 14px; border-left: 4px solid #2563eb; padding-left: 10px; }\n"
		".symbols-grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(280px, 1fr)); gap: 20px; }\n"
		"ul.badge-list { list-style: none; padding: 0; margin: 0; display: flex; flex-wrap: wrap; gap: 8px; }\n"
		"ul.badge-list li { font-family: 'Consolas', 'Monaco', monospace; background: #e2e8f0; color: #0f172a; padding: 4px 10px; border-radius: 6px; font-size: 0.9rem; font-weight: 600; }\n"
		"ul.badge-list.term li { background: #dbeafe; color: #1e40af; }\n"
		"table { border-collapse: collapse; width: 100%%; font-size: 0.92rem; }\n"
		"table.prods { width: auto; min-width: 320px; }\n"
		"table.prods td { padding: 5px 10px; border-bottom: 1px solid #f1f5f9; font-family: 'Consolas', 'Monaco', monospace; }\n"
		"table.prods td.lhs { font-weight: bold; color: #0f172a; text-align: right; }\n"
		"table.prods td.arrow { color: #64748b; text-align: center; }\n"
		"table.prods td.rhs { color: #1d4ed8; }\n"
		"table.grid-table th, table.grid-table td { border: 1px solid #cbd5e1; padding: 8px 12px; }\n"
		"table.grid-table th { background-color: #1e293b; color: #ffffff; font-weight: 600; text-align: center; }\n"
		"table.grid-table tr:nth-child(even) { background-color: #f8fafc; }\n"
		"table.grid-table tr:hover { background-color: #f1f5f9; }\n"
		".mono { font-family: 'Consolas', 'Monaco', monospace; }\n"
		".nt-col { font-weight: bold; background-color: #f1f5f9; color: #0f172a; white-space: nowrap; }\n"
		".yes { color: #15803d; font-weight: bold; }\n"
		".no { color: #64748b; }\n"
		".cell-prod { background-color: #eff6ff; color: #1e3a8a; font-family: 'Consolas', 'Monaco', monospace; font-weight: 600; white-space: nowrap; }\n"
		".cell-conflict { background-color: #fef2f2; color: #b91c1c; font-family: 'Consolas', 'Monaco', monospace; font-weight: bold; white-space: nowrap; }\n"
		".cell-error { color: #94a3b8; font-style: italic; text-align: center; font-size: 0.85rem; }\n"
		".alert-error { background-color: #fef2f2; border: 1px solid #fecaca; border-left: 5px solid #dc2626; color: #991b1b; padding: 16px 20px; border-radius: 8px; margin-bottom: 24px; }\n"
		".alert-error h3 { margin-top: 0; font-size: 1.1rem; }\n"
		"</style>\n"
		"</head>\n"
		"<body>\n"
		"<div class=\"container\">\n"
		"<h1>LL(1) Grammar Tools &mdash; Vincenzo Lo Cicero</h1>\n"
	);

	// ---------------------------- Start Symbols ---------------------------------
	fprintf(fp, "<div class=\"symbols-grid\">\n");

	fprintf(fp, "<div class=\"card\">\n<h2>%d Non-Terminal Symbols</h2>\n<ul class=\"badge-list\">\n", pTables->countNonTerminals);
	for (x = 0; x < pTables->countNonTerminals; x++)
	{
		fprintf(fp, "<li>");
		PrintHtmlEscaped(fp, pTables->aNonTerminals[x].strLexeme);
		fprintf(fp, "</li>\n");
	}
	fprintf(fp, "</ul>\n</div>\n");

	fprintf(fp, "<div class=\"card\">\n<h2>%d Terminal Symbols</h2>\n<ul class=\"badge-list term\">\n", pTables->countTerminals);
	for (x = 0; x < pTables->countTerminals; x++)
	{
		fprintf(fp, "<li>");
		PrintHtmlEscaped(fp, pTables->aTerminals[x].strLexeme);
		fprintf(fp, "</li>\n");
	}
	fprintf(fp, "</ul>\n</div>\n");

	fprintf(fp, "</div>\n");
	// ---------------------------- End Symbols ---------------------------------

	// ---------------------------- Start Productions ---------------------------------
	fprintf(fp, "<div class=\"card\">\n<h2>%d Productions</h2>\n<table class=\"prods\">\n", pTables->countProds);

	for ( x = 0; x < pTables->countProds; x++ )
	{
		fprintf(fp, "<tr>\n");
		for ( y = 0; y < pTables->aProds[x].count; y++ )
		{
			if ( y == 0 )
			{
				fprintf(fp, "<td class=\"lhs\">");
				PrintHtmlEscaped(fp, pTables->aProds[x].aSymbols[y].strLexeme);
				fprintf(fp, "</td>\n");
				fprintf(fp, "<td class=\"arrow\">&rarr;</td>\n");
				fprintf(fp, "<td class=\"rhs\">");

				if ( pTables->aProds[x].count == 1 )
				{
					fprintf(fp, "&epsilon;");
				}
			}
			else
			{
				PrintHtmlEscaped(fp, pTables->aProds[x].aSymbols[y].strLexeme);
				fputc(' ', fp);
			}
		}
		fprintf(fp, "</td>\n  </tr>\n");
	}
	fprintf(fp, "</table>\n</div>\n");
	// ---------------------------- End Productions ---------------------------------

	// ---------------------------- Start First Follow Sets ---------------------------------
	fprintf(fp,
		"<div class=\"card\">\n"
		"<h2>First and Follow Sets</h2>\n"
		"<table class=\"grid-table\">\n"
		"<thead>\n"
		"<tr>\n"
		"<th>Non-Terminal</th>\n"
		"<th>NULLABLE</th>\n"
		"<th>FIRST</th>\n"
		"<th>FOLLOW</th>\n"
		"</tr>\n"
		"</thead>\n"
		"<tbody>\n"
	);

	for ( x = 0; x < pTables->countNonTerminals; x++ )
	{
		fprintf(fp, "<tr>\n");
		fprintf(fp, "<td class=\"nt-col mono\">");
		PrintHtmlEscaped(fp, pTables->aNonTerminals[x].strLexeme);
		fprintf(fp, "</td>\n");

		if ( pTables->aNonTerminals[x].bNullable )
			fprintf(fp, "<td align=\"center\" class=\"yes\">Yes</td>\n");
		else
			fprintf(fp, "<td align=\"center\" class=\"no\">No</td>\n");

		fprintf(fp, "<td class=\"mono\">");
		bEmpty = 0;
		pTermList = pTables->aNonTerminals[x].pFirstSet;
		while ( pTermList != NULL )
		{
			if ( pTermList->idSymbol == T_EMPTY )
				bEmpty = 1;
			else
			{
				PrintHtmlEscaped(fp, pTables->aTerminals[pTermList->idSymbol].strLexeme);
				fputc(' ', fp);
			}
			pTermList = pTermList->next;
		}
		if ( bEmpty )
			fprintf(fp, "&epsilon;");
		fprintf(fp, "</td>\n");

		fprintf(fp, "<td class=\"mono\">");
		bDollar = 0;
		pTermList = pTables->aNonTerminals[x].pFollowSet;
		while ( pTermList != NULL )
		{
			if ( pTermList->idSymbol == T_DOLLAR )
				bDollar = 1;
			else
			{
				PrintHtmlEscaped(fp, pTables->aTerminals[pTermList->idSymbol].strLexeme);
				fputc(' ', fp);
			}
			pTermList = pTermList->next;
		}
		if ( bDollar )
			fprintf(fp, "$ ");
		fprintf(fp, "</td>\n");

		fprintf(fp, "</tr>\n");
	}
	fprintf(fp, "</tbody>\n</table>\n</div>\n");
	// ---------------------------- End First Follow Sets ----------------------------------

	// ---------------------------- Start Problems List ------------------------------------
	if ( pTables->problemList )
	{
		fprintf(fp,
			"<div class=\"alert-error\">\n"
			"<h3>Warning: The grammar is NOT LL(1)</h3>\n"
			"<p>The following table entries contain more than one production (conflicts):</p>\n"
			"<ul class=\"mono\">\n"
		);

		pProblemList = pTables->problemList;
		while ( pProblemList != NULL )
		{
			fprintf(fp, "<li>Table[");
			PrintHtmlEscaped(fp, pTables->aNonTerminals[pProblemList->idNonTerminal].strLexeme);
			fprintf(fp, ", ");
			if ( pProblemList->idTerminal != T_DOLLAR && pProblemList->idTerminal < pTables->countTerminals )
				PrintHtmlEscaped(fp, pTables->aTerminals[pProblemList->idTerminal].strLexeme);
			else
				fputc('$', fp);
			fprintf(fp, "]</li>\n");
			pProblemList = pProblemList->next;
		}
		fprintf(fp, "</ul>\n</div>\n");
	}
	// ---------------------------- End Problems List -------------------------------------

	// ---------------------------- Start LL(1) Table ---------------------------------
	fprintf(fp, "<div class=\"card\">\n");
	if ( pTables->problemList )
		fprintf(fp, "<h2>LL Table (with conflicts)</h2>\n");
	else
		fprintf(fp, "<h2>LL(1) Parsing Table</h2>\n");

	fprintf(fp, "<table class=\"grid-table\">\n  <thead>\n    <tr>\n      <th>Non-Terminal</th>\n");
	for ( x = 0; x < pTables->countTerminals + 1; x++ )
	{
		fprintf(fp, "<th class=\"mono\">");
		if ( x < pTables->countTerminals )
			PrintHtmlEscaped(fp, pTables->aTerminals[x].strLexeme);
		else
			fputc('$', fp);
		fprintf(fp, "</th>\n");
	}
	fprintf(fp, "    </tr>\n  </thead>\n  <tbody>\n");

	for ( x = 0; x < pTables->countNonTerminals; x++ )
	{
		fprintf(fp, "<tr>\n");
		fprintf(fp, "<td class=\"nt-col mono\">");
		PrintHtmlEscaped(fp, pTables->aNonTerminals[x].strLexeme);
		fprintf(fp, "</td>\n");

		for ( y = 0; y < pTables->countTerminals + 1; y++ )
		{
			if ( pTables->tableLL[x][y] != NULL )
			{
				pProdList = pTables->tableLL[x][y];
				if ( pProdList->next != NULL )
					fprintf(fp, "<td class=\"cell-conflict\">");
				else
					fprintf(fp, "<td class=\"cell-prod\">");

				while ( pProdList != NULL )
				{
					for ( z = 0; z < pTables->aProds[pProdList->idProd].count; z++ )
					{
						PrintHtmlEscaped(fp, pTables->aProds[pProdList->idProd].aSymbols[z].strLexeme);
						if ( z == 0 )
						{
							fprintf(fp, " &rarr; ");
							if ( pTables->aProds[pProdList->idProd].count == 1 )
								fprintf(fp, "&epsilon;");
						}
						else
						{
							fputc(' ', fp);
						}
					}
					if ( pProdList->next != NULL )
						fprintf(fp, "<br>");
					pProdList = pProdList->next;
				}
				fprintf(fp, "</td>\n");
			}
			else
			{
				fprintf(fp, "<td class=\"cell-error\">error</td>\n");
			}
		}
		fprintf(fp, "</tr>\n");
	}

	fprintf(fp, "</tbody>\n</table>\n</div>\n");
	fprintf(fp, "\n\n\n");
	fclose(fp);
}

#define NT_OFFSET 1000

static int AppendStrBuf(char **buf, size_t *len, size_t *cap, const char *s, size_t slen)
{
	while (*len + slen + 1 > *cap)
	{
		size_t newCap = (*cap) * 2;
		char *tmp = (char *)realloc(*buf, newCap);
		if (!tmp) return 0;
		*buf = tmp;
		*cap = newCap;
	}
	memcpy(*buf + *len, s, slen);
	*len += slen;
	(*buf)[*len] = '\0';
	return 1;
}

char* TranslateSemanticAction(const char *rawCode, int rhsLen, const char *lhsExpr, const char *rhsFmt)
{
	if (!rawCode) return NULL;
	size_t cap = strlen(rawCode) * 2 + 128, len = 0;
	char *out = (char *)malloc(cap);
	if (!out) return NULL;
	out[0] = '\0';

	int inStr = 0, inChar = 0, inLineCmt = 0, inBlkCmt = 0;
	const char *p = rawCode;
	size_t lhsLen = strlen(lhsExpr);

	while (*p != '\0')
	{
		if (inLineCmt) { if (*p == '\n') inLineCmt = 0; if (!AppendStrBuf(&out, &len, &cap, p++, 1)) { free(out); return NULL; } continue; }
		if (inBlkCmt) { if (*p == '*' && *(p+1) == '/') { if (!AppendStrBuf(&out, &len, &cap, p, 2)) { free(out); return NULL; } p += 2; inBlkCmt = 0; continue; } if (!AppendStrBuf(&out, &len, &cap, p++, 1)) { free(out); return NULL; } continue; }
		if (inStr) { if (*p == '\\' && *(p+1)) { if (!AppendStrBuf(&out, &len, &cap, p, 2)) { free(out); return NULL; } p += 2; continue; } if (*p == '"') inStr = 0; if (!AppendStrBuf(&out, &len, &cap, p++, 1)) { free(out); return NULL; } continue; }
		if (inChar) { if (*p == '\\' && *(p+1)) { if (!AppendStrBuf(&out, &len, &cap, p, 2)) { free(out); return NULL; } p += 2; continue; } if (*p == '\'') inChar = 0; if (!AppendStrBuf(&out, &len, &cap, p++, 1)) { free(out); return NULL; } continue; }
		if (*p == '/' && *(p+1) == '/') { inLineCmt = 1; if (!AppendStrBuf(&out, &len, &cap, p, 2)) { free(out); return NULL; } p += 2; continue; }
		if (*p == '/' && *(p+1) == '*') { inBlkCmt = 1; if (!AppendStrBuf(&out, &len, &cap, p, 2)) { free(out); return NULL; } p += 2; continue; }
		if (*p == '"') { inStr = 1; if (!AppendStrBuf(&out, &len, &cap, p++, 1)) { free(out); return NULL; } continue; }
		if (*p == '\'') { inChar = 1; if (!AppendStrBuf(&out, &len, &cap, p++, 1)) { free(out); return NULL; } continue; }

		if (*p == '$')
		{
			if (*(p+1) == '$')
			{
				if (!AppendStrBuf(&out, &len, &cap, lhsExpr, lhsLen)) { free(out); return NULL; }
				p += 2;
				continue;
			}
			if ((*(p+1) >= '0' && *(p+1) <= '9') ||
			    (*(p+1) == '-' && *(p+2) >= '0' && *(p+2) <= '9'))
			{
				char *endp = NULL;
				long idx = strtol(p + 1, &endp, 10);
				char tmp[128];
				if (idx >= 1 && idx <= rhsLen)
				{
					int n = snprintf(tmp, sizeof(tmp), rhsFmt, (int)idx);
					if (n < 0 || !AppendStrBuf(&out, &len, &cap, tmp, (size_t)n)) { free(out); return NULL; }
				}
				else
				{
					fprintf(stderr, "Warning: semantic placeholder $%ld out of range (1..%d).\n", idx, rhsLen);
					int n = snprintf(tmp, sizeof(tmp), "((YYSTYPE)0 /* invalid $%ld */)", idx);
					if (n < 0 || !AppendStrBuf(&out, &len, &cap, tmp, (size_t)n)) { free(out); return NULL; }
				}
				p = endp;
				continue;
			}
		}
		if (!AppendStrBuf(&out, &len, &cap, p++, 1)) { free(out); return NULL; }
	}
	return out;
}

void MakeCFile(Tables *pTables, const char *szFileName)
{
	int x, y, numTerms;

	if (!pTables || pTables->countProds <= 0 || pTables->countNonTerminals <= 0)
	{
		printf("Impossibile generare il parser C: grammatica vuota.\n");
		return;
	}

	numTerms = pTables->countTerminals + 1;
	int startNt = (pTables->startSymbolId >= 0 && pTables->startSymbolId < pTables->countNonTerminals)
	              ? pTables->startSymbolId : 0;

	FILE *fp = fopen(szFileName, "wt");
	if (!fp) { printf("Errore nell'apertura del file %s.\n", szFileName); return; }

	fprintf(fp, "/* Parser LL(1) con azioni semantiche - vlcGrammarTools */\n#include <stdio.h>\n#include <stdlib.h>\n\n");
	if (pTables->strPrologue && *pTables->strPrologue)
		fprintf(fp, "/* --- Prologo --- */\n%s\n\n", pTables->strPrologue);

	fprintf(fp, "#ifndef YYSTYPE\n#define YYSTYPE double\n#endif\n#ifndef YYSCAN_T\ntypedef void *yyscan_t;\n#endif\nint yylex(YYSTYPE *yylval_param, yyscan_t yyscanner);\nvoid yyerror(yyscan_t yyscanner, const char *s);\n\n");
	fprintf(fp, "#define LL_NUM_NONTERM %d\n#define LL_NUM_TERMINALS %d\n#define LL_NUM_PRODUCTIONS %d\n#define LL_TOKEN_DOLLAR %d\n#define LL_SYM_NT_BASE 10000\n#define LL_SYM_ACTION_BASE 20000\n\n",
		pTables->countNonTerminals, numTerms, pTables->countProds, pTables->countTerminals);

	fprintf(fp, "enum LlTokens {\n");
	for (int t = 0; t < pTables->countTerminals; t++)
	{
		const char *lex = pTables->aTerminals[t].strLexeme;
		if (IsValidCIdent(lex) && strcmp(lex, "EOF") != 0 && strncmp(lex, "SYM_", 4) != 0)
		{
			fprintf(fp, "\tTOK_%s = %d,\n", lex, t);
		}
		else
		{
			fprintf(fp, "\tTOK_SYM_%d = %d, /* ", t, t);
			PrintCSanitizedComment(fp, lex);
			fprintf(fp, " */\n");
		}
	}
	fprintf(fp, "\tTOK_EOF = %d\n};\n\n", pTables->countTerminals);

	fprintf(fp, "enum LlNonTerminals {\n");
	for (int nt = 0; nt < pTables->countNonTerminals; nt++)
	{
		fprintf(fp, "\tNT_%s = %d%s\n", pTables->aNonTerminals[nt].strLexeme, nt,
		        (nt < pTables->countNonTerminals - 1) ? "," : "");
	}
	fprintf(fp, "};\n\n");

	for (x = 0; x < pTables->countProds; x++) {
		fprintf(fp, "static const int prod_%d[] = { ", x);
		for (y = 1; y < pTables->aProds[x].count; y++) {
			int sv = (pTables->aProds[x].aSymbols[y].Type == T_NONTERMINAL) ? (10000 + pTables->aProds[x].aSymbols[y].id) : pTables->aProds[x].aSymbols[y].id;
			fprintf(fp, "%d, ", sv);
		}
		fprintf(fp, "-1 };\n");
	}
	fprintf(fp, "static const int * const aProductions[LL_NUM_PRODUCTIONS] = { ");
	for (x = 0; x < pTables->countProds; x++) fprintf(fp, "prod_%d%s", x, (x < pTables->countProds - 1) ? ", " : "");
	fprintf(fp, " };\nstatic const int aProdRhsLen[LL_NUM_PRODUCTIONS] = { ");
	for (x = 0; x < pTables->countProds; x++) fprintf(fp, "%d%s", pTables->aProds[x].count - 1, (x < pTables->countProds - 1) ? ", " : "");
	fprintf(fp, " };\n\nstatic const int tableLL[LL_NUM_NONTERM][LL_NUM_TERMINALS] = {\n");
	for (x = 0; x < pTables->countNonTerminals; x++) {
		fprintf(fp, "\t{ ");
		for (y = 0; y < numTerms; y++) {
			int pid = (pTables->tableLL && pTables->tableLL[x][y]) ? pTables->tableLL[x][y]->idProd : -1;
			fprintf(fp, "%3d%s", pid, (y < numTerms - 1) ? ", " : "");
		}
		fprintf(fp, " }%s\n", (x < pTables->countNonTerminals - 1) ? "," : "");
	}
	fprintf(fp, "};\n\nstatic void ExecuteLLAction(int prodId, YYSTYPE *valStack, int *vtop)\n{\n\tint rhsLen = aProdRhsLen[prodId];\n\tYYSTYPE yyval = (rhsLen > 0) ? valStack[*vtop - rhsLen + 1] : (YYSTYPE)0;\n\tswitch (prodId) {\n");
	for (x = 0; x < pTables->countProds; x++) {
		if (pTables->aProds[x].strAction && *pTables->aProds[x].strAction) {
			int rhsLen = pTables->aProds[x].count - 1;
			char rhsFmt[64];
			snprintf(rhsFmt, sizeof(rhsFmt), "valStack[*vtop - %d + %%d]", rhsLen);
			char *tr = TranslateSemanticAction(pTables->aProds[x].strAction, rhsLen, "yyval", rhsFmt);
			fprintf(fp, "\t\tcase %d: {\n%s\n\t\t\tbreak;\n\t\t}\n", x, tr ? tr : "");
			free(tr);
		}
	}
	fprintf(fp, "\t\tdefault: break;\n\t}\n\t*vtop -= rhsLen;\n\tvalStack[++(*vtop)] = yyval;\n}\n\n");

	fprintf(fp,
		"int ParseLL1(yyscan_t yyscanner, YYSTYPE *pResult)\n{\n"
		"\tsize_t cap = 256;\n"
		"\tint *symStack = (int *)malloc(cap * sizeof(int));\n"
		"\tYYSTYPE *valStack = (YYSTYPE *)malloc(cap * sizeof(YYSTYPE));\n"
		"\tif (!symStack || !valStack) {\n"
		"\t\tfree(symStack); free(valStack);\n"
		"\t\tyyerror(yyscanner, \"Out of memory (initial allocation)\");\n"
		"\t\treturn 0;\n"
		"\t}\n"
		"\tint stop = -1, vtop = -1;\n"
		"\tYYSTYPE yylval = (YYSTYPE)0;\n"
		"\tsymStack[++stop] = LL_TOKEN_DOLLAR;\n"
		"\tsymStack[++stop] = LL_SYM_NT_BASE + %d;\n"
		"\tint lookahead = yylex(&yylval, yyscanner);\n"
		"\twhile (stop >= 0) {\n"
		"\t\tint topSym = symStack[stop];\n"
		"\t\tif (topSym >= LL_SYM_ACTION_BASE) {\n"
		"\t\t\tint pid = topSym - LL_SYM_ACTION_BASE;\n"
		"\t\t\tstop--;\n"
		"\t\t\tif (aProdRhsLen[pid] == 0 && vtop >= (int)cap - 1) {\n"
		"\t\t\t\tsize_t new_cap = cap * 2;\n"
		"\t\t\t\tint *tmp_s = (int *)realloc(symStack, new_cap * sizeof(int));\n"
		"\t\t\t\tif (!tmp_s) { free(symStack); free(valStack); yyerror(yyscanner, \"OOM\"); return 0; }\n"
		"\t\t\t\tsymStack = tmp_s;\n"
		"\t\t\t\tYYSTYPE *tmp_v = (YYSTYPE *)realloc(valStack, new_cap * sizeof(YYSTYPE));\n"
		"\t\t\t\tif (!tmp_v) { free(symStack); free(valStack); yyerror(yyscanner, \"OOM\"); return 0; }\n"
		"\t\t\t\tvalStack = tmp_v;\n"
		"\t\t\t\tcap = new_cap;\n"
		"\t\t\t}\n"
		"\t\t\tExecuteLLAction(pid, valStack, &vtop);\n"
		"\t\t}\n",
		startNt
	);

	fprintf(fp,
		"\t\telse if (topSym < LL_SYM_NT_BASE) {\n"
		"\t\t\tif (topSym == lookahead) {\n"
		"\t\t\t\tstop--;\n"
		"\t\t\t\tif (lookahead != LL_TOKEN_DOLLAR) {\n"
		"\t\t\t\t\tif (vtop >= (int)cap - 1) {\n"
		"\t\t\t\t\t\tsize_t new_cap = cap * 2;\n"
		"\t\t\t\t\t\tint *tmp_s = (int *)realloc(symStack, new_cap * sizeof(int));\n"
		"\t\t\t\t\t\tif (!tmp_s) { free(symStack); free(valStack); yyerror(yyscanner, \"OOM\"); return 0; }\n"
		"\t\t\t\t\t\tsymStack = tmp_s;\n"
		"\t\t\t\t\t\tYYSTYPE *tmp_v = (YYSTYPE *)realloc(valStack, new_cap * sizeof(YYSTYPE));\n"
		"\t\t\t\t\t\tif (!tmp_v) { free(symStack); free(valStack); yyerror(yyscanner, \"OOM\"); return 0; }\n"
		"\t\t\t\t\t\tvalStack = tmp_v;\n"
		"\t\t\t\t\t\tcap = new_cap;\n"
		"\t\t\t\t\t}\n"
		"\t\t\t\t\tvalStack[++vtop] = yylval;\n"
		"\t\t\t\t\tlookahead = yylex(&yylval, yyscanner);\n"
		"\t\t\t\t}\n"
		"\t\t\t}\n"
		"\t\t\telse {\n"
		"\t\t\t\tyyerror(yyscanner, \"Syntax error\");\n"
		"\t\t\t\tfree(symStack); free(valStack);\n"
		"\t\t\t\treturn 0;\n"
		"\t\t\t}\n"
		"\t\t} else {\n"
		"\t\t\tint pid = (lookahead >= 0 && lookahead < LL_NUM_TERMINALS) ? tableLL[topSym - LL_SYM_NT_BASE][lookahead] : -1;\n"
		"\t\t\tif (pid < 0) {\n"
		"\t\t\t\tyyerror(yyscanner, \"Syntax error\");\n"
		"\t\t\t\tfree(symStack); free(valStack);\n"
		"\t\t\t\treturn 0;\n"
		"\t\t\t}\n"
		"\t\t\tstop--;\n"
		"\t\t\tint needed = stop + 2 + aProdRhsLen[pid];\n"
		"\t\t\tif (needed > (int)cap) {\n"
		"\t\t\t\tsize_t new_cap = (size_t)needed * 2;\n"
		"\t\t\t\tint *tmp_s = (int *)realloc(symStack, new_cap * sizeof(int));\n"
		"\t\t\t\tif (!tmp_s) { free(symStack); free(valStack); yyerror(yyscanner, \"OOM\"); return 0; }\n"
		"\t\t\t\tsymStack = tmp_s;\n"
		"\t\t\t\tYYSTYPE *tmp_v = (YYSTYPE *)realloc(valStack, new_cap * sizeof(YYSTYPE));\n"
		"\t\t\t\tif (!tmp_v) { free(symStack); free(valStack); yyerror(yyscanner, \"OOM\"); return 0; }\n"
		"\t\t\t\tvalStack = tmp_v;\n"
		"\t\t\t\tcap = new_cap;\n"
		"\t\t\t}\n"
		"\t\t\tsymStack[++stop] = LL_SYM_ACTION_BASE + pid;\n"
		"\t\t\tfor (int i = aProdRhsLen[pid] - 1; i >= 0; i--) {\n"
		"\t\t\t\tsymStack[++stop] = aProductions[pid][i];\n"
		"\t\t\t}\n"
		"\t\t}\n"
		"\t}\n"
		"\tif (pResult && vtop >= 0) *pResult = valStack[vtop];\n"
		"\tfree(symStack);\n"
		"\tfree(valStack);\n"
		"\treturn 1;\n"
		"}\n"
	);

	if (pTables->strEpilogue && *pTables->strEpilogue)
		fprintf(fp, "\n/* --- Epilogo --- */\n%s\n", pTables->strEpilogue);
	fclose(fp);
}

