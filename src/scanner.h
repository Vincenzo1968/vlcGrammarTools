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

#ifndef MYSCANNER_H
#define MYSCANNER_H

typedef enum tagStates
{
	S0,
	S1,
	S2,
	S3,
	S4,
	S5,
	S6,
	S7,
	S8,
	S9,
	S10,
	S11
} States;

typedef enum tagTokenType
{
	T_SCAN_ERROR,
	T_UNKNOWN,
	T_EOF,
	T_COLON,			// ':'
	T_SEMICOLON,		// ';'
	T_VERTBAR,			// '|'
	T_TERMINAL,
	T_NONTERMINAL
}TokenTypeEnum;

typedef struct tagToken
{
	TokenTypeEnum Type;
	
	char szValue[512];

	char lexeme[512]; // used only for numeric constants.

	int line;
	int column;
}Token;

typedef struct tagScannerParams
{
	char *szInput;

	int pos;

	int line;
	int column;
} ScannerParams;

int InitScanner(Token *pToken, ScannerParams *p);
void FreeScanner(ScannerParams *p);

int ReadFile(char *szFileName, ScannerParams *p);

void GetNextToken(Token *pToken, ScannerParams *p);

#endif // MYSCANNER_H
