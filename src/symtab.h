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

#ifndef SYMTAB_H
#define SYMTAB_H

#define HT_SIZE 251
//#define HT_SIZE 509
//#define HT_SIZE 1021
//#define HT_SIZE 2039
//#define HT_SIZE 4093
//#define HT_SIZE 8191
//#define HT_SIZE 16381
//#define HT_SIZE 32749

typedef struct tagHashTable
{
	char *Key;
	struct tagHashTable *next;
} HashTable;

HashTable* htNewNode(char *Key);
int htFind(HashTable **pHashTable, char *Key);
void htInsert(HashTable **pHashTable, char *Key);
void htFree(HashTable* first);

#endif // SYMTAB_H
