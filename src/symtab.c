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
#include <string.h>
#include <malloc.h>

#include "symtab.h"

// -------------------------- HashTable -----------------------------------------

HashTable* htNewNode(char *Key)
{
	HashTable *n;

	n = (HashTable*)malloc(sizeof(HashTable));

	if( n == NULL )
		return NULL;

	n->Key = (char*)malloc(strlen(Key)+1);
	if ( n->Key == NULL )
	{
		free(n);
		return NULL;
	}
	strcpy(n->Key, Key);
	n->next = NULL;

	return n;
}

int htFind(HashTable **pHashTable, char *Key)
{
	int index = 0;
	HashTable *t;
	int a = 31415;
	//int b = 27183;
	char *s = Key;

	for(; *s != '\0'; s++)
		index = (a*index + *s) % HT_SIZE;
	if ( index < 0 )
		index *= -1;

	t = pHashTable[index];
	while ( t != NULL )
	{
		if ( strcmp(t->Key, Key) == 0 )
		{
			return index;
		}
		t = t->next;
	}

	return -1;
}

void htInsert(HashTable **pHashTable, char *Key)
{
	int index = 0;
	HashTable *t = NULL;
	int a = 31415;
	//int b = 27183;
	char *s = Key;

	for(; *s != '\0'; s++)
		index = (a*index + *s) % HT_SIZE;
	if ( index < 0 )
		index *= -1;

	t = pHashTable[index];
	if ( t == NULL )
	{
		pHashTable[index] = htNewNode(Key);
		return;
	}

	while ( t != NULL )
	{
		if ( strcmp(t->Key, Key) == 0 )
		{
			printf("\nError: The key %s is already present in the hashtable\n", Key);
			return;
		}
		if ( t->next == NULL )
		{
			t->next = htNewNode(Key);
			return;
		}
		t = t->next;
	}
}

void htFree(HashTable* first)
{
	HashTable *n1 = first, *n2;

	while ( n1 != NULL )
	{
		n2 = n1->next;
		if ( n1->Key )
			free(n1->Key);

		free(n1);
		n1 = n2;
	}
}

