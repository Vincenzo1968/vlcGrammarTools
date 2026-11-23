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

#ifndef MYPARSER_H
#define MYPARSER_H

#include "scanner.h"
#include "ast.h"

typedef struct tagParserParams
{
	Token myToken;
	ScannerParams sp;

	int numErrors;
	int numWarnings;

	Tables *pTables;

	int stack[512];
	int top;

	HashTable** pHT;
	char strTemp[512];
} ParserParams;

int InitParser(ParserParams *pp);

int Parse(char *szFileName, Tables *pTables);

int match(TokenTypeEnum ExpectedToken, ParserParams *pp);

int FindOrInsertSymbol(char *lexeme, int type, ParserParams *pp);

void Panic(ParserParams *pp);

int prods_list(ParserParams *pp);
int prod(ParserParams *pp);
int rhs(ParserParams *pp);
int symbols_list(ParserParams *pp);

#endif // MYPARSER_H
