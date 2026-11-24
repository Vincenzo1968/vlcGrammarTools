#define _POSIX_C_SOURCE 200809L
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
#include <math.h>
#include <string.h>
#include "parser.h"
#include "symtab.h"
#include "ast.h"

/*
 * prods_list : {prod} prod : T_NONTERMINAL ':' rhs; rhs : symbols_list {'|' symbols_list} ';' symbols_list : {T_TERMINAL | T_NONTERMINAL}
 */

int InitParser(ParserParams *pp)
{
	int x;
	
	if ( !InitScanner(&(pp->myToken), &(pp->sp)) )
		return 0;

	pp->numErrors = 0;
	pp->numWarnings = 0;

	pp->pTables = NULL;

	//pp->stack[128];
	pp->top = -1;

	pp->pHT = NULL;
	pp->strTemp[0] = '\0';
	
	pp->pHT = (HashTable**)malloc((sizeof(HashTable**)) * HT_SIZE);
	if ( !pp->pHT )
	{
		printf("Insufficient memory to allocate HashTable!\n");
		return 0;
	}
	for ( x = 0; x < HT_SIZE; x++ )
	{
		pp->pHT[x] = NULL;
	}	
	
	return 1;
}

void FreeParser(ParserParams *pp)
{
	int x;
	
	FreeScanner(&(pp->sp));

	for ( x = 0; x < HT_SIZE; x++ )
	{
		htFree(pp->pHT[x]);
	}
	free(pp->pHT);
}

int FindOrInsertSymbol(char *lexeme, int type, ParserParams *pp)
{
	int x;

	if ( type == T_TERMINAL )
	{
		x = 0;
		while ( x < pp->pTables->countTerminals )
		{
			if ( strcmp(lexeme, pp->pTables->aTerminals[x].strLexeme) == 0 )
			{
				return x;
			}
			x++;
		}
		if ( x >= pp->pTables->countTempTerminals )
		{
			pp->pTables->countTempTerminals += 21;
			pp->pTables->aTerminals = (Symbol*)realloc(pp->pTables->aTerminals, (sizeof(Symbol))*pp->pTables->countTempTerminals);
			if ( !pp->pTables->aTerminals )
			{
				printf("Insufficient memory to reallocate the terminals array!\n");
				return x;
			}
		}
		pp->pTables->aTerminals[x].bNullable = 0;
		pp->pTables->aTerminals[x].id = x;
		pp->pTables->aTerminals[x].Type = T_TERMINAL;
		pp->pTables->aTerminals[x].pFirstSet = NULL;
		pp->pTables->aTerminals[x].pFollowSet = NULL;
		pp->pTables->aTerminals[x].strLexeme = (char*)malloc(strlen(lexeme) + 1);
		if ( pp->pTables->aTerminals[x].strLexeme != NULL )
		{
			strcpy(pp->pTables->aTerminals[x].strLexeme, lexeme);
		}
		else
		{
			printf("Insufficient memory to allocate aNonTerminals[%d].strLexeme\n", x);
		}

		pp->pTables->countTerminals++;
	}
	else // type == T_NONTERMINAL
	{
		x = 0;
		while ( x < pp->pTables->countNonTerminals )
		{
			if ( strcmp(lexeme, pp->pTables->aNonTerminals[x].strLexeme) == 0 )
			{
				return x;
			}
			x++;
		}
		if ( x >= pp->pTables->countTempNonTerminals )
		{
			pp->pTables->countTempNonTerminals += 21;
			pp->pTables->aNonTerminals = (Symbol*)realloc(pp->pTables->aNonTerminals, (sizeof(Symbol))*pp->pTables->countTempNonTerminals);
			if ( !pp->pTables->aNonTerminals )
			{
				printf("Insufficient memory to reallocate non-terminal arrays!\n");
				return x;
			}
		}
		pp->pTables->aNonTerminals[x].bNullable = 0;
		pp->pTables->aNonTerminals[x].id = x;
		pp->pTables->aNonTerminals[x].Type = T_NONTERMINAL;
		pp->pTables->aNonTerminals[x].pFirstSet = NULL;
		pp->pTables->aNonTerminals[x].pFollowSet = NULL;
		pp->pTables->aNonTerminals[x].strLexeme = (char*)malloc(strlen(lexeme) + 1);
		if ( pp->pTables->aNonTerminals[x].strLexeme != NULL )
		{
			strcpy(pp->pTables->aNonTerminals[x].strLexeme, lexeme);
		}
		else
		{
			printf("Insufficient memory to allocate aNonTerminals[%d].strLexeme\n", x);
		}
		pp->pTables->countNonTerminals++;
	}

	return x;
}

void Panic(ParserParams *pp)
{ 
	pp->numErrors++;

	while ( pp->myToken.Type != T_SEMICOLON && pp->myToken.Type != T_EOF )
	{
		GetNextToken(&(pp->myToken), &(pp->sp));
	}

	if ( pp->myToken.Type == T_SEMICOLON )
	{
		GetNextToken(&(pp->myToken), &(pp->sp));
	}
}

int Parse(char *szFileName, Tables *pTables)
{
	int ret;
	ParserParams myParserParams;
	memset(&myParserParams, 0, sizeof(ParserParams));
	
	if ( !InitParser(&myParserParams) )
		return 0;
		
	if ( !ReadFile(szFileName, &(myParserParams.sp)) ) { FreeScanner(&(myParserParams.sp)); free(myParserParams.myToken.strAction); myParserParams.myToken.strAction = NULL; FreeParser(&myParserParams); return 0; }
					
	GetNextToken(&(myParserParams.myToken), &(myParserParams.sp));

	myParserParams.pTables = pTables;

	if ( myParserParams.myToken.Type == T_PROLOGUE )
	{
		if ( !pTables->bIgnoreActions && myParserParams.myToken.strAction )
			pTables->strPrologue = strdup(myParserParams.myToken.strAction);
		GetNextToken(&(myParserParams.myToken), &(myParserParams.sp));
	}
	if ( myParserParams.myToken.Type == T_PERCENT_PERCENT )
		GetNextToken(&(myParserParams.myToken), &(myParserParams.sp));

	while ( myParserParams.myToken.Type != T_EOF )
	{
		if ( myParserParams.myToken.Type == T_PERCENT_PERCENT )
		{
			if ( !pTables->bIgnoreActions )
				pTables->strEpilogue = strdup(myParserParams.sp.szInput + myParserParams.sp.pos);
			break;
		}
		ret = prods_list(&myParserParams);
		if ( !ret )
		{
			Panic(&myParserParams);
		}
	}

	ret = myParserParams.numErrors ? 0 : 1;

	if ( myParserParams.numErrors || myParserParams.numWarnings )
		printf("Errors: %d\nWarnings: %d\n", myParserParams.numErrors, myParserParams.numWarnings);

	free(myParserParams.myToken.strAction); myParserParams.myToken.strAction = NULL; FreeParser(&myParserParams);
	
	return ret;
}

int match(TokenTypeEnum ExpectedToken, ParserParams *pp)
{
	if ( pp->myToken.Type == ExpectedToken )
	{
		if ( pp->myToken.Type == T_TERMINAL )
		{
			FindOrInsertSymbol(pp->myToken.szValue, T_TERMINAL, pp);
		}
		else if ( pp->myToken.Type == T_NONTERMINAL )
		{
			FindOrInsertSymbol(pp->myToken.szValue, T_NONTERMINAL, pp);
		}

		GetNextToken(&(pp->myToken), &(pp->sp));
		return 1;
	}

	return 0;
}


/*
 * prods_list : {prod} prod : T_NONTERMINAL ':' rhs; rhs : symbols_list {'|' symbols_list} ';' symbols_list : {T_TERMINAL | T_NONTERMINAL}
 */

// prods_list : {prod}
int prods_list(ParserParams *pp)
{
	if ( pp->myToken.Type != T_NONTERMINAL )
	{
		printf("%s Line %d Column %d: prods_list error. Expected a non-terminal symbol.\n",
			pp->myToken.szValue,
			pp->myToken.line,
			pp->myToken.column);

		return 0;
	}

	while (pp->myToken.Type == T_NONTERMINAL)
	{
		if ( !prod(pp) )
			return 0;
	}

	return 1;
}

// prod : T_NONTERMINAL ':' rhs;
int prod(ParserParams *pp)
{
	int id;

	if ( !pp->numErrors )
	{
		if ( pp->myToken.Type == T_NONTERMINAL )
		{
			id = FindOrInsertSymbol(pp->myToken.szValue, T_NONTERMINAL, pp);
			pp->stack[++pp->top] = id + 10000;
		}
	}

	if ( !match(T_NONTERMINAL, pp) )
	{
		printf("Line %d Column %d: Production error. Non-terminal symbol expected.\n",
			pp->myToken.line,
			pp->myToken.column);
		return 0;
	}

	if ( !match(T_COLON, pp) )
	{
		printf("Line %d Column %d: Production error. Expected ':'.\n",
			pp->myToken.line,
			pp->myToken.column);
		return 0;
	}

	if ( !rhs(pp) )
		return 0;

	return 1;
}

// rhs : symbols_list {'|' symbols_list} ';'
int rhs(ParserParams *pp)
{
	int x;
	int id;
	int f;

	if ( !symbols_list(pp) )
		return 0;

	while ( pp->myToken.Type == T_VERTBAR || pp->myToken.Type == T_ACTION )
	{
		char *pendingAction = NULL;
		if ( pp->myToken.Type == T_ACTION )
		{
			if ( !pp->pTables->bIgnoreActions && pp->myToken.strAction )
				pendingAction = strdup(pp->myToken.strAction);
			GetNextToken(&(pp->myToken), &(pp->sp));
			if ( pp->myToken.Type != T_VERTBAR && pp->myToken.Type != T_SEMICOLON )
			{
				printf("Line %d Column %d: Expected '|' or ';' after action.\n", pp->myToken.line, pp->myToken.column);
				free(pendingAction);
				return 0;
			}
		}
		if ( pp->myToken.Type == T_SEMICOLON )
		{
			if ( pendingAction )
			{
				free(pp->myToken.strAction);
				pp->myToken.strAction = pendingAction;
			}
			break;
		}
		if ( !pp->numErrors )
		{
			id = pp->stack[0];
			id -= 10000;
			strcpy(pp->strTemp, pp->pTables->aNonTerminals[id].strLexeme);
			strcat(pp->strTemp, " -> ");
			for ( x = 1; x <= pp->top; x++ )
			{
				id = pp->stack[x];
				if ( id >= 10000 ) // Non-terminal
				{
					id -= 10000;
					strcat(pp->strTemp, pp->pTables->aNonTerminals[id].strLexeme);
					strcat(pp->strTemp, " ");

				}
				else // Terminal 
				{
					strcat(pp->strTemp, pp->pTables->aTerminals[id].strLexeme);
					strcat(pp->strTemp, " ");
				}				
			}
			f = htFind(pp->pHT, pp->strTemp);
			if ( f == -1 ) // Production not present. Proceeding to add it to the array of productions..
			{
				htInsert(pp->pHT, pp->strTemp);

				x = pp->pTables->countProds;
				if ( x >= pp->pTables->countTempProds )
				{
					pp->pTables->countTempProds += 21;
					pp->pTables->aProds = (ArrayProduction*)realloc(pp->pTables->aProds, (sizeof(ArrayProduction))*pp->pTables->countTempProds);
					if ( !pp->pTables->aProds )
					{
						printf("Insufficient memory to reallocate productions array!\n");
						return 0;
					}
				}
				pp->pTables->aProds[x].count = pp->top + 1;
				pp->pTables->aProds[x].aSymbols = (Symbol *)malloc(sizeof(Symbol) * (pp->top + 1));
				if ( !pp->pTables->aProds[x].aSymbols )
				{
					printf("Insufficient memory for production array.\n");
					return 0;
				}
				while ( pp->top > 0 )
				{
					id = pp->stack[pp->top];
					if ( id >= 10000 ) // Non-Terminal
					{
						id -= 10000;
						pp->pTables->aProds[x].aSymbols[pp->top] = pp->pTables->aNonTerminals[id];
					}
					else // Terminal
					{
						pp->pTables->aProds[x].aSymbols[pp->top] = pp->pTables->aTerminals[id];
					}
					pp->top--;
				}
				id = pp->stack[0];
				pp->pTables->aProds[x].aSymbols[0] = pp->pTables->aNonTerminals[id - 10000];

				pp->pTables->aProds[x].strAction = pendingAction;
				pendingAction = NULL;
				if ( !pp->pTables->bIgnoreActions && pp->myToken.strAction )
					pp->pTables->aProds[x].strAction = strdup(pp->myToken.strAction);
				pp->pTables->countProds++;
			}
			else // Production already present
			{
				free(pendingAction);
				pendingAction = NULL;
				printf("Warning: The production \"%s\" is already defined. Duplicates are discarded.\n", pp->strTemp);
				pp->numWarnings++;
				pp->top = 0;
			}
		}

		//match(T_VERTBAR, pp);
		GetNextToken(&(pp->myToken), &(pp->sp));		

		if ( !symbols_list(pp) )
			return 0;
	}

	if ( pp->myToken.Type == T_SEMICOLON )
	{
		if ( !pp->numErrors )
		{
			id = pp->stack[0];
			id -= 10000;
			strcpy(pp->strTemp, pp->pTables->aNonTerminals[id].strLexeme);
			strcat(pp->strTemp, " -> ");
			for ( x = 1; x <= pp->top; x++ )
			{
				id = pp->stack[x];
				if ( id >= 10000 ) // Non-Terminal
				{
					id -= 10000;
					strcat(pp->strTemp, pp->pTables->aNonTerminals[id].strLexeme);
					strcat(pp->strTemp, " ");

				}
				else // Terminal
				{
					strcat(pp->strTemp, pp->pTables->aTerminals[id].strLexeme);
					strcat(pp->strTemp, " ");
				}				
			}
			f = htFind(pp->pHT, pp->strTemp);
			if ( f == -1 ) // Production not present. Proceeding to add it to the array of productions..
			{
				htInsert(pp->pHT, pp->strTemp);

				x = pp->pTables->countProds;
				if ( x >= pp->pTables->countTempProds )
				{
					pp->pTables->countTempProds += 21;
					pp->pTables->aProds = (ArrayProduction*)realloc(pp->pTables->aProds, (sizeof(ArrayProduction))*pp->pTables->countTempProds);
					if ( !pp->pTables->aProds )
					{
						printf("Insufficient memory to reallocate productions array!\n");
						return 0;
					}
				}
				pp->pTables->aProds[x].count = pp->top + 1;
				pp->pTables->aProds[x].aSymbols = (Symbol *)malloc(sizeof(Symbol) * (pp->top + 1));
				if ( !pp->pTables->aProds[x].aSymbols )
				{
					printf("Insufficient memory for production array.\n");
					return 0;
				}
				while ( pp->top > 0 )
				{
					id = pp->stack[pp->top];
					if ( id >= 10000 ) // Non-Terminal
					{
						id -= 10000;
						pp->pTables->aProds[x].aSymbols[pp->top] = pp->pTables->aNonTerminals[id];
					}
					else // Terminal 
					{
						pp->pTables->aProds[x].aSymbols[pp->top] = pp->pTables->aTerminals[id];
					}

					pp->top--;
				}
				id = pp->stack[0];
				pp->pTables->aProds[x].aSymbols[0] = pp->pTables->aNonTerminals[id - 10000];

				pp->top--;

				pp->pTables->countProds++;
			}
			else // Production already present
			{
				printf("Warning: the production \"%s\" is already defined. Duplicates are being discarded.\n", pp->strTemp);
				pp->numWarnings++;
				pp->top = -1;
			}
		}
	}

	if ( !match(T_SEMICOLON, pp) )
	{
		printf("Line %d Column %d: RHS error. Expected ';'.\n",
			pp->myToken.line,
			pp->myToken.column);
		return 0;
	}

	return 1;
}

// symbols_list: {T_TERMINAL | T_NONTERMINAL}
int symbols_list(ParserParams *pp)
{
	int id;

	while ( pp->myToken.Type == T_TERMINAL || pp->myToken.Type == T_NONTERMINAL )
	{
		if ( pp->myToken.Type == T_TERMINAL )
		{
			id = FindOrInsertSymbol(pp->myToken.szValue, T_TERMINAL, pp);
			pp->stack[++pp->top] = id;
		}
		else // NON_TERMINAL
		{
			id = FindOrInsertSymbol(pp->myToken.szValue, T_NONTERMINAL, pp);
			pp->stack[++pp->top] = id + 10000;
		}

		GetNextToken(&(pp->myToken), &(pp->sp));		
		if ( pp->myToken.Type == T_SCAN_ERROR || pp->myToken.Type == T_UNKNOWN )
		{
			pp->numErrors++;
		}
	}

	return 1;
}
