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
#include <string.h>
#include <malloc.h>
#include "scanner.h"

int InitScanner(Token *pToken, ScannerParams *pParams)
{
	pToken->szValue[0] = '\0';
	pToken->lexeme[0]  = '\0';
	pToken->strAction  = NULL;

	pParams->szInput = NULL;
	pParams->pos = 0;
	pParams->line = 1;
	pParams->column = 1;
	pParams->bStartSymbol = 1;
	pParams->bNonTerminalLower = 1;

	return 1;
}

void FreeScanner(ScannerParams *pParams)
{
	if ( pParams->szInput != NULL )
		free(pParams->szInput);
	pParams->szInput = NULL;
}

int ReadFile(char *szFileName, ScannerParams *p)
{
	FILE *fp;
	long dimFile = 0;
	size_t bytereads = 0;

	if ( p->szInput != NULL )
	{
		free(p->szInput);
		p->szInput = NULL;
	}

	fp = fopen(szFileName, "rb");
	if ( fp == NULL )
	{
		printf("Error opening file '%s'.\n", szFileName);
		return 0;
	}

	if ( fseek(fp, 0, SEEK_END) )
	{
		fclose(fp);
		return 0;
	}

	dimFile = ftell(fp);
	if ( dimFile <= 0 )
	{
		fclose(fp);
		return 0;
	}

	p->szInput = (char *)malloc((size_t)dimFile + 1);
	if ( !p->szInput )
	{
		printf("Error: insufficient memory.\n");
		fclose(fp);
		p->szInput = NULL;
		return 0;
	}

	fseek(fp, 0, SEEK_SET);

	bytereads = fread(p->szInput, 1, (size_t)dimFile, fp);
	p->szInput[bytereads] = '\0';

	fclose(fp);

	return 1;
}

static char* ReadBalancedCodeBlock(ScannerParams *p, int isPrologue, int *pErrLine, int *pErrCol)
{
	size_t cap = 256;
	size_t len = 0;
	char *buf = (char *)malloc(cap);
	if (!buf) return NULL;
	buf[0] = '\0';

	int depth = 1;
	int inStr = 0;
	int inChar = 0;
	int inLineComment = 0;
	int inBlockComment = 0;

	while (1)
	{
		char c = *(p->szInput + p->pos);
		if (c == '\0')
		{
			*pErrLine = p->line;
			*pErrCol = p->column;
			free(buf);
			return NULL;
		}

		p->pos++;
		p->column++;
		if (c == '\n')
		{
			p->line++;
			p->column = 1;
			inLineComment = 0;
		}

		if (inLineComment)
		{
			/* continua */
		}
		else if (inBlockComment)
		{
			if (c == '*' && *(p->szInput + p->pos) == '/')
			{
				if (len + 3 >= cap)
				{
					cap *= 2;
					char *tmp = (char *)realloc(buf, cap);
					if (!tmp) { free(buf); return NULL; }
					buf = tmp;
				}
				buf[len++] = c;
				buf[len++] = *(p->szInput + p->pos);
				p->pos++;
				p->column++;
				inBlockComment = 0;
				continue;
			}
		}
		else if (inStr)
		{
			if (c == '\\' && *(p->szInput + p->pos) != '\0')
			{
				if (len + 3 >= cap)
				{
					cap *= 2;
					char *tmp = (char *)realloc(buf, cap);
					if (!tmp) { free(buf); return NULL; }
					buf = tmp;
				}
				buf[len++] = c;
				buf[len++] = *(p->szInput + p->pos);
				if (*(p->szInput + p->pos) == '\n') { p->line++; p->column = 1; }
				else { p->column++; }
				p->pos++;
				continue;
			}
			if (c == '"') inStr = 0;
		}
		else if (inChar)
		{
			if (c == '\\' && *(p->szInput + p->pos) != '\0')
			{
				if (len + 3 >= cap)
				{
					cap *= 2;
					char *tmp = (char *)realloc(buf, cap);
					if (!tmp) { free(buf); return NULL; }
					buf = tmp;
				}
				buf[len++] = c;
				buf[len++] = *(p->szInput + p->pos);
				if (*(p->szInput + p->pos) == '\n') { p->line++; p->column = 1; }
				else { p->column++; }
				p->pos++;
				continue;
			}
			if (c == '\'') inChar = 0;
		}
		else
		{
			if (c == '/' && *(p->szInput + p->pos) == '/') inLineComment = 1;
			else if (c == '/' && *(p->szInput + p->pos) == '*') inBlockComment = 1;
			else if (c == '"') inStr = 1;
			else if (c == '\'') inChar = 1;
			else if (isPrologue && c == '%' && *(p->szInput + p->pos) == '}')
			{
				p->pos++;
				p->column++;
				break;
			}
			else if (!isPrologue && c == '{')
			{
				depth++;
			}
			else if (!isPrologue && c == '}')
			{
				depth--;
				if (depth == 0) break;
			}
		}

		if (len + 2 >= cap)
		{
			cap *= 2;
			char *tmp = (char *)realloc(buf, cap);
			if (!tmp) { free(buf); return NULL; }
			buf = tmp;
		}
		buf[len++] = c;
	}

	buf[len] = '\0';
	return buf;
}

void GetNextToken(Token *pToken, ScannerParams *p)
{
	if (pToken->strAction) { free(pToken->strAction); pToken->strAction = NULL; }
	States state = S0;
	char c;
	int k = 0;
	int maxLex = (int)sizeof(pToken->lexeme) - 1;

	if ( p->szInput == NULL )
	{
		printf("GetNextToken Error: NULL Input\n");
		pToken->Type = T_SCAN_ERROR;
		return;
	}

	c = *(p->szInput + p->pos);

	while ( c == ' ' || c == '\t' || c == '\r' || c == '\n' )
	{
		if ( c == '\n' )
		{
			p->line++;
			p->column = 1;
		}
		else
		{
			p->column++;
		}
		p->pos++;
		c = *(p->szInput + p->pos);
	}

	if ( c == '\0' )
	{
		pToken->Type = T_EOF;
		return;
	}

	while ( 1 )
	{
		c = *(p->szInput + p->pos++);

		switch(state)
		{
		case S0:
			k = 0;
			pToken->line = p->line;
			pToken->column = p->column;

			if ( c == '\0' )
			{
				pToken->Type = T_EOF;
				return;
			}
			else if ( c == ':' )
			{
				pToken->Type = T_COLON;
				return;
			}
			else if ( c == '|' )
			{
				pToken->Type = T_VERTBAR;
				return;
			}
			else if ( c == ';' )
			{
				pToken->Type = T_SEMICOLON;
				return;
			}
			else if ( c == '%' )
			{
				char nextC = *(p->szInput + p->pos);
				if ( nextC == '{' )
				{
					p->pos++;
					p->column++;
					int errL = p->line, errC = p->column;
					char *blk = ReadBalancedCodeBlock(p, 1, &errL, &errC);
					if (!blk)
					{
						pToken->Type = T_SCAN_ERROR;
						printf("Scanner error: Line %d Column %d: Expected '%%}', encountered EOF.\n", errL, errC);
						return;
					}
					pToken->Type = T_PROLOGUE;
					pToken->strAction = blk;
					return;
				}
				else if ( nextC == '%' )
				{
					p->pos++;
					p->column++;
					pToken->Type = T_PERCENT_PERCENT;
					strcpy(pToken->lexeme, "%%");
					strcpy(pToken->szValue, "%%");
					return;
				}
				else
				{
					pToken->Type = T_SCAN_ERROR;
					printf("Scanner error: Line %d Column %d: Unexpected character '%%'.\n", p->line, p->column);
					return;
				}
			}
			else if ( c == '{' )
			{
				state = S7;
			}
			else if ( c == '/' )
			{
				state = S8;
			}
			else if ( c == '\'' )
			{
				state = S2;
			}
			else if ( c == '"' )
			{
				state = S5;
			}
			else if ( ((c >= 'a' && c <= 'z')) || ((c >= 'A') && (c <= 'Z')) )
			{
				if (k < maxLex) pToken->lexeme[k++] = c;

				if ( p->bStartSymbol )
				{
					p->bNonTerminalLower = (c >= 'a' && c <= 'z') ? 1 : 0;
					p->bStartSymbol = 0;
				}

				state = S1;
			}
			else
			{
				pToken->Type = T_UNKNOWN;
				pToken->szValue[0] = c;
				pToken->szValue[1] = '\0';

				k = 0;
				pToken->lexeme[k++] = c;

				while (1)
				{
					char nextC = *(p->szInput + p->pos);
					if ( nextC == ' ' || nextC == '\t' || nextC == '\r' || nextC == '\n' ||
					     nextC == ';' || nextC == '|' || nextC == '\0' )
					{
						break;
					}
					p->pos++;
					p->column++;
					if ( k < maxLex )
						pToken->lexeme[k++] = nextC;
				}

				pToken->lexeme[k] = '\0';
				pToken->line = p->line;
				pToken->column = p->column;

				printf("GetNextToken Error: Line %d Column %d: Invalid identifier '%s'.\n",
				       pToken->line, pToken->column, pToken->lexeme);

				return;
			}
			break;
		case S1:
			if ( ((c >= 'a' && c <= 'z')) || ((c >= 'A') && (c <= 'Z')) ||
			     ((c >= '0') && (c <= '9')) || (c == '_') )
			{
				if (k < maxLex) pToken->lexeme[k++] = c;
			}
			else
			{
				pToken->lexeme[k] = '\0';

				if ( p->bNonTerminalLower )
				{
					pToken->Type = (pToken->lexeme[0] >= 'a' && pToken->lexeme[0] <= 'z')
					               ? T_NONTERMINAL : T_TERMINAL;
				}
				else
				{
					pToken->Type = (pToken->lexeme[0] >= 'a' && pToken->lexeme[0] <= 'z')
					               ? T_TERMINAL : T_NONTERMINAL;
				}

				pToken->line = p->line;
				pToken->column = p->column;
				snprintf(pToken->szValue, sizeof(pToken->szValue), "%s", pToken->lexeme);
				--p->pos;

				return;
			}
			break;
		case S2:
			if ( c == '\\' )
			{
				state = S3;
			}
			else
			{
				if ( c == '\n' )
				{
					p->line++;
					p->column = 1;
				}

				pToken->lexeme[0] = c;
				state = S4;
			}
			break;
		case S3:
			if ( c == '\\' || c == '\'' )
			{
				pToken->lexeme[0] = c;
				state = S4;
			}
			else if ( c == 'r' ) { pToken->lexeme[0] = '\r'; state = S4; }
			else if ( c == 'n' ) { pToken->lexeme[0] = '\n'; state = S4; }
			else if ( c == 't' ) { pToken->lexeme[0] = '\t'; state = S4; }
			else
			{
				if ( c == '\n' )
				{
					p->line++;
					p->column = 1;
				}

				pToken->Type = T_SCAN_ERROR;
				printf("Scanner Error: Line %d Column %d: Invalid character %c\nExpected: newline, tab, or backslash\n", pToken->line, pToken->column, c);
				return;
			}
			break;
		case S4:
			if ( c == '\'' )
			{
				pToken->Type = T_TERMINAL;
				pToken->lexeme[1] = '\0';
				strcpy(pToken->szValue, pToken->lexeme);
				return;
			}
			else
			{
				pToken->Type = T_SCAN_ERROR;
				printf("Scanner error: Line %d Column %d: invalid character %c\nExpected: closing quote\n", pToken->line, pToken->column, c);
				return;
			}
			break;
		case S5:
			if ( c == '\\' )
			{
				state = S6;
			}
			else if ( c == '"' )
			{
				pToken->lexeme[k] = '\0';
				pToken->Type = T_TERMINAL;
				snprintf(pToken->szValue, sizeof(pToken->szValue), "%s", pToken->lexeme);
				return;
			}
			else if ( c == '\n' )
			{
				p->line++;
				p->column = 1;

				pToken->Type = T_SCAN_ERROR;
				printf("Scanner error: Line %d Column %d: The string must end on the same line.\n", pToken->line, pToken->column);
				return;
			}
			else if ( c == '\0' )
			{
				pToken->Type = T_SCAN_ERROR;
				printf("Scanner error: Line %d Column %d: Unterminated string literal.\n", pToken->line, pToken->column);
				--p->pos;
				return;
			}
			else
			{
				if (k < maxLex) pToken->lexeme[k++] = c;
			}
			break;
		case S6:
			if ( c == '"' ) { if (k < maxLex) pToken->lexeme[k++] = '"'; state = S5; }
			else if ( c == '\\' ) { if (k < maxLex) pToken->lexeme[k++] = '\\'; state = S5; }
			else if ( c == 't' ) { if (k < maxLex) pToken->lexeme[k++] = '\t'; state = S5; }
			else if ( c == 'r' ) { if (k < maxLex) pToken->lexeme[k++] = '\r'; state = S5; }
			else if ( c == 'n' ) { if (k < maxLex) pToken->lexeme[k++] = '\n'; state = S5; }
			else if ( c == '\n' )
			{
				p->line++;
				p->column = 1;

				pToken->Type = T_SCAN_ERROR;
				printf("Scanner error: Line %d Column %d: The string must end on the same line.\n", pToken->line, pToken->column);
				return;
			}
			else
			{
				pToken->Type = T_SCAN_ERROR;
				printf("Scanner error: Line %d, Column %d: Unrecognized special character '\\%c'.\n", pToken->line, pToken->column, c);
				return;
			}
			break;
		case S7:
		{
			p->pos--;
			p->column--;
			int errL = p->line, errC = p->column;
			char *blk = ReadBalancedCodeBlock(p, 0, &errL, &errC);
			if (!blk)
			{
				pToken->Type = T_SCAN_ERROR;
				printf("Scanner error: Line %d Column %d: Expected '}', encountered EOF.\n", errL, errC);
				return;
			}
			pToken->Type = T_ACTION;
			pToken->strAction = blk;
			strcpy(pToken->lexeme, "{...}");
			strcpy(pToken->szValue, "{...}");
			return;
		}
		case S8:
			if ( c == '/' ) state = S9;
			else if ( c == '*' ) state = S10;
			else
			{
				pToken->Type = T_SCAN_ERROR;
				printf("Scanner error: Line %d, Column %d: invalid character %c.\n", pToken->line, pToken->column, c);
				--p->pos;
				return;
			}
			break;
		case S9:
			if ( c == '\r' || c == '\n' )
			{
				while ( c == ' ' || c == '\t' || c == '\r' || c == '\n' )
				{
					if ( c == '\n' )
					{
						p->line++;
						p->column = 1;
					}
					else
					{
						p->column++;
					}
					p->pos++;
					c = *(p->szInput + p->pos);
				}

				if ( c == '\0' )
				{
					pToken->Type = T_EOF;
					return;
				}

				state = S0;
			}
			else if ( c == '\0' )
			{
				pToken->Type = T_EOF;
				return;
			}
			break;
		case S10:
			if ( c == '*' )
				state = S11;
			else if ( c == '\0' )
			{
				pToken->Type = T_SCAN_ERROR;
				printf("Scanner error: Line %d, Column %d: expected '*/' to end the comment.\nReached end of file.\n", pToken->line, pToken->column);
				--p->pos;
				return;
			}
			break;
		case S11:
			if ( c == '/' )
			{
				c = *(p->szInput + p->pos);
				if ( c == ' ' || c == '\t' || c == '\r' || c == '\n' )
				{
					while ( c == ' ' || c == '\t' || c == '\r' || c == '\n' )
					{
						if ( c == '\n' )
						{
							p->line++;
							p->column = 1;
						}
						else
						{
							p->column++;
						}
						p->pos++;
						c = *(p->szInput + p->pos);
					}

					if ( c == '\0' )
					{
						pToken->Type = T_EOF;
						return;
					}
				}

				state = S0;
			}
			else if ( c == '\0' )
			{
				pToken->Type = T_SCAN_ERROR;
				printf("Scanner error: Line %d, Column %d: expected '*/' to end the comment.\nReached end of file.\n", pToken->line, pToken->column);
				--p->pos;
				return;
			}
			else
			{
				state = S10;
			}
			break;
		}

		p->column++;
	}
}
