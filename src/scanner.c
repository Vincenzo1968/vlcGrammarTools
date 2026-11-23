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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include "scanner.h"

int InitScanner(Token *pToken, ScannerParams *pParams)
{
	pToken->szValue[0] = '\0';
	pToken->lexeme[0]  = '\0';
	
	pParams->szInput = NULL;

	pParams->pos = 0;

	pParams->line = 1;
	pParams->column = 1;	
	
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
	int dimFile = 0;
	int bytereads = 0;

	if ( p->szInput != NULL )
		free(p->szInput);

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

	p->szInput = (char*)malloc(sizeof(char)*dimFile + 1);
	if ( !p->szInput )
	{
		printf("Error: insufficient memory.\n");
		fclose(fp);
		p->szInput = NULL;
		return 0;
	}

	fseek(fp, 0, SEEK_SET);

	bytereads = fread(p->szInput, 1, dimFile, fp);
	*(p->szInput + bytereads) = '\0';

	fclose(fp);

	return 1;
}

void GetNextToken(Token *pToken, ScannerParams *p)
{
	States state = S0;
	char c;
	int k = 0;

	static int bStartSymbol = 1;
	static int bNonTerminalLower = 1;

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
			else if ( c == '{' )
			{
				state = S7;
			}
			else if ( c == '/' )
			{
				state = S8;
			}
			else if ( c == '\'' ) // Terminal char literal
			{
				state = S2;
			}
			else if ( c == '"' ) // Terminal string literal
			{
				state = S5;
			}
			else if ( ((c >= 'a' && c <= 'z')) || ((c >= 'A') && (c <= 'Z')) )
			{
				pToken->lexeme[k++] = c;

				if ( bStartSymbol )
				{
					if ( c >= 'a' && c <= 'z' )
					{
						bNonTerminalLower = 1;
					}
					else
					{
						bNonTerminalLower = 0;
					}
					bStartSymbol = 0;
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

				while ( c != ' ' && c != '\t' && c != '\r' && c != '\n' &&
					    c != ';' && c != '|' && c != '\0' 
					  )
				{
					c = *(p->szInput + p->pos);
					p->pos++;
					pToken->lexeme[k++] = c;

					p->column++;

					if ( c == '\n' )
					{
						p->line++;
						p->column = 1;
					}

					if ( c == '\0' )
					{
						--p->pos;
					}
				}

				k--;
				p->pos--;

				pToken->lexeme[k] = '\0';
				pToken->line = p->line;
				pToken->column = p->column;

				printf("GetNextToken Error: Line %d Column %d: Invalid identifier '%s'.\n", pToken->line, pToken->column, pToken->lexeme);

				return;
			}
			break;
		case S1:
			if ( ((c >= 'a' && c <= 'z')) || ((c >= 'A') && (c <= 'Z')) || ((c >= '0') && (c <= '9')) )
			{
				pToken->lexeme[k++] = c;
			}
			else if ( c == '_' )
			{
				pToken->lexeme[k++] = c;
			}
			else
			{
				pToken->lexeme[k] = '\0';

				if ( bNonTerminalLower )
				{
					if ( (pToken->lexeme[0] >= 'a' && pToken->lexeme[0] <= 'z') )
					{
						pToken->Type = T_NONTERMINAL; // Non-terminals begin with a lowercase character
					}
					else
					{
						pToken->Type = T_TERMINAL; // Terminals begin with an uppercase character
					}
				}
				else
				{
					if ( (pToken->lexeme[0] >= 'a' && pToken->lexeme[0] <= 'z') )
					{
						pToken->Type = T_TERMINAL; // Terminals begin with an uppercase character
					}
					else
					{
						pToken->Type = T_NONTERMINAL; // Non-terminals begin with a lowercase character
					}
				}

				pToken->line = p->line;
				pToken->column = p->column;
				strcpy(pToken->szValue, pToken->lexeme);
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
			if ( c == '\\' )
			{
				pToken->lexeme[0] = c;
				state = S4;
			}
			else if ( c == '\'' )
			{
				pToken->lexeme[0] = c;
				state = S4;
			}
			else if ( c == 'r' )
			{
				pToken->lexeme[0] = '\r';
				state = S4;
			}
			else if ( c == 'n' )
			{
				pToken->lexeme[0] = '\n';
				state = S4;
			}
			else if ( c == 't' )
			{
				pToken->lexeme[0] = '\t';
				state = S4;
			}
			else
			{
				if ( c == '\n' )
				{
					p->line++;
					p->column = 1;
				}

				pToken->Type = T_SCAN_ERROR;
				printf("Scanner Error: Line %d Column %d: Invalid character %c\nExpexted: newline, tab, or backslash\n", pToken->line, pToken->column, c);
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
				strcpy(pToken->szValue, pToken->lexeme);
				
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
			else
			{
				pToken->lexeme[k++] = c;
			}
			break;
		case S6:
			if ( c == '"' )
			{
				pToken->lexeme[k++] = '"';
				state = S5;
			}
			else if ( c == '\\' )
			{
				pToken->lexeme[k++] = '\\';
				state = S5;
			}
			else if ( c == 't' )
			{
				pToken->lexeme[k++] = '\t';
				state = S5;
			}
			else if ( c == 'r' )
			{
				pToken->lexeme[k++] = '\r';
				state = S5;
			}
			else if ( c == 'n' )
			{
				pToken->lexeme[k++] = '\n';
				state = S5;
			}
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
			while ( c != '}' )
			{
				if ( c == '\0' )
				{
					pToken->Type = T_SCAN_ERROR;
					printf("Scanner error: Line %d Column %d: Expected '}', encountered EOF.\n", pToken->line, pToken->column);
					--p->pos;
					return;
				}

				p->pos++;
				c = *(p->szInput + p->pos);

				p->column++;

				if ( c == '\n' )
				{
					p->line++;
					p->column = 1;
				}
			}

			state = S0;

			break;
		case S8:
			if ( c == '/' ) // Comment until the end of the line
			{
				state = S9;
			}
			else if ( c == '*' ) // Multi-line comment up to '*/'
			{
				state = S10;
			}
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
			{
				state = S11;
			}
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
		} // end switch

		p->column++;
	} // end while
}

