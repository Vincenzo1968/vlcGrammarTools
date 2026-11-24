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
		}
		free(pTables->aProds);
	}

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
	pTables->aNonTerminals = (Symbol*)malloc((sizeof(Symbol))*pTables->countTempNonTerminals);
	if ( !pTables->aNonTerminals )
	{
		printf("Insufficient memory to allocate non-terminal arrays!\n");
		return;
	}

	pTables->aTerminals = (Symbol*)malloc((sizeof(Symbol))*pTables->countTempTerminals);
	if ( !pTables->aTerminals )
	{
		printf("Insufficient memory to allocate terminal arrays!\n");
		return;
	}

	pTables->aProds = (ArrayProduction*)malloc((sizeof(ArrayProduction))*pTables->countTempProds);	
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
	//int i, s = 0, stack[64];
	int i, s = 0, stack[128];

	int up = start, down = stop - 1;
	Item part = a[stop];
	Item tmp;

	stack[s++] = start;
	stack[s++] = stop;

	while (s > 0)
	{
		stop = stack[--s];
		start = stack[--s];

		if (start >= stop)
			continue;

		// START PARTITIONING

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
	int *pStack;
	int top;
	int bStrEqual;
	int *pHashTable;

	Symbol tempNonTerminal;

	pfnCompare = Compare; // we set the pointer to the comparison function

	pStack = (int*)malloc((sizeof(int))*pTables->countNonTerminals);
	if ( !pStack )
	{
		printf("Sort function: Insufficient memory to allocate stack!\n");
		return;
	}

	pHashTable = (int*)malloc((sizeof(int))*pTables->countNonTerminals);
	if ( !pHashTable )
	{
		free(pStack);
		printf("Sort function: Not enough memory to allocate the HashTable!\n");
		return;
	}
	for ( x = 0; x < pTables->countNonTerminals; x++ )
		pHashTable[x] = 0;

	top = -1;

	pStack[++top] = pTables->aProds[0].aSymbols[0].id;
	pHashTable[pTables->aProds[0].aSymbols[0].id] = 1;

	for(x = 0; x < pTables->countProds; x++)
	{
		if ( pTables->aProds[x].aSymbols[0].id == pStack[top] )
		{
			y = pTables->aProds[x].count - 1;
			while ( y >= 1 )
			{
				if ( pTables->aProds[x].aSymbols[y].Type == T_NONTERMINAL )
				{
					if ( !pHashTable[pTables->aProds[x].aSymbols[y].id] )
					{
						pStack[++top] = pTables->aProds[x].aSymbols[y].id;
						pHashTable[pTables->aProds[x].aSymbols[y].id] = 1;
					}
				}
				y--;
			}
		}
	}

	for ( x = 0; x <= top; x++ )
	{
		y = pStack[x];

		if ( x != y )
		{
			tempNonTerminal = pTables->aNonTerminals[x];
			pTables->aNonTerminals[x] = pTables->aNonTerminals[y];
			pTables->aNonTerminals[y] = tempNonTerminal;
			x++;
		}
	}

	for ( x = 0; x < pTables->countNonTerminals; x++ )
	{
		pTables->aNonTerminals[x].id = x;

		for ( y = 0; y < pTables->countProds; y++ )
		{
			for ( z = 0; z < pTables->aProds[y].count; z++ )
			{
				if ( pTables->aNonTerminals[x].Type == T_NONTERMINAL )
				{
					bStrEqual = strcmp(pTables->aProds[y].aSymbols[z].strLexeme, pTables->aNonTerminals[x].strLexeme) == 0;
					if ( bStrEqual )
					{
						pTables->aProds[y].aSymbols[z].id = x;
					}
				}
			}
		}
	}

	QuickSortNonRecursive(pTables->aProds, 0, pTables->countProds - 1);

	free(pHashTable);
	free(pStack);
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
	int x, y, z;
	char* strSym;

	for ( x = 0; x < pTables->countTerminals; x++ )
	{
		pTables->aTerminals[x].bNullable = 0;
	}

	for ( x = 0; x < pTables->countProds; x++ )
	{
		pTables->aProds[x].aSymbols[0].bNullable = 0;
		for ( y = 1; y < pTables->aProds[x].count; y++ )
		{
			pTables->aProds[x].aSymbols[y].bNullable = 0;
		}
	}

	// Let's calculate the direct nullability:
	for(x = 0; x < pTables->countProds; x++)
	{
		strSym = pTables->aProds[x].aSymbols[0].strLexeme;
		if ( pTables->aProds[x].count == 1 )
		{
			for(y = 0; y < pTables->countNonTerminals; y++)
			{
				if ( strcmp(pTables->aNonTerminals[y].strLexeme, strSym) == 0 )
				{
					pTables->aNonTerminals[y].bNullable = 1;
					pTables->aProds[x].aSymbols[0].bNullable = 1;
					//printf("%s e' direttamente nullabile.\n", strSym);
					break;
				}
			}
		}
	}

	// Let's calculate indirect nullability
	for(x = 0; x < pTables->countProds; x++)
	{
		if ( pTables->aProds[x].count == 1 )
			continue;

		for (y = 1; y < pTables->aProds[x].count; y++)
		{
			if ( !IsNullable(pTables, pTables->aProds[x].aSymbols[y].strLexeme) )
			{
				pTables->aProds[x].aSymbols[y].bNullable = 0;
				break;
			}
			else
			{
				pTables->aProds[x].aSymbols[y].bNullable = 1;
			}
		}
		if ( y < pTables->aProds[x].count )
			continue;

		strSym = pTables->aProds[x].aSymbols[0].strLexeme;
		pTables->aProds[x].aSymbols[0].bNullable = 1;
		for(y = 0; y < pTables->countNonTerminals; y++)
		{
			if ( strcmp(pTables->aNonTerminals[y].strLexeme, strSym) == 0 )
			{
				pTables->aNonTerminals[y].bNullable = 1;
				break;
			}
		}
	}


	for ( z = 0; z < pTables->countNonTerminals; z++ )
	{
		for ( x = 0; x < pTables->countProds; x++ )
		{
			for ( y = 0; y < pTables->aProds[x].count; y++ )
			{
				if ( pTables->aProds[x].aSymbols[y].Type == T_TERMINAL )
				{
					pTables->aProds[x].aSymbols[y].bNullable = 0;
					continue;
				}

				if ( strcmp(pTables->aProds[x].aSymbols[y].strLexeme, pTables->aNonTerminals[z].strLexeme) == 0 )
				{
					pTables->aProds[x].aSymbols[y].bNullable = pTables->aNonTerminals[z].bNullable;
				}
			}
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

	int bTerminalFirst;
	int bAllNullable;
	int countTerminal, countNonTerminal;

	int *pStack1;
	int *pStack2;
	int *pStack3;
	int top1, top2, top3;

	int idNonTerminal;
	TerminalList termList;
	TerminalList *pTermList;
	TerminalList *pTermListFrom;
	TerminalList *pTermListTemp;

	HashTable **pHashTable;
	char szKey[256];

	pHashTable = (HashTable**)malloc((sizeof(HashTable**)) * HT_SIZE);
	if ( !pHashTable )
	{
		printf("Insufficient memory to allocate HashTable!\n");
		return;
	}
	for ( x = 0; x < HT_SIZE; x++ )
	{
		pHashTable[x] = NULL;
	}


	pStack1 = (int*)malloc((sizeof(int))*pTables->countProds);
	if ( !pStack1 )
	{
		free(pHashTable);
		printf("Insufficient memory to allocate Stack1!\n");
		return;
	}

	pStack2 = (int*)malloc((sizeof(int))*pTables->countProds);
	if ( !pStack2 )
	{
		free(pHashTable);
		free(pStack1);
		printf("Insufficient memory to allocate Stack2!\n");
		return;
	}

	pStack3 = (int*)malloc((sizeof(int))*pTables->countProds);
	if ( !pStack3 )
	{
		free(pHashTable);
		free(pStack1);
		free(pStack2);
		printf("Insufficient memory to allocate Stack3!\n");
		return;
	}

	top1 = top2 = top3 = -1;
	for(x = 0; x < pTables->countProds; x++)
	{
		bTerminalFirst = 0;
		countTerminal = countNonTerminal = 0;

		if ( pTables->aProds[x].count == 1 )
		{
			countTerminal++;
			bTerminalFirst = 1;
		}

		for (y = 1; y < pTables->aProds[x].count; y++)
		{
			if ( pTables->aProds[x].aSymbols[y].Type == T_TERMINAL )
			{
				countTerminal++;
				if ( y == 1 )
					bTerminalFirst = 1;
			}

			if ( pTables->aProds[x].aSymbols[y].Type == T_NONTERMINAL )
			{
				countNonTerminal++;
			}
		}

		// rhs contains productions whose body begins with a terminal or empty
		if ( bTerminalFirst ) 
		{
			pStack1[++top1] = x;
		}

		// The RHS contains at least one terminal (but not at the beginning of the body).
		if ( (!bTerminalFirst) && (countTerminal > 0) && (countNonTerminal > 0) ) 
		{
			pStack2[++top2] = x;
		}

		// The RHS contains only non-terminal symbols.
		if ( (countTerminal == 0) && (countNonTerminal > 0) ) 
		{
			pStack3[++top3] = x;
		}
	}


	for(x = top1; x >= 0; x--)
	{
		//printf("FIRST(%s) = {", pTables->aProds[pStack1[x]].aSymbols[0].strLexeme);
		//pTermList = pTables->aProds[pStack1[x]].aSymbols[0].pFirstSet;
		idNonTerminal = pTables->aProds[pStack1[x]].aSymbols[0].id;
		pTermList = pTables->aNonTerminals[idNonTerminal].pFirstSet;
		if ( pTables->aProds[pStack1[x]].count == 1 )
		{
			termList.idSymbol = T_EMPTY;
			termList.next = NULL;
			strcpy(szKey, pTables->aNonTerminals[idNonTerminal].strLexeme);
			strcat(szKey, "empty");
			if ( htFind(pHashTable, szKey) == -1 ) // File Not Found
			{
				pTermList = AppendTerminalList(&termList, pTermList, 0);
				//printf("empty");
				htInsert(pHashTable, szKey);
			}
		}
		else
		{
			termList.idSymbol = pTables->aProds[pStack1[x]].aSymbols[1].id;
			termList.next = NULL;
			strcpy(szKey, pTables->aNonTerminals[idNonTerminal].strLexeme);
			strcat(szKey, pTables->aProds[pStack1[x]].aSymbols[1].strLexeme);
			if ( htFind(pHashTable, szKey) == -1 ) // File Not Found
			{
				pTermList = AppendTerminalList(&termList, pTermList, 1);
				//printf("%s", pTables->aProds[pStack1[x]].aSymbols[1].strLexeme);
				htInsert(pHashTable, szKey);
			}
		}
		//printf("}\n");
		pTables->aNonTerminals[idNonTerminal].pFirstSet = pTermList;
	}

	for(x = top2; x >= 0; x--)
	{
		//printf("FIRST(%s) = {", pTables->aProds[pStack2[x]].aSymbols[0].strLexeme);
		//pTermList = pTables->aProds[pStack2[x]].aSymbols[0].pFirstSet;
		idNonTerminal = pTables->aProds[pStack2[x]].aSymbols[0].id;
		pTermList = pTables->aNonTerminals[idNonTerminal].pFirstSet;
		for (y = 1; y < pTables->aProds[pStack2[x]].count; y++)
		{
			if ( pTables->aProds[pStack2[x]].aSymbols[y].Type == T_TERMINAL )
			{
				termList.idSymbol = pTables->aProds[pStack2[x]].aSymbols[y].id;
				termList.next = NULL;
				strcpy(szKey, pTables->aNonTerminals[idNonTerminal].strLexeme);
				strcat(szKey, pTables->aProds[pStack2[x]].aSymbols[y].strLexeme);
				if ( htFind(pHashTable, szKey) == -1 ) // File Not Found
				{
					pTermList = AppendTerminalList(&termList, pTermList, 1);
					//printf("%s", pTables->aProds[pStack2[x]].aSymbols[y].strLexeme);
					htInsert(pHashTable, szKey);
				}
				break;
			}
			else // NON-TERMINAL
			{
				//if ( pTables->aProds[pStack2[x]].aSymbols[y].id == idNonTerminal )
				//	continue;

				pTermListFrom = pTables->aNonTerminals[pTables->aProds[pStack2[x]].aSymbols[y].id].pFirstSet;
				pTermListTemp = pTermListFrom;
				while( pTermListTemp != NULL )
				{
					strcpy(szKey, pTables->aNonTerminals[idNonTerminal].strLexeme);
					if ( pTermListTemp->idSymbol == T_DOLLAR )
						strcat(szKey, "$");
					else if ( pTermListTemp->idSymbol == T_EMPTY )
						strcat(szKey, "empty");
					else
						strcat(szKey, pTables->aTerminals[pTermListTemp->idSymbol].strLexeme);
					if ( htFind(pHashTable, szKey) == -1 ) // Not found
					{
						pTermList = AppendTerminalList(pTermListTemp, pTermList, 1);
						if ( pTermListTemp->idSymbol != T_EMPTY )
							htInsert(pHashTable, szKey);
					}
					pTermListTemp = pTermListTemp->next;
				}
				//printf("(FIRST(%s) - empty) ", pTables->aProds[pStack2[x]].aSymbols[y].strLexeme);
				if ( !(pTables->aProds[pStack2[x]].aSymbols[y].bNullable) )
				{
					break;
				}
			}
		}
		//printf("}\n");
		pTables->aNonTerminals[idNonTerminal].pFirstSet = pTermList;
	}

	for(x = top3; x >= 0; x--)
	{
		//printf("FIRST(%s) = {", pTables->aProds[pStack3[x]].aSymbols[0].strLexeme);
		idNonTerminal = pTables->aProds[pStack3[x]].aSymbols[0].id;
		pTermList = pTables->aNonTerminals[idNonTerminal].pFirstSet;
		bAllNullable = 1;
		for (y = 1; y < pTables->aProds[pStack3[x]].count; y++)
		{
			//if ( pTables->aProds[pStack3[x]].aSymbols[y].id == idNonTerminal )
			//	continue;

			pTermListFrom = pTables->aNonTerminals[pTables->aProds[pStack3[x]].aSymbols[y].id].pFirstSet;
			pTermListTemp = pTermListFrom;
			while( pTermListTemp != NULL )
			{
				strcpy(szKey, pTables->aNonTerminals[idNonTerminal].strLexeme);
				if ( pTermListTemp->idSymbol == T_DOLLAR )
					strcat(szKey, "$");
				else if ( pTermListTemp->idSymbol == T_EMPTY )
					strcat(szKey, "empty");
				else
					strcat(szKey, pTables->aTerminals[pTermListTemp->idSymbol].strLexeme);
				if ( htFind(pHashTable, szKey) == -1 ) // Not found
				{
					pTermList = AppendTerminalList(pTermListTemp, pTermList, 1);
					if ( pTermListTemp->idSymbol != T_EMPTY )
						htInsert(pHashTable, szKey);
				}
				pTermListTemp = pTermListTemp->next;
			}
			//printf("(FIRST(%s) - empty) ", pTables->aProds[pStack3[x]].aSymbols[y].strLexeme);
			if ( !(pTables->aProds[pStack3[x]].aSymbols[y].bNullable) )
			{
				bAllNullable = 0;
				break;
			}
		}
		if ( bAllNullable )
		{
			termList.idSymbol = T_EMPTY;
			termList.next = NULL;
			strcpy(szKey, pTables->aNonTerminals[idNonTerminal].strLexeme);
			strcat(szKey, "empty");
			if ( htFind(pHashTable, szKey) == -1 ) // Not found
			{
				pTermList = AppendTerminalList(&termList, pTermList, 0);
				htInsert(pHashTable, szKey);
			}
			//printf("empty");
		}
		//printf("}\n");

		pTables->aNonTerminals[idNonTerminal].pFirstSet = pTermList;
	}

	for ( x = 0; x < HT_SIZE; x++ )
	{
		htFree(pHashTable[x]);
	}
	free(pHashTable);
	free(pStack1);
	free(pStack2);
	free(pStack3);

	// S -> c A d
	// A -> a b | a
}

void MakeFollowSets(Tables *pTables)
{
	int x, y, z;
	int w;
	int bNullable;

	int idNonTerminal = 0;
	int idNonTerminalNext = 0;

	TerminalList termList;
	TerminalList *pTermList;
	TerminalList *pTermListFrom;
	TerminalList *pTermListTemp;

	ConstraintsList constraintsList;
	ConstraintsList *pConstraintsList;
	ConstraintsList *pConstraintsListTemp;


	int *paNonTerminals;
	int index;

	HashTable **pHashTable;
	char szKey[256];

	pConstraintsList = NULL;

	pHashTable = (HashTable**)malloc((sizeof(HashTable**)) * HT_SIZE);
	if ( !pHashTable )
	{
		printf("Insufficient memory to allocate HashTable!\n");
		return;
	}
	for ( x = 0; x < HT_SIZE; x++ )
	{
		pHashTable[x] = NULL;
	}


	paNonTerminals = (int*)malloc((sizeof(int))*pTables->countNonTerminals);
	if ( !paNonTerminals )
	{
		printf("Insufficient memory to allocate Array Non-Terminals!\n");
		return;
	}

	index = 0;
	if ( pTables->countProds > 0 )
		paNonTerminals[index++] = pTables->aProds[0].aSymbols[0].id;

	for ( x = 1; x < pTables->countProds; x++ )
	{
		if ( pTables->aProds[x].aSymbols[0].id == paNonTerminals[index - 1] )
			continue;

		paNonTerminals[index++] = pTables->aProds[x].aSymbols[0].id;
	}
	
	if ( index > 0 )
	{
		termList.idSymbol = T_DOLLAR;
		termList.next = NULL;
		idNonTerminal = paNonTerminals[0];
		//idNonTerminal = pTables->aNonTerminals[0].id;
		strcpy(szKey, pTables->aNonTerminals[idNonTerminal].strLexeme);
		strcat(szKey, "$");
		if ( htFind(pHashTable, szKey) == -1 ) // Not found
		{
			pTables->aNonTerminals[0].pFollowSet = AppendTerminalList(&termList, pTables->aNonTerminals[0].pFollowSet, 1);
			htInsert(pHashTable, szKey);
		}
	}

	for ( x = 0; x < index; x++ )
	{
		idNonTerminal = paNonTerminals[x];
		pTermList = pTables->aNonTerminals[idNonTerminal].pFollowSet;

		for ( y = 0; y < pTables->countProds; y++ )
		{
			for ( z = 1; z < pTables->aProds[y].count; z++ )
			{
				if ( pTables->aProds[y].aSymbols[z].Type == T_NONTERMINAL && pTables->aProds[y].aSymbols[z].id == idNonTerminal )
				{
					bNullable = 1;
					for ( w = z + 1; w < pTables->aProds[y].count; w++ )
					{
						if ( pTables->aProds[y].aSymbols[w].Type == T_TERMINAL )
						{
							termList.idSymbol = pTables->aProds[y].aSymbols[w].id;
							termList.next = NULL;
							strcpy(szKey, pTables->aNonTerminals[idNonTerminal].strLexeme);
							strcat(szKey, pTables->aProds[y].aSymbols[w].strLexeme);
							if ( htFind(pHashTable, szKey) == -1 ) // Not found
							{
								pTermList = AppendTerminalList(&termList, pTermList, 1);
								htInsert(pHashTable, szKey);
							}
							bNullable = 0;
						}
						else // NON-TERMINAL
						{
							idNonTerminalNext = pTables->aProds[y].aSymbols[w].id;

							//if ( idNonTerminalNext == idNonTerminal )
							//	continue;

							if ( idNonTerminalNext == pTables->aProds[y].aSymbols[0].id )
								continue;

							pTermListFrom = pTables->aNonTerminals[idNonTerminalNext].pFirstSet;
							pTermListTemp = pTermListFrom;
							if ( pTermListTemp == NULL )
							{
								constraintsList.idNT1 = idNonTerminal;
								constraintsList.idNT2 = idNonTerminalNext;
								constraintsList.type = TYPE_CONSTRAINT_FIRST;
								constraintsList.next = NULL;
								pConstraintsList = AppendConstraintsList(&constraintsList, pConstraintsList);
							}
							while( pTermListTemp != NULL )
							{
								strcpy(szKey, pTables->aNonTerminals[idNonTerminal].strLexeme);
								if ( pTermListTemp->idSymbol == T_DOLLAR )
									strcat(szKey, "$");
								else if ( pTermListTemp->idSymbol == T_EMPTY )
									strcat(szKey, "empty");
								else
									strcat(szKey, pTables->aTerminals[pTermListTemp->idSymbol].strLexeme);
								if ( pTermListTemp->idSymbol != T_EMPTY && htFind(pHashTable, szKey) == -1 ) // Not found
								{
									pTermList = AppendTerminalList(pTermListTemp, pTermList, 1);
									htInsert(pHashTable, szKey);
								}
								pTermListTemp = pTermListTemp->next;
							}
							bNullable = pTables->aNonTerminals[idNonTerminalNext].bNullable;
						}

						if ( !bNullable )
							break;
					}

					if ( bNullable && pTables->aProds[y].aSymbols[0].id != idNonTerminal )
					{
						pTermListFrom = pTables->aNonTerminals[pTables->aProds[y].aSymbols[0].id].pFollowSet;
						pTermListTemp = pTermListFrom;
						if ( pTermListTemp == NULL )
						{
							constraintsList.idNT1 = idNonTerminal;
							constraintsList.idNT2 = pTables->aProds[y].aSymbols[0].id;
							constraintsList.type = TYPE_CONSTRAINT_FOLLOW;
							constraintsList.next = NULL;
							pConstraintsList = AppendConstraintsList(&constraintsList, pConstraintsList);
						}
						while( pTermListTemp != NULL )
						{
							strcpy(szKey, pTables->aNonTerminals[idNonTerminal].strLexeme);
							if ( pTermListTemp->idSymbol == T_DOLLAR )
								strcat(szKey, "$");
							else if ( pTermListTemp->idSymbol == T_EMPTY )
								strcat(szKey, "empty");
							else
								strcat(szKey, pTables->aTerminals[pTermListTemp->idSymbol].strLexeme);
							if ( pTermListTemp->idSymbol != T_EMPTY && htFind(pHashTable, szKey) == -1 ) // Not found
							{
								pTermList = AppendTerminalList(pTermListTemp, pTermList, 1);
								htInsert(pHashTable, szKey);
							}
							pTermListTemp = pTermListTemp->next;
						}
					}
				}
			}
		}

		pTables->aNonTerminals[idNonTerminal].pFollowSet = pTermList;
	}


	pConstraintsListTemp = pConstraintsList;
	while( pConstraintsListTemp != NULL )
	{
		idNonTerminal = pConstraintsListTemp->idNT1;
		idNonTerminalNext = pConstraintsListTemp->idNT2;

		pTermList = pTables->aNonTerminals[idNonTerminal].pFollowSet;

		if ( pConstraintsListTemp->type == TYPE_CONSTRAINT_FIRST )
			pTermListFrom = pTables->aNonTerminals[idNonTerminalNext].pFirstSet;
		else
			pTermListFrom = pTables->aNonTerminals[idNonTerminalNext].pFollowSet;
		pTermListTemp = pTermListFrom;
		while( pTermListTemp != NULL )
		{
			strcpy(szKey, pTables->aNonTerminals[idNonTerminal].strLexeme);
			if ( pTermListTemp->idSymbol == T_DOLLAR )
				strcat(szKey, "$");
			else if ( pTermListTemp->idSymbol == T_EMPTY )
				strcat(szKey, "empty");
			else
				strcat(szKey, pTables->aTerminals[pTermListTemp->idSymbol].strLexeme);
			if ( pTermListTemp->idSymbol != T_EMPTY && htFind(pHashTable, szKey) == -1 ) // Not found
			{
				pTermList = AppendTerminalList(pTermListTemp, pTermList, 1);
				htInsert(pHashTable, szKey);
			}
			pTermListTemp = pTermListTemp->next;
		}
		pConstraintsListTemp = pConstraintsListTemp->next;

		pTables->aNonTerminals[idNonTerminal].pFollowSet = pTermList;
	}



	for ( x = 0; x < HT_SIZE; x++ )
	{
		htFree(pHashTable[x]);
	}
	free(pHashTable);

	FreeConstraintsList(pConstraintsList);
	free(paNonTerminals);
}

void MakeTableLL(Tables *pTables)
{
	int x, y, z;
	int rows, columns;

	int bNullable;

	int idTerminal;
	int idNonTerminal;
	int idNonTerminalHead;

	TerminalList *pTermList;
	TerminalList *pTermListFrom;

	ProdsList prodsList;
	ProblemList problemList;

	HashTable **pHashTable;
	char szKey[512];

	HashTable **pHashTableProblem;
	char szKeyProblem[128];

	pHashTable = (HashTable**)malloc((sizeof(HashTable**)) * HT_SIZE);
	if ( !pHashTable )
	{
		printf("Insufficient memory to allocate HashTable!\n");
		return;
	}
	for ( x = 0; x < HT_SIZE; x++ )
	{
		pHashTable[x] = NULL;
	}

	pHashTableProblem = (HashTable**)malloc((sizeof(HashTable**)) * HT_SIZE);
	if ( !pHashTableProblem )
	{
		free(pHashTable);
		printf("Insufficient memory to allocate HashTableProblem!\n");
		return;
	}
	for ( x = 0; x < HT_SIZE; x++ )
	{
		pHashTableProblem[x] = NULL;
	}


	pTables->problemList = NULL;

	rows = pTables->countNonTerminals;
	columns = pTables->countTerminals + 1; // +1 for the $ symbol

	pTables->tableLL = (ProdsList* **)malloc(sizeof(ProdsList* *) * rows);
	if ( !pTables->tableLL )
	{
		free(pHashTable);
		free(pHashTableProblem);
		printf("Insufficient memory to allocate the table rows LL!\n");
		return;
	}

	for ( x = 0; x < rows; x++ )
	{
		pTables->tableLL[x] = (ProdsList* *)malloc(sizeof(ProdsList*) * columns);
		if ( !pTables->tableLL[x] )
		{
			free(pHashTable);
			free(pHashTableProblem);
			printf("Insufficient memory to allocate the columns from Table LL!\n");
			return;
		}
	}

	for ( x = 0; x < rows; x++ )
		for ( y = 0; y < columns; y++ )
			pTables->tableLL[x][y] = NULL;


	for ( x = 0; x < pTables->countProds; x++ )
	{
		bNullable = 1;

		for ( y = 1; y < pTables->aProds[x].count; y++ )
		{
			if ( pTables->aProds[x].aSymbols[y].Type == T_TERMINAL )
			{
				idNonTerminalHead = pTables->aProds[x].aSymbols[0].id;
				idTerminal = pTables->aProds[x].aSymbols[y].id;
				prodsList.idProd = x;
				prodsList.next = NULL;
				strcpy(szKey, pTables->aNonTerminals[idNonTerminalHead].strLexeme);
				if ( idTerminal == T_EMPTY )
					strcat(szKey, "empty");
				else if ( idTerminal == T_DOLLAR || idTerminal >= pTables->countTerminals )
					strcat(szKey, "$");
				else
					strcat(szKey, pTables->aTerminals[idTerminal].strLexeme);
				strcpy(szKeyProblem, szKey);
				for ( z = 0; z < pTables->aProds[x].count; z++ )
				{
					strcat(szKey, pTables->aProds[x].aSymbols[z].strLexeme);
				}
				if ( htFind(pHashTable, szKey) == -1 ) // Not Found
				{
					pTables->tableLL[idNonTerminalHead][idTerminal] = AppendProdsList(&prodsList, pTables->tableLL[idNonTerminalHead][idTerminal]); 
					if ( pTables->tableLL[idNonTerminalHead][idTerminal]->next != NULL )
					{
						if ( htFind(pHashTableProblem, szKeyProblem) == -1 )
						{
							problemList.idNonTerminal = idNonTerminalHead;
							problemList.idTerminal = idTerminal;
							problemList.next = NULL;
							pTables->problemList = AppendProblemList(&problemList, pTables->problemList);
							htInsert(pHashTableProblem, szKeyProblem);
						}
					}
					htInsert(pHashTable, szKey);
				}
				bNullable = 0;
			}
			else // NON-TERMINAL
			{
				idNonTerminal = pTables->aProds[x].aSymbols[y].id;
				idNonTerminalHead = pTables->aProds[x].aSymbols[0].id;

				pTermListFrom = pTables->aNonTerminals[idNonTerminal].pFirstSet;
				pTermList = pTermListFrom;
				while( pTermList != NULL )
				{
					idTerminal = pTermList->idSymbol;
					if ( idTerminal >= 0 || idTerminal == T_DOLLAR) // We do not consider empty
					{
						if ( idTerminal == T_DOLLAR )
							idTerminal = pTables->countTerminals;
						prodsList.idProd = x;
						prodsList.next = NULL;
						strcpy(szKey, pTables->aNonTerminals[idNonTerminalHead].strLexeme);
						if ( idTerminal == T_EMPTY )
							strcat(szKey, "empty");
						else if ( idTerminal == T_DOLLAR || idTerminal >= pTables->countTerminals )
							strcat(szKey, "$");
						else
							strcat(szKey, pTables->aTerminals[idTerminal].strLexeme);
						strcpy(szKeyProblem, szKey);
						for ( z = 0; z < pTables->aProds[x].count; z++ )
						{
							strcat(szKey, pTables->aProds[x].aSymbols[z].strLexeme);
						}
						if ( htFind(pHashTable, szKey) == -1 ) // Not found
						{
							pTables->tableLL[idNonTerminalHead][idTerminal] = AppendProdsList(&prodsList, pTables->tableLL[idNonTerminalHead][idTerminal]); 
							if ( pTables->tableLL[idNonTerminalHead][idTerminal]->next != NULL )
							{
								if ( htFind(pHashTableProblem, szKeyProblem) == -1 )
								{
									problemList.idNonTerminal = idNonTerminalHead;
									problemList.idTerminal = idTerminal;
									problemList.next = NULL;
									pTables->problemList = AppendProblemList(&problemList, pTables->problemList);
									htInsert(pHashTableProblem, szKeyProblem);
								}
							}
							htInsert(pHashTable, szKey);
						}
					}
					pTermList = pTermList->next;
				}
				bNullable = pTables->aNonTerminals[idNonTerminal].bNullable;
			}

			if ( !bNullable )
				break;
		}

		if ( bNullable )
		{
			// If the right-hand side of the production is nullable, we must consider the Follow Set.
			idNonTerminalHead = pTables->aProds[x].aSymbols[0].id;
			pTermListFrom = pTables->aNonTerminals[idNonTerminalHead].pFollowSet;
			pTermList = pTermListFrom;
			while( pTermList != NULL )
			{
				idTerminal = pTermList->idSymbol;
				if ( idTerminal >= 0 || idTerminal == T_DOLLAR) // We do not consider empty
				{
					if ( idTerminal == T_DOLLAR )
						idTerminal = pTables->countTerminals;
					prodsList.idProd = x;
					prodsList.next = NULL;
					strcpy(szKey, pTables->aNonTerminals[idNonTerminalHead].strLexeme);
					if ( idTerminal == T_EMPTY )
						strcat(szKey, "empty");
					else if ( idTerminal == T_DOLLAR || idTerminal >= pTables->countTerminals )
						strcat(szKey, "$");
					else
						strcat(szKey, pTables->aTerminals[idTerminal].strLexeme);
					strcpy(szKeyProblem, szKey);
					for ( z = 0; z < pTables->aProds[x].count; z++ )
					{
						strcat(szKey, pTables->aProds[x].aSymbols[z].strLexeme);
					}
					if ( htFind(pHashTable, szKey) == -1 ) // Not found
					{
						pTables->tableLL[idNonTerminalHead][idTerminal] = AppendProdsList(&prodsList, pTables->tableLL[idNonTerminalHead][idTerminal]); 
						if ( pTables->tableLL[idNonTerminalHead][idTerminal]->next != NULL )
						{
							if ( htFind(pHashTableProblem, szKeyProblem) == -1 )
							{
								problemList.idNonTerminal = idNonTerminalHead;
								problemList.idTerminal = idTerminal;
								problemList.next = NULL;
								pTables->problemList = AppendProblemList(&problemList, pTables->problemList);
								htInsert(pHashTableProblem, szKeyProblem);
							}
						}
						htInsert(pHashTable, szKey);
					}
				}
				pTermList = pTermList->next;
			}
		} // END if( bNullable )
	}

	for ( x = 0; x < HT_SIZE; x++ )
	{
		htFree(pHashTable[x]);
		htFree(pHashTableProblem[x]);
	}
	free(pHashTable);
	free(pHashTableProblem);
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
		fprintf(fp, "<li>%s</li>\n", pTables->aNonTerminals[x].strLexeme);
	}
	fprintf(fp, "</ul>\n</div>\n");

	fprintf(fp, "<div class=\"card\">\n<h2>%d Terminal Symbols</h2>\n<ul class=\"badge-list term\">\n", pTables->countTerminals);
	for (x = 0; x < pTables->countTerminals; x++)
	{
		fprintf(fp, "<li>%s</li>\n", pTables->aTerminals[x].strLexeme);
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
				fprintf(fp, "<td class=\"lhs\">%s</td>\n", pTables->aProds[x].aSymbols[y].strLexeme);
				fprintf(fp, "<td class=\"arrow\">&rarr;</td>\n");
				fprintf(fp, "<td class=\"rhs\">");

				if ( pTables->aProds[x].count == 1 )
				{
					fprintf(fp, "&epsilon;");
				}
			}
			else
			{
				fprintf(fp, "%s ", pTables->aProds[x].aSymbols[y].strLexeme);
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
		fprintf(fp, "<td class=\"nt-col mono\">%s</td>\n", pTables->aNonTerminals[x].strLexeme);

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
				fprintf(fp, "%s ", pTables->aTerminals[pTermList->idSymbol].strLexeme);
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
				fprintf(fp, "%s ", pTables->aTerminals[pTermList->idSymbol].strLexeme);
			pTermList = pTermList->next;
		}
		if ( bDollar )
			fprintf(fp, "$ ");
		fprintf(fp, "</td>\n");

		fprintf(fp, "</tr>\n");
	}
	fprintf(fp, "</tbody>\n</table>\n</div>\n");
	// ---------------------------- End First Follow Sets ----------------------------------

	// ---------------------------- Start Problems List (se presente, mostralo in evidenza) -----
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
			if ( pProblemList->idTerminal != T_DOLLAR && pProblemList->idTerminal < pTables->countTerminals )
			{
				fprintf(fp, "<li>Table[%s, %s]</li>\n",
					pTables->aNonTerminals[pProblemList->idNonTerminal].strLexeme,
					pTables->aTerminals[pProblemList->idTerminal].strLexeme);
			}
			else
			{
				fprintf(fp, "<li>Table[%s, $]</li>\n",
					pTables->aNonTerminals[pProblemList->idNonTerminal].strLexeme);
			}
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
		if ( x < pTables->countTerminals )
			fprintf(fp, "<th class=\"mono\">%s</th>\n", pTables->aTerminals[x].strLexeme);
		else
			fprintf(fp, "<th class=\"mono\">$</th>\n");
	}
	fprintf(fp, "    </tr>\n  </thead>\n  <tbody>\n");

	for ( x = 0; x < pTables->countNonTerminals; x++ )
	{
		fprintf(fp, "<tr>\n");
		fprintf(fp, "<td class=\"nt-col mono\">%s</td>\n", pTables->aNonTerminals[x].strLexeme);

		for ( y = 0; y < pTables->countTerminals + 1; y++ )
		{
			if ( pTables->tableLL[x][y] != NULL )
			{
				pProdList = pTables->tableLL[x][y];
				// If there are multiple productions in the same cell, apply the conflict style
				if ( pProdList->next != NULL )
					fprintf(fp, "<td class=\"cell-conflict\">");
				else
					fprintf(fp, "<td class=\"cell-prod\">");

				while ( pProdList != NULL )
				{
					for ( z = 0; z < pTables->aProds[pProdList->idProd].count; z++ )
					{
						fprintf(fp, "%s", pTables->aProds[pProdList->idProd].aSymbols[z].strLexeme);
						if ( z == 0 )
						{
							fprintf(fp, " &rarr; ");
							if ( pTables->aProds[pProdList->idProd].count == 1 )
								fprintf(fp, "&epsilon;");
						}
						else
						{
							fprintf(fp, " ");
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

void MakeCFile(Tables *pTables, const char *szFileName)
{
	//int x, y, z;
	int x, y;
	FILE *fp;

	if ( pTables->problemList != NULL )
	{
		printf("Unable to generate the C parser: the grammar is not LL(1).\n");
		return;
	}

	fp = fopen(szFileName, "wt");
	if ( fp == NULL )
	{
		printf("Error opening file %s.\n", szFileName);
		return;
	}

	fprintf(fp, "/* Automatically generated by LL(1) Grammar Tools – Vincenzo Lo Cicero */\n");
	fprintf(fp, "#ifndef PARSER_TABLES_H\n#define PARSER_TABLES_H\n\n");

	fprintf(fp, "#define NUM_NONTERMINALS %d\n", pTables->countNonTerminals);
	fprintf(fp, "#define NUM_TERMINALS    %d /* Including '$' */\n", pTables->countTerminals + 1);
	fprintf(fp, "#define NUM_PRODUCTIONS  %d\n", pTables->countProds);
	fprintf(fp, "#define NT_OFFSET        %d\n", NT_OFFSET);
	fprintf(fp, "#define TOKEN_DOLLAR     %d\n", pTables->countTerminals);
	fprintf(fp, "#define START_SYMBOL     (NT_OFFSET + 0)\n\n");

	// 1. Terminal Names (for error messages and debugging)
	fprintf(fp, "const char *szTerminals[NUM_TERMINALS] = {\n");
	for ( x = 0; x < pTables->countTerminals; x++ )
		fprintf(fp, "\t\"%s\",\n", pTables->aTerminals[x].strLexeme);
	fprintf(fp, "\t\"$\"\n};\n\n");

	// 2. Names of Non-Terminals
	fprintf(fp, "const char *szNonTerminals[NUM_NONTERMINALS] = {\n");
	for ( x = 0; x < pTables->countNonTerminals; x++ )
		fprintf(fp, "\t\"%s\"%s\n", pTables->aNonTerminals[x].strLexeme,
				(x < pTables->countNonTerminals - 1) ? "," : "");
	fprintf(fp, "};\n\n");

	// 3. Production Structure (LHS and RHS)
	fprintf(fp, "typedef struct {\n\tint lhs;\n\tint rhsLen;\n\tint rhs[32];\n} Production;\n\n");
	fprintf(fp, "const Production aProductions[NUM_PRODUCTIONS] = {\n");
	for ( x = 0; x < pTables->countProds; x++ )
	{
		int rhsLen = pTables->aProds[x].count - 1;
		fprintf(fp, "\t{ NT_OFFSET + %d, %d, {", pTables->aProds[x].aSymbols[0].id, rhsLen);

		if ( rhsLen == 0 )
		{
			fprintf(fp, "-1 "); // Epsilon production
		}
		else
		{
			for ( y = 1; y < pTables->aProds[x].count; y++ )
			{
				if ( pTables->aProds[x].aSymbols[y].Type == T_NONTERMINAL )
					fprintf(fp, "NT_OFFSET + %d", pTables->aProds[x].aSymbols[y].id);
				else
					fprintf(fp, "%d", pTables->aProds[x].aSymbols[y].id);

				if ( y < pTables->aProds[x].count - 1 )
					fprintf(fp, ", ");
			}
			fprintf(fp, " ");
		}
		fprintf(fp, "} }%s\n", (x < pTables->countProds - 1) ? "," : "");
	}
	fprintf(fp, "};\n\n");

	// 4. Table LL(1): -1 = error, >= 0 = index in aProductions
	fprintf(fp, "const int tableLL[NUM_NONTERMINALS][NUM_TERMINALS] = {\n");
	for ( x = 0; x < pTables->countNonTerminals; x++ )
	{
		fprintf(fp, "\t{ ");
		for ( y = 0; y < pTables->countTerminals + 1; y++ )
		{
			if ( pTables->tableLL[x][y] != NULL )
				fprintf(fp, "%2d", pTables->tableLL[x][y]->idProd);
			else
				fprintf(fp, "-1");

			if ( y < pTables->countTerminals )
				fprintf(fp, ", ");
		}
		fprintf(fp, " }%s /* %s */\n",
				(x < pTables->countNonTerminals - 1) ? "," : "",
				pTables->aNonTerminals[x].strLexeme);
	}
	fprintf(fp, "};\n\n");

	fprintf(fp, "#endif /* PARSER_TABLES_H */\n");
	fclose(fp);
}

#define MAX_STACK 1024

void PrintStack(Tables *pTables, int *stack, int top)
{
	int i;
	printf("[ ");
	for ( i = 0; i <= top; i++ )
	{
		if ( stack[i] == pTables->countTerminals )
			printf("$ ");
		else if ( stack[i] >= NT_OFFSET )
			printf("%s ", pTables->aNonTerminals[stack[i] - NT_OFFSET].strLexeme);
		else
			printf("%s ", pTables->aTerminals[stack[i]].strLexeme);
	}
	printf("]");
}

// tokens[] contains the IDs of the input terminals, terminated by countTerminals ($)
int SimulateLL1(Tables *pTables, int *tokens, int numTokens)
{
	int stack[MAX_STACK];
	int top  = -1;
	int ip   = 0;  // Instruction/Input pointer 
	int step = 1;

	if ( pTables->problemList != NULL )
	{
		printf("Error: The grammar is not LL(1).\n");
		return 0;
	}

	// Push '$' and initial symbol
	stack[++top] = pTables->countTerminals; // '$' 
	stack[++top] = NT_OFFSET + 0;            // start symbol 

	printf("\n--- LL(1) PARSER SIMULATION ---\n");

	while ( top >= 0 )
	{
		int X = stack[top];
		int a = (ip < numTokens) ? tokens[ip] : pTables->countTerminals;

		printf("Step %2d | Stack: ", step++);
		PrintStack(pTables, stack, top);
		printf(" | Input: %s | Action: ",
			   (a == pTables->countTerminals) ? "$" : pTables->aTerminals[a].strLexeme);

		if ( X < NT_OFFSET ) /* X is a terminal or '$' */
		{
			if ( X == a )
			{
				top--; // Pop
				ip++;  // Advance input
				if ( X == pTables->countTerminals )
				{
					printf("ACCEPT!\n");
					return 1;
				}
				printf("Match '%s'\n", pTables->aTerminals[X].strLexeme);
			}
			else
			{
				printf("SYNTAX ERROR! Expected '%s', found '%s'\n",
					   (X == pTables->countTerminals) ? "$" : pTables->aTerminals[X].strLexeme,
					   (a == pTables->countTerminals) ? "$" : pTables->aTerminals[a].strLexeme);
				return 0;
			}
		}
		else // X is a non-terminal
		{
			int ntIdx = X - NT_OFFSET;
			ProdsList *entry = pTables->tableLL[ntIdx][a];

			if ( entry == NULL )
			{
				printf("SYNTAX ERROR! No production in Table[%s, %s]\n",
					   pTables->aNonTerminals[ntIdx].strLexeme,
					   (a == pTables->countTerminals) ? "$" : pTables->aTerminals[a].strLexeme);
				return 0;
			}
			else
			{
				int prodId = entry->idProd;
				int count = pTables->aProds[prodId].count;
				int k;

				top--; // Non-terminal pop

				// Print applied production
				printf("%s -> ", pTables->aProds[prodId].aSymbols[0].strLexeme);
				if ( count == 1 )
					printf("epsilon");
				else
					for ( k = 1; k < count; k++ )
						printf("%s ", pTables->aProds[prodId].aSymbols[k].strLexeme);
				printf("\n");

				// Push of RHS symbols in REVERSE order
				for ( k = count - 1; k >= 1; k-- )
				{
					if ( top >= MAX_STACK - 1 )
					{
						printf("Stack overflow!\n");
						return 0;
					}
					if ( pTables->aProds[prodId].aSymbols[k].Type == T_NONTERMINAL )
						stack[++top] = NT_OFFSET + pTables->aProds[prodId].aSymbols[k].id;
					else
						stack[++top] = pTables->aProds[prodId].aSymbols[k].id;
				}
			}
		}
	}

	return 0;
}
