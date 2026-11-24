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

#define _POSIX_C_SOURCE 200809L
#include "lalr.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void InitConversionReport(ConversionReport *pRep) { memset(pRep, 0, sizeof(ConversionReport)); }

static char *SerializeGrammar(const Tables *pTables)
{
	size_t cap = 4096;
	size_t len = 0;
	char *buf = (char *)malloc(cap);
	if (!buf)
		return NULL;
	*buf = '\0';

	for (int i = 0; i < pTables->countProds; i++)
	{
		char line[1024];
		int pos = snprintf(line, sizeof(line), "%s -> ", pTables->aProds[i].aSymbols[0].strLexeme);
		if (pTables->aProds[i].count == 1)
		{
			pos += snprintf(line + pos, sizeof(line) - pos, "&epsilon;");
		}
		else
		{
			for (int j = 1; j < pTables->aProds[i].count; j++)
			{
				pos += snprintf(line + pos, sizeof(line) - pos, "%s ",
								pTables->aProds[i].aSymbols[j].strLexeme);
			}
		}
		pos += snprintf(line + pos, sizeof(line) - pos, "\n");

		if (len + pos + 1 >= cap)
		{
			cap *= 2;
			char *tmp = (char *)realloc(buf, cap);
			if (!tmp)
			{
				free(buf);
				return NULL;
			}
			buf = tmp;
		}
		memcpy(buf + len, line, pos + 1);
		len += pos;
	}
	return buf;
}

void AddConversionStep(ConversionReport *pRep, const char *title, const char *desc,
					   const Tables *pTables)
{
	ConversionStep *step = (ConversionStep *)calloc(1, sizeof(ConversionStep));
	if (!step)
		return;

	strncpy(step->title, title, sizeof(step->title) - 1);
	strncpy(step->description, desc, sizeof(step->description) - 1);
	if (pTables)
		step->grammarSnapshot = SerializeGrammar(pTables);

	if (!pRep->steps)
	{
		pRep->steps = step;
	}
	else
	{
		ConversionStep *cur = pRep->steps;
		while (cur->next)
			cur = cur->next;
		cur->next = step;
	}
}

void FreeConversionReport(ConversionReport *pRep)
{
	ConversionStep *cur = pRep->steps;
	while (cur)
	{
		ConversionStep *nxt = cur->next;
		free(cur->grammarSnapshot);
		free(cur);
		cur = nxt;
	}
	pRep->steps = NULL;
}

void InitLalrTables(LalrTables *pLalr) { memset(pLalr, 0, sizeof(LalrTables)); }

static void FreeLalrActionList(LalrAction *act)
{
	while (act)
	{
		LalrAction *nxt = act->next;
		free(act);
		act = nxt;
	}
}

void FreeLalrTables(LalrTables *pLalr, const Tables *pTables)
{
	if (pLalr->states)
	{
		for (int i = 0; i < pLalr->numStates; i++)
		{
			for (int j = 0; j < pLalr->states[i].numItems; j++)
			{
				free(pLalr->states[i].items[j].lookaheads);
			}
			free(pLalr->states[i].items);
			free(pLalr->states[i].transitions);
		}
		free(pLalr->states);
	}

	if (pLalr->actionTable && pTables)
	{
		int cols = pTables->countTerminals + 1;
		for (int i = 0; i < pLalr->numStates; i++)
		{
			if (pLalr->actionTable[i])
			{
				for (int j = 0; j < cols; j++)
				{
					FreeLalrActionList(pLalr->actionTable[i][j]);
				}
				free(pLalr->actionTable[i]);
			}
		}
		free(pLalr->actionTable);
	}

	if (pLalr->gotoTable)
	{
		for (int i = 0; i < pLalr->numStates; i++)
		{
			free(pLalr->gotoTable[i]);
		}
		free(pLalr->gotoTable);
	}
	memset(pLalr, 0, sizeof(LalrTables));
}

static int FindItemInState(const LalrState *st, int prodId, int dotPos)
{
	for (int i = 0; i < st->numItems; i++)
	{
		if (st->items[i].prodId == prodId && st->items[i].dotPos == dotPos)
			return i;
	}
	return -1;
}

static int AddOrMergeItem(LalrState *st, int prodId, int dotPos, const int *la,
						  int numTermPlusDollar)
{
	int idx = FindItemInState(st, prodId, dotPos);
	if (idx >= 0)
	{
		int changed = 0;
		if (la)
		{
			for (int t = 0; t < numTermPlusDollar; t++)
			{
				if (la[t] && !st->items[idx].lookaheads[t])
				{
					st->items[idx].lookaheads[t] = 1;
					changed = 1;
				}
			}
		}
		return changed;
	}

	if (st->numItems >= st->capItems)
	{
		st->capItems = st->capItems ? st->capItems * 2 : 16;
		st->items = (LalrItem *)realloc(st->items, st->capItems * sizeof(LalrItem));
	}

	LalrItem *it = &st->items[st->numItems++];
	it->prodId = prodId;
	it->dotPos = dotPos;
	it->lookaheads = (int *)calloc(numTermPlusDollar, sizeof(int));
	if (la)
	{
		memcpy(it->lookaheads, la, numTermPlusDollar * sizeof(int));
	}
	return 1;
}

static void ComputeSuffixFirst(const Tables *pTables, int prodId, int startIdx, const int *parentLA,
							   int *outLA)
{
	int numTerms = pTables->countTerminals;
	int dollarIdx = numTerms;
	memset(outLA, 0, (numTerms + 1) * sizeof(int));

	if (prodId < 0)
	{
		memcpy(outLA, parentLA, (numTerms + 1) * sizeof(int));
		return;
	}

	const ArrayProduction *p = &pTables->aProds[prodId];
	int allNullable = 1;

	for (int k = startIdx; k < p->count; k++)
	{
		Symbol sym = p->aSymbols[k];
		if (sym.Type == T_TERMINAL)
		{
			outLA[sym.id] = 1;
			allNullable = 0;
			break;
		}
		else
		{
			TerminalList *fl = pTables->aNonTerminals[sym.id].pFirstSet;
			while (fl)
			{
				if (fl->idSymbol >= 0 && fl->idSymbol < numTerms)
					outLA[fl->idSymbol] = 1;
				else if (fl->idSymbol == T_DOLLAR)
					outLA[dollarIdx] = 1;
				fl = fl->next;
			}
			if (!pTables->aNonTerminals[sym.id].bNullable)
			{
				allNullable = 0;
				break;
			}
		}
	}

	if (allNullable && parentLA)
	{
		for (int t = 0; t <= numTerms; t++)
		{
			if (parentLA[t])
				outLA[t] = 1;
		}
	}
}

static void ComputeClosure(const Tables *pTables, LalrState *st, int withLookahead)
{
	int numTerms = pTables->countTerminals;
	int *tempLA = (int *)malloc((numTerms + 1) * sizeof(int));
	int changed = 1;

	while (changed)
	{
		changed = 0;
		for (int i = 0; i < st->numItems; i++)
		{
			int prodId = st->items[i].prodId;
			int dotPos = st->items[i].dotPos;
			int nextType = -1;
			int nextId = -1;

			if (prodId == -1)
			{
				if (dotPos == 1)
				{
					nextType = T_NONTERMINAL;
					nextId = pTables->aProds[0].aSymbols[0].id;
				}
			}
			else
			{
				const ArrayProduction *p = &pTables->aProds[prodId];
				if (dotPos < p->count)
				{
					nextType = p->aSymbols[dotPos].Type;
					nextId = p->aSymbols[dotPos].id;
				}
			}

			if (nextType == T_NONTERMINAL)
			{
				if (withLookahead)
				{
					ComputeSuffixFirst(pTables, prodId, dotPos + 1, st->items[i].lookaheads,
									   tempLA);
				}
				else
				{
					memset(tempLA, 0, (numTerms + 1) * sizeof(int));
				}

				for (int pr = 0; pr < pTables->countProds; pr++)
				{
					if (pTables->aProds[pr].aSymbols[0].id == nextId)
					{
						if (AddOrMergeItem(st, pr, 1, tempLA, numTerms + 1))
						{
							changed = 1;
						}
					}
				}
			}
		}
	}
	free(tempLA);
}

static int SameLR0Core(const LalrState *a, const LalrState *b)
{
	if (a->numItems != b->numItems)
		return 0;
	for (int i = 0; i < a->numItems; i++)
	{
		if (FindItemInState(b, a->items[i].prodId, a->items[i].dotPos) < 0)
			return 0;
	}
	return 1;
}

static void AddAction(LalrTables *pLalr, int stateId, int termId, LalrActionType type, int val)
{
	LalrAction *head = pLalr->actionTable[stateId][termId];
	for (LalrAction *cur = head; cur != NULL; cur = cur->next)
	{
		if (cur->type == type && cur->value == val)
			return;
	}

	LalrAction *node = (LalrAction *)calloc(1, sizeof(LalrAction));
	node->type = type;
	node->value = val;

	if (!head)
	{
		pLalr->actionTable[stateId][termId] = node;
	}
	else
	{
		LalrAction *cur = head;
		while (cur->next)
			cur = cur->next;
		cur->next = node;
		pLalr->numConflicts++;
		pLalr->isLALR1 = 0;
	}
}

int BuildLALR1(Tables *pTables, LalrTables *pLalr)
{
	if (!pTables || pTables->countProds == 0)
		return 0;

	int numNT = pTables->countNonTerminals;
	int numT = pTables->countTerminals;
	int totalSyms = numNT + numT;

	pLalr->capStates = 32;
	pLalr->states = (LalrState *)calloc(pLalr->capStates, sizeof(LalrState));
	pLalr->numStates = 0;
	pLalr->numConflicts = 0;
	pLalr->isLALR1 = 1;

	LalrState *s0 = &pLalr->states[pLalr->numStates++];
	s0->id = 0;
	s0->transitions = (int *)malloc(totalSyms * sizeof(int));
	for (int k = 0; k < totalSyms; k++)
		s0->transitions[k] = -1;

	int *initLA = (int *)calloc(numT + 1, sizeof(int));
	initLA[numT] = 1;
	AddOrMergeItem(s0, -1, 1, initLA, numT + 1);
	free(initLA);

	ComputeClosure(pTables, s0, 0);

	for (int stIdx = 0; stIdx < pLalr->numStates; stIdx++)
	{
		for (int sym = 0; sym < totalSyms; sym++)
		{
			int isNT = (sym >= numT);
			int symId = isNT ? (sym - numT) : sym;

			LalrState candidate = {0};
			for (int it = 0; it < pLalr->states[stIdx].numItems; it++)
			{
				int pId = pLalr->states[stIdx].items[it].prodId;
				int dot = pLalr->states[stIdx].items[it].dotPos;

				int match = 0;
				if (pId == -1)
				{
					if (dot == 1 && isNT && symId == pTables->aProds[0].aSymbols[0].id)
						match = 1;
				}
				else
				{

					const ArrayProduction *p = &pTables->aProds[pId];
					if (dot < p->count && ((isNT && p->aSymbols[dot].Type == T_NONTERMINAL &&
											p->aSymbols[dot].id == symId) ||
										   (!isNT && p->aSymbols[dot].Type == T_TERMINAL &&
											p->aSymbols[dot].id == symId)))
					{
						match = 1;
					}
				}
				if (match)
				{
					AddOrMergeItem(&candidate, pId, dot + 1, NULL, numT + 1);
				}
			}
			if (candidate.numItems == 0)
				continue;
			ComputeClosure(pTables, &candidate, 0);
			int existing = -1;
			for (int k = 0; k < pLalr->numStates; k++)
			{
				if (SameLR0Core(&pLalr->states[k], &candidate))
				{
					existing = k;
					break;
				}
			}
			if (existing >= 0)
			{
				pLalr->states[stIdx].transitions[sym] = existing;
				for (int j = 0; j < candidate.numItems; j++)
					free(candidate.items[j].lookaheads);
				free(candidate.items);
			}
			else
			{
				if (pLalr->numStates >= pLalr->capStates)
				{
					pLalr->capStates *= 2;
					LalrState *tmp = (LalrState *)realloc(pLalr->states, pLalr->capStates * sizeof(LalrState));
					if (!tmp)
						return 0;
					pLalr->states = tmp;
				}

				int newId = pLalr->numStates++;
				candidate.id = newId;
				candidate.transitions = (int *)malloc(totalSyms * sizeof(int));
				for (int k = 0; k < totalSyms; k++)
					candidate.transitions[k] = -1;
				pLalr->states[newId] = candidate;
				pLalr->states[stIdx].transitions[sym] = newId;
			}
		}
	}
	int changed = 1;
	while (changed)
	{
		changed = 0;
		for (int stIdx = 0; stIdx < pLalr->numStates; stIdx++)
		{
			LalrState *st = &pLalr->states[stIdx];
			int beforeCount = st->numItems;
			ComputeClosure(pTables, st, 1);
			if (st->numItems != beforeCount)
				changed = 1;
			for (int it = 0; it < st->numItems; it++)
			{
				int pId = st->items[it].prodId;
				int dot = st->items[it].dotPos;
				int symIdx = -1;
				if (pId == -1)
				{
					if (dot == 1)
						symIdx = numT + pTables->aProds[0].aSymbols[0].id;
				}
				else if (dot < pTables->aProds[pId].count)
				{
					Symbol s = pTables->aProds[pId].aSymbols[dot];
					symIdx = (s.Type == T_NONTERMINAL) ? (numT + s.id) : s.id;
				}
				if (symIdx >= 0 && st->transitions[symIdx] >= 0)
				{
					int targetSt = st->transitions[symIdx];
					if (AddOrMergeItem(&pLalr->states[targetSt], pId, dot + 1,
									   st->items[it].lookaheads, numT + 1))
					{
						changed = 1;
					}
				}
			}
		}
	}
	
	if (pLalr->numStates <= 0)
	{
		return 0;   // or handle the error as you do elsewhere
	}
	size_t nStates = (size_t)pLalr->numStates;

	pLalr->actionTable = (LalrAction ***)calloc(nStates, sizeof(LalrAction **));
	pLalr->gotoTable   = (int **)calloc(nStates, sizeof(int *));
	
	for (int stIdx = 0; stIdx < pLalr->numStates; stIdx++)
	{
		pLalr->actionTable[stIdx] = (LalrAction **)calloc(numT + 1, sizeof(LalrAction *));
		pLalr->gotoTable[stIdx] = (int *)malloc(numNT * sizeof(int));
		for (int nt = 0; nt < numNT; nt++)
		{
			pLalr->gotoTable[stIdx][nt] = pLalr->states[stIdx].transitions[numT + nt];
		}
		LalrState *st = &pLalr->states[stIdx];
		for (int t = 0; t < numT; t++)
		{
			if (st->transitions[t] >= 0)
			{
				AddAction(pLalr, stIdx, t, ACT_SHIFT, st->transitions[t]);
			}
		}
		for (int it = 0; it < st->numItems; it++)
		{
			int pId = st->items[it].prodId;
			int dot = st->items[it].dotPos;
			if (pId == -1 && dot == 2)
			{
				AddAction(pLalr, stIdx, numT, ACT_ACCEPT, 0);
			}
			else if (pId >= 0 && dot == pTables->aProds[pId].count)
			{
				for (int t = 0; t <= numT; t++)
				{
					if (st->items[it].lookaheads[t])
					{
						AddAction(pLalr, stIdx, t, ACT_REDUCE, pId);
					}
				}
			}
		}
	}
	return 1;
}

static int AddNewNonTerminal(Tables *pTables, const char *baseName)
{
	char candidate[256];
	int suffix = 1;
	snprintf(candidate, sizeof(candidate), "%s_p", baseName);

	while (1)
	{
		int exists = 0;
		for (int i = 0; i < pTables->countNonTerminals; i++)
		{
			if (strcmp(pTables->aNonTerminals[i].strLexeme, candidate) == 0)
			{
				exists = 1;
				break;
			}
		}
		if (!exists)
			break;
		snprintf(candidate, sizeof(candidate), "%s_p%d", baseName, suffix++);
	}

	int idx = pTables->countNonTerminals;
	pTables->aNonTerminals = (Symbol *)realloc(pTables->aNonTerminals, (idx + 1) * sizeof(Symbol));
	memset(&pTables->aNonTerminals[idx], 0, sizeof(Symbol));
	pTables->aNonTerminals[idx].Type = T_NONTERMINAL;
	pTables->aNonTerminals[idx].id = idx;
	pTables->aNonTerminals[idx].strLexeme = strdup(candidate);
	pTables->countNonTerminals++;
	return idx;
}

static void AppendProduction(Tables *pTables, int lhsId, const Symbol *rhs, int rhsCount)
{
	int idx = pTables->countProds;
	pTables->aProds =
		(ArrayProduction *)realloc(pTables->aProds, (idx + 1) * sizeof(ArrayProduction));
	pTables->aProds[idx].count = 1 + rhsCount;
	pTables->aProds[idx].strAction = NULL;
	pTables->aProds[idx].aSymbols = (Symbol *)malloc((1 + rhsCount) * sizeof(Symbol));
	pTables->aProds[idx].aSymbols[0] = pTables->aNonTerminals[lhsId];
	for (int i = 0; i < rhsCount; i++)
	{
		pTables->aProds[idx].aSymbols[i + 1] = rhs[i];
	}
	pTables->countProds++;
}

static void RemoveProductionAt(Tables *pTables, int prodIdx)
{
	free(pTables->aProds[prodIdx].aSymbols);
	free(pTables->aProds[prodIdx].strAction);
	for (int i = prodIdx; i < pTables->countProds - 1; i++)
	{
		pTables->aProds[i] = pTables->aProds[i + 1];
	}
	pTables->countProds--;
}

static int EliminateLeftRecursion(Tables *pTables)
{
	int origNTCount = pTables->countNonTerminals;
	int anyChanged = 0;

	for (int i = 0; i < origNTCount; i++)
	{
		for (int j = 0; j < i; j++)
		{
			int p = 0;
			while (p < pTables->countProds)
			{
				if (pTables->aProds[p].aSymbols[0].id == i && pTables->aProds[p].count > 1 &&
					pTables->aProds[p].aSymbols[1].Type == T_NONTERMINAL &&
					pTables->aProds[p].aSymbols[1].id == j)
				{
					int gammaLen = pTables->aProds[p].count - 2;
					Symbol *gamma = NULL;
					if (gammaLen > 0)
					{
						gamma = (Symbol *)malloc(gammaLen * sizeof(Symbol));
						memcpy(gamma, &pTables->aProds[p].aSymbols[2], gammaLen * sizeof(Symbol));
					}
					RemoveProductionAt(pTables, p);

					int currentProds = pTables->countProds;
					for (int q = 0; q < currentProds; q++)
					{
						if (pTables->aProds[q].aSymbols[0].id == j)
						{
							int deltaLen = pTables->aProds[q].count - 1;
							Symbol *newRhs =
								(Symbol *)malloc((deltaLen + gammaLen + 1) * sizeof(Symbol));
							if (deltaLen > 0)
								memcpy(newRhs, &pTables->aProds[q].aSymbols[1],
									   deltaLen * sizeof(Symbol));
							if (gammaLen > 0)
								memcpy(newRhs + deltaLen, gamma, gammaLen * sizeof(Symbol));
							AppendProduction(pTables, i, newRhs, deltaLen + gammaLen);
							free(newRhs);
						}
					}
					free(gamma);
					anyChanged = 1;
				}
				else
				{
					p++;
				}
			}
		}

		int hasDirectLR = 0;
		for (int p = 0; p < pTables->countProds; p++)
		{
			if (pTables->aProds[p].aSymbols[0].id == i && pTables->aProds[p].count > 1 &&
				pTables->aProds[p].aSymbols[1].Type == T_NONTERMINAL &&
				pTables->aProds[p].aSymbols[1].id == i)
			{
				hasDirectLR = 1;
				break;
			}
		}

		if (hasDirectLR)
		{
			anyChanged = 1;
			int newNT = AddNewNonTerminal(pTables, pTables->aNonTerminals[i].strLexeme);

			int *isAlpha = (int *)calloc(pTables->countProds, sizeof(int));
			int *isBeta = (int *)calloc(pTables->countProds, sizeof(int));
			int oldCount = pTables->countProds;

			for (int p = 0; p < oldCount; p++)
			{
				if (pTables->aProds[p].aSymbols[0].id == i)
				{
					if (pTables->aProds[p].count > 1 &&
						pTables->aProds[p].aSymbols[1].Type == T_NONTERMINAL &&
						pTables->aProds[p].aSymbols[1].id == i)
					{
						isAlpha[p] = 1;
					}
					else
					{
						isBeta[p] = 1;
					}
				}
			}

			for (int p = 0; p < oldCount; p++)
			{
				if (isBeta[p])
				{
					int betaLen = pTables->aProds[p].count - 1;
					Symbol *rhs = (Symbol *)malloc((betaLen + 1) * sizeof(Symbol));
					if (betaLen > 0)
						memcpy(rhs, &pTables->aProds[p].aSymbols[1], betaLen * sizeof(Symbol));
					rhs[betaLen] = pTables->aNonTerminals[newNT];
					AppendProduction(pTables, i, rhs, betaLen + 1);
					free(rhs);
				}
			}

			for (int p = 0; p < oldCount; p++)
			{
				if (isAlpha[p])
				{
					int alphaLen = pTables->aProds[p].count - 2;
					Symbol *rhs = (Symbol *)malloc((alphaLen + 1) * sizeof(Symbol));
					if (alphaLen > 0)
						memcpy(rhs, &pTables->aProds[p].aSymbols[2], alphaLen * sizeof(Symbol));
					rhs[alphaLen] = pTables->aNonTerminals[newNT];
					AppendProduction(pTables, newNT, rhs, alphaLen + 1);
					free(rhs);
				}
			}
			AppendProduction(pTables, newNT, NULL, 0);

			for (int p = oldCount - 1; p >= 0; p--)
			{
				if (isAlpha[p] || isBeta[p])
				{
					RemoveProductionAt(pTables, p);
				}
			}
			free(isAlpha);
			free(isBeta);
		}
	}
	return anyChanged;
}

static int SymbolsEqual(Symbol a, Symbol b) { return (a.Type == b.Type && a.id == b.id); }

static int PerformLeftFactoring(Tables *pTables)
{
	int anyChanged = 0;
	int progress = 1;

	while (progress)
	{
		progress = 0;
		for (int nt = 0; nt < pTables->countNonTerminals; nt++)
		{
			int bestLen = 0;
			Symbol bestPrefix[64];

			for (int p1 = 0; p1 < pTables->countProds; p1++)
			{
				if (pTables->aProds[p1].aSymbols[0].id != nt || pTables->aProds[p1].count <= 1)
					continue;
				for (int p2 = p1 + 1; p2 < pTables->countProds; p2++)
				{
					if (pTables->aProds[p2].aSymbols[0].id != nt || pTables->aProds[p2].count <= 1)
						continue;

					int common = 0;
					while (1 + common < pTables->aProds[p1].count &&
						   1 + common < pTables->aProds[p2].count &&
						   SymbolsEqual(pTables->aProds[p1].aSymbols[1 + common],
										pTables->aProds[p2].aSymbols[1 + common]))
					{
						common++;
					}
					if (common > bestLen)
					{
						bestLen = common;
						for (int k = 0; k < common && k < 64; k++)
						{
							bestPrefix[k] = pTables->aProds[p1].aSymbols[1 + k];
						}
					}
				}
			}

			if (bestLen > 0)
			{
				anyChanged = 1;
				progress = 1;
				int newNT = AddNewNonTerminal(pTables, pTables->aNonTerminals[nt].strLexeme);
				int oldCount = pTables->countProds;
				int *matched = (int *)calloc(oldCount, sizeof(int));

				for (int p = 0; p < oldCount; p++)
				{
					if (pTables->aProds[p].aSymbols[0].id == nt &&
						pTables->aProds[p].count > bestLen)
					{
						int ok = 1;
						for (int k = 0; k < bestLen; k++)
						{
							if (!SymbolsEqual(pTables->aProds[p].aSymbols[1 + k], bestPrefix[k]))
							{
								ok = 0;
								break;
							}
						}
						if (ok)
						{
							matched[p] = 1;
							int remLen = pTables->aProds[p].count - 1 - bestLen;
							AppendProduction(pTables, newNT,
											 remLen > 0 ? &pTables->aProds[p].aSymbols[1 + bestLen]
														: NULL,
											 remLen);
						}
					}
				}

				Symbol *newRhs = (Symbol *)malloc((bestLen + 1) * sizeof(Symbol));
				memcpy(newRhs, bestPrefix, bestLen * sizeof(Symbol));
				newRhs[bestLen] = pTables->aNonTerminals[newNT];
				AppendProduction(pTables, nt, newRhs, bestLen + 1);
				free(newRhs);

				for (int p = oldCount - 1; p >= 0; p--)
				{
					if (matched[p])
						RemoveProductionAt(pTables, p);
				}
				free(matched);
				break;
			}
		}
	}
	return anyChanged;
}

static void ResetLLAnalysisData(Tables *pTables)
{
	for (int i = 0; i < pTables->countNonTerminals; i++)
	{
		FreeTerminalList(pTables->aNonTerminals[i].pFirstSet);

		FreeTerminalList(pTables->aNonTerminals[i].pFollowSet);
		pTables->aNonTerminals[i].pFirstSet = NULL;
		pTables->aNonTerminals[i].pFollowSet = NULL;
		pTables->aNonTerminals[i].bNullable = 0;
	}
	if (pTables->tableLL)
	{
		for (int i = 0; i < pTables->countNonTerminals; i++)
		{
			for (int j = 0; j < pTables->countTerminals + 1; j++)
			{
				FreeProdsList(pTables->tableLL[i][j]);
			}
			free(pTables->tableLL[i]);
		}
		free(pTables->tableLL);
		pTables->tableLL = NULL;
	}
	FreeProblemList(pTables->problemList);
	pTables->problemList = NULL;
}

int ConvertLALRtoLL1(Tables *pTables, ConversionReport *pRep)
{
	pRep->converted = 1;
	strncpy(pRep->sourceType, "LALR(1)", sizeof(pRep->sourceType) - 1);
	strncpy(pRep->targetType, "LL(1)",   sizeof(pRep->targetType) - 1);

	AddConversionStep(pRep, "1. Original LALR(1) grammar",
					  "Initial grammar before top-down transformations.", pTables);
	ResetLLAnalysisData(pTables);
	if (EliminateLeftRecursion(pTables))
	{
		AddConversionStep(pRep, "2. Elimination of Left Recursion",
						  "Left recursion (immediate and indirect) removed by introducing "
						  "new auxiliary non-terminal symbols (_p).",
						  pTables);
	}
	else
	{
		AddConversionStep(pRep, "2. Left Recursion Check",
						  "No left recursion detected in the grammar.", NULL);
	}
	if (PerformLeftFactoring(pTables))
	{
		AddConversionStep(pRep, "3. Left Factoring",
						  "Extracted common prefixes between alternative productions to eliminate "
						  "FIRST/FIRST conflicts.",
						  pTables);
	}
	else
	{
		AddConversionStep(pRep, "3. Left Factoring Check",
						  "No common prefix to factorize.", NULL);
	}
	MakeNullableSet(pTables);
	MakeFirstSets(pTables);
	MakeFollowSets(pTables);
	MakeTableLL(pTables);
	if (pTables->problemList == NULL)
	{
		pRep->success = 1;
		AddConversionStep(pRep, "4. LL(1) Table verification completed",
						  "The transformed grammar is free of conflicts and is now a "
						  "valid LL(1) grammar.",
						  NULL);
		return 1;
	}
	else
	{
		pRep->success = 0;
		AddConversionStep(pRep, "4. LL(1) Table check failed",
						  "Conflicts remain in the LL(1) table (the underlying language is not "
						  "strictly LL(1)).",
						  NULL);
		return 0;
	}
}

static void WriteHtmlHeader(FILE *fp, const char *pageTitle)
{
    fprintf(
        fp,
        "\n<html lang='en'> \n<head> \n"
        "<meta charset='UTF-8'>\n"
        "<meta name='viewport' content='width=device-width, initial-scale=1.0'>\n"
        "<title>%s - Vincenzo Lo Cicero</title> \n"
        "<style>\n"
        "body { font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto,"
        "Helvetica, Arial, sans-serif; background-color: #f4f6f9; color: #1e293b; margin:"
        "0; padding: 24px; line-height: 1.5; }\n"
        ".container { max-width: 1280px; margin: 0 auto; }\n"
        "h1 { font-size: 1.6rem; color: #0f172a; margin-bottom: 24px; border-bottom: 2px"
        "solid #cbd5e1; padding-bottom: 10px; }\n"
        ".card { background: #ffffff; border-radius: 8px; box-shadow: 0 1px 3px"
        "rgba(0,0,0,0.1); padding: 20px 24px; margin-bottom: 24px; overflow-x: auto; }\n"
        ".card h2 { font-size: 1.15rem; color: #334155; margin-top: 0; margin-bottom:"
        "14px; border-left: 4px solid #2563eb; padding-left: 10px; }\n"
        ".step-box { background: #f8fafc; border: 1px solid #e2e8f0; border-left: 4px"
        "solid #0284c7; border-radius: 6px; padding: 14px 18px; margin-bottom: 14px; }\n"
        ".step-box h3 { margin: 0 0 6px 0; font-size: 1rem; color: #0369a1; }\n"
        ".step-box p { margin: 0 0 8px 0; font-size: 0.92rem; color: #475569; }\n"
        "pre.gram-snap { background: #0f172a; color: #e2e8f0; padding: 12px;"
        "border-radius: 6px; font-family: 'Consolas', 'Monaco', monospace; font-size:"
        "0.88rem; overflow-x: auto; margin: 0; }\n"
        "table.grid-table { border-collapse: collapse; width: 100%%; font-size: 0.88rem;"
        "}\n"
        "table.grid-table th, table.grid-table td { border: 1px solid #cbd5e1; padding:"
        "7px 10px; }\n"
        "table.grid-table th { background-color: #1e293b; color: #ffffff; font-weight:"
        "600; text-align: center; }\n"
        "table.grid-table tr:nth-child(even) { background-color: #f8fafc; }\n"
        ".mono { font-family: 'Consolas', 'Monaco', monospace; }\n"
        ".cell-shift { background-color: #eff6ff; color: #1d4ed8; font-weight: 600;"
        "text-align: center; }\n"
        ".cell-reduce { background-color: #f0fdf4; color: #15803d; font-weight: 600;"
        "text-align: center; }\n"
        ".cell-accept { background-color: #fef9c3; color: #854d0e; font-weight: bold;"
        "text-align: center; }\n"
        ".cell-goto { background-color: #f1f5f9; color: #0f172a; font-weight: 600;"
        "text-align: center; }\n"
        ".cell-conflict { background-color: #fef2f2; color: #b91c1c; font-weight: bold;"
        "text-align: center; }\n"
        ".cell-err { color: #cbd5e1; text-align: center; }\n"
        ".badge-la { background: #e0e7ff; color: #3730a3; padding: 2px 6px;"
        "border-radius: 4px; font-size: 0.82rem; }\n"
        "</style>\n</head>\n<body>\n"
        "<div class='container'>\n<h1>%s - Vincenzo Lo Cicero</h1>\n",
        pageTitle, pageTitle);
}

static void WriteConversionReportSection(FILE *fp, const ConversionReport *pRep)
{
    if (!pRep || !pRep->converted)
        return;
    fprintf(fp, "<div class='card'>\n<h2>Conversion Report: %s → %s</h2>\n",
            pRep->sourceType, pRep->targetType);
    for (const ConversionStep *s = pRep->steps; s != NULL; s = s->next)
    {
        fprintf(fp, "<div class='step-box'>\n<h3>%s</h3>\n<p>%s</p>\n",
                s->title, s->description);
        if (s->grammarSnapshot)
            fprintf(fp, "<pre class='gram-snap'>%s</pre> \n", s->grammarSnapshot);
        fprintf(fp, "</div>\n");
    }
    fprintf(fp, "</div>\n");
}

void MakeHtmlFileWithReport(Tables *pTables, ConversionReport *pRep, const char *szFileName)
{
    if (!pRep || !pRep->converted)
    {
        MakeHtmlFile(pTables, szFileName);
        return;
    }
    MakeHtmlFile(pTables, szFileName);
    FILE *fp = fopen(szFileName, "rt");
    if (!fp) return;
    fseek(fp, 0, SEEK_END);
    long sz = ftell(fp);
    rewind(fp);
    char *orig = (char *)malloc(sz + 1);          
    if (!orig) { fclose(fp); return; }
    size_t nread = fread(orig, 1, sz, fp);
    orig[nread] = '\0';
    fclose(fp);
    char *h1End = strstr(orig, "</h1>");          
    if (!h1End) { free(orig); return; }
    h1End += 5;
    fp = fopen(szFileName, "wt");
    if (!fp) { free(orig); return; }
    fwrite(orig, 1, h1End - orig, fp);
    fprintf(fp, "\n");
    WriteConversionReportSection(fp, pRep);
    fprintf(fp, "%s", h1End);
    fclose(fp);
    free(orig);
}

void MakeLalrHtmlFile(Tables *pTables, LalrTables *pLalr, ConversionReport *pRep,
                      const char *szFileName)
{
    FILE *fp = fopen(szFileName, "wt");
    if (!fp)
    {
        printf("Error opening file %s.\n", szFileName);
        return;
    }
    WriteHtmlHeader(fp, "LALR(1) Grammar Tools");
    WriteConversionReportSection(fp, pRep);

    fprintf(fp,
            "<div class='card'>\n<h2>Numbered Productions (%d)</h2>\n"
            "<table class='grid-table' style='width: auto;'>\n"
            "<tr><th>Production ID</th><th>Production</th></tr>\n",
            pTables->countProds);
    for (int i = 0; i < pTables->countProds; i++)
    {
        fprintf(fp, "<tr><td class='mono' align='center'>%d</td><td class='mono'>%s →",
                i, pTables->aProds[i].aSymbols[0].strLexeme);
        if (pTables->aProds[i].count == 1)
            fprintf(fp, "ε");
        else
            for (int j = 1; j < pTables->aProds[i].count; j++)
                fprintf(fp, "%s ", pTables->aProds[i].aSymbols[j].strLexeme);
        fprintf(fp, "</td></tr>\n");
    }
    fprintf(fp, "</table>\n</div>\n");

    fprintf(fp,
            "<div class='card'>\n<h2>Canonical LALR(1) Automaton (%d States)</h2>\n"
            "<table class='grid-table'>\n"
            "<tr><th>State</th><th>LALR(1) Item [Production, Lookahead]</th></tr>\n",
            pLalr->numStates);
    for (int st = 0; st < pLalr->numStates; st++)
    {
        fprintf(fp, "<tr><td class='mono' align='center'>I%d</td><td class='mono'>\n", st);
        for (int it = 0; it < pLalr->states[st].numItems; it++)
        {
            int pId = pLalr->states[st].items[it].prodId;
            int dot = pLalr->states[st].items[it].dotPos;
            if (pId == -1)
                fprintf(fp, "S' → %s%s%s", (dot == 1) ? "• " : "",
                        pTables->aProds[0].aSymbols[0].strLexeme,
                        (dot == 2) ? " •" : "");
            else
            {
                const ArrayProduction *p = &pTables->aProds[pId];
                fprintf(fp, "%s → ", p->aSymbols[0].strLexeme);
                for (int k = 1; k < p->count; k++)
                {
                    if (k == dot) fprintf(fp, "• ");
                    fprintf(fp, "%s ", p->aSymbols[k].strLexeme);
                }
                if (dot == p->count) fprintf(fp, "•");
            }
            fprintf(fp, " <span class='badge-la'>{ ");
            for (int t = 0; t <= pTables->countTerminals; t++)
                if (pLalr->states[st].items[it].lookaheads[t])
                    fprintf(fp, "%s ", (t == pTables->countTerminals)
                                             ? "$"
                                             : pTables->aTerminals[t].strLexeme);
            fprintf(fp, "}</span><br>\n");
        }
        fprintf(fp, "</td></tr>\n");
    }
    fprintf(fp, "</table>\n</div>\n");

    fprintf(fp,
            "<div class='card'>\n<h2>LALR(1) Parsing Table (ACTION / GOTO)</h2>\n"
            "<table class='grid-table'>\n"
            "<tr><th rowspan='2'>State</th>"
            "<th colspan='%d'>ACTION</th><th colspan='%d'>GOTO</th></tr>\n",
            pTables->countTerminals + 1, pTables->countNonTerminals);
    fprintf(fp, "<tr>");
    for (int t = 0; t < pTables->countTerminals; t++)
        fprintf(fp, "<th class='mono'>%s</th>", pTables->aTerminals[t].strLexeme);
    fprintf(fp, "<th class='mono'>$</th>");
    for (int nt = 0; nt < pTables->countNonTerminals; nt++)
        fprintf(fp, "<th class='mono'>%s</th>", pTables->aNonTerminals[nt].strLexeme);
    fprintf(fp, "</tr>\n");

    for (int st = 0; st < pLalr->numStates; st++)
    {
        fprintf(fp, "<tr><td class='mono' align='center'>%d</td>", st);
        for (int t = 0; t <= pTables->countTerminals; t++)
        {
            LalrAction *act = pLalr->actionTable[st][t];
            if (!act)
            {
                fprintf(fp, "<td class='cell-err'>·</td>");
            }
            else
            {
                const char *cls = (act->next != NULL)            ? "cell-conflict"
                                  : (act->type == ACT_SHIFT)     ? "cell-shift"
                                  : (act->type == ACT_REDUCE)    ? "cell-reduce"
                                                                 : "cell-accept";
                fprintf(fp, "<td class='mono %s'>", cls);
                for (LalrAction *cur = act; cur != NULL; cur = cur->next)
                {
                    if (cur->type == ACT_SHIFT)  fprintf(fp, "s%d", cur->value);
                    if (cur->type == ACT_REDUCE) fprintf(fp, "r%d", cur->value);
                    if (cur->type == ACT_ACCEPT) fprintf(fp, "ACC");
                    if (cur->next)               fprintf(fp, " / ");
                }
                fprintf(fp, "</td>");
            }
        }
        for (int nt = 0; nt < pTables->countNonTerminals; nt++)
        {
            int g = pLalr->gotoTable[st][nt];
            if (g < 0) fprintf(fp, "<td class='cell-err'>·</td>");
            else       fprintf(fp, "<td class='mono cell-goto'>%d</td>", g);
        }
        fprintf(fp, "</tr>\n");
    }
    fprintf(fp, "</table>\n</div>\n");
    fprintf(fp, "</div>\n</body>\n</html>\n");
    fclose(fp);
}

void MakeLalrCFile(Tables *pTables, LalrTables *pLalr, const char *szFileName)
{
	if (!pLalr->isLALR1) { printf("Impossibile generare il parser C: conflitti LALR(1).\n"); return; }
	FILE *fp = fopen(szFileName, "wt");
	if (!fp) { printf("Errore nell'apertura del file %s.\n", szFileName); return; }
	int numT = pTables->countTerminals, numNT = pTables->countNonTerminals;

	fprintf(fp, "/* Parser LALR(1) con azioni semantiche - vlcGrammarTools */\n#include <stdio.h>\n#include <stdlib.h>\n\n");
	if (pTables->strPrologue && *pTables->strPrologue)
		fprintf(fp, "/* --- Prologo --- */\n%s\n\n", pTables->strPrologue);

	fprintf(fp, "#ifndef YYSTYPE\n#define YYSTYPE double\n#endif\n#ifndef YYSCAN_T\ntypedef void *yyscan_t;\n#endif\nint yylex(YYSTYPE *yylval_param, yyscan_t yyscanner);\nvoid yyerror(yyscan_t yyscanner, const char *s);\n\n");
	fprintf(fp, "#define LALR_NUM_STATES %d\n#define LALR_NUM_TERMINALS %d\n#define LALR_NUM_NONTERM %d\n#define LALR_NUM_PRODUCTIONS %d\n#define LALR_TOKEN_DOLLAR %d\n\n",
		pLalr->numStates, numT + 1, numNT, pTables->countProds, numT);

	fprintf(fp, "enum LalrTokens {\n");
	for (int t = 0; t < numT; t++) {
		const char *lex = pTables->aTerminals[t].strLexeme;
		if ((*lex >= 'A' && *lex <= 'Z') || (*lex >= 'a' && *lex <= 'z') || *lex == '_') fprintf(fp, "\tTOK_%s = %d,\n", lex, t);
		else fprintf(fp, "\tTOK_SYM_%d = %d, /* %s */\n", t, t, lex);
	}
	fprintf(fp, "\tTOK_EOF = %d\n};\n\n", numT);

	fprintf(fp, "typedef enum { ACT_ERR = 0, ACT_SHIFT = 1, ACT_REDUCE = 2, ACT_ACCEPT = 3 } ActionType;\n");
	fprintf(fp, "typedef struct { ActionType type; int val; } LalrCell;\ntypedef struct { int lhs; int rhsLen; } LalrProd;\n\n");
	fprintf(fp, "static const LalrProd aProductions[LALR_NUM_PRODUCTIONS] = {\n");
	for (int i = 0; i < pTables->countProds; i++)
		fprintf(fp, "\t{ %d, %d }%s\n", pTables->aProds[i].aSymbols[0].id, pTables->aProds[i].count - 1, (i < pTables->countProds - 1) ? "," : "");
	fprintf(fp, "};\n\nstatic const LalrCell actionTable[LALR_NUM_STATES][LALR_NUM_TERMINALS] = {\n");
	for (int st = 0; st < pLalr->numStates; st++) {
		fprintf(fp, "\t{ ");
		for (int t = 0; t <= numT; t++) {
			LalrAction *a = pLalr->actionTable[st][t];
			if (!a) fprintf(fp, "{ACT_ERR, 0}"); else fprintf(fp, "{%d, %d}", (int)a->type, a->value);
			if (t < numT) fprintf(fp, ", ");
		}
		fprintf(fp, " }%s\n", (st < pLalr->numStates - 1) ? "," : "");
	}
	fprintf(fp, "};\n\nstatic const int gotoTable[LALR_NUM_STATES][LALR_NUM_NONTERM] = {\n");
	for (int st = 0; st < pLalr->numStates; st++) {
		fprintf(fp, "\t{ ");
		for (int nt = 0; nt < numNT; nt++) { fprintf(fp, "%3d", pLalr->gotoTable[st][nt]); if (nt < numNT - 1) fprintf(fp, ", "); }
		fprintf(fp, " }%s\n", (st < pLalr->numStates - 1) ? "," : "");
	}
	fprintf(fp, "};\n\nint ParseLALR1(yyscan_t yyscanner, YYSTYPE *pResult)\n{\n\tint stateStack[1024], top = 0;\n\tYYSTYPE valStack[1024];\n\tYYSTYPE yylval = (YYSTYPE)0;\n\tstateStack[0] = 0;\n\tint lookahead = yylex(&yylval, yyscanner);\n");
	fprintf(fp, "\twhile (1) {\n\t\tint st = stateStack[top];\n\t\tLalrCell cell = (lookahead >= 0 && lookahead < LALR_NUM_TERMINALS) ? actionTable[st][lookahead] : (LalrCell){ACT_ERR, 0};\n");
	fprintf(fp, "\t\tif (cell.type == ACT_SHIFT) { top++; stateStack[top] = cell.val; valStack[top] = yylval; lookahead = yylex(&yylval, yyscanner); }\n");
	fprintf(fp, "\t\telse if (cell.type == ACT_REDUCE) {\n\t\t\tLalrProd p = aProductions[cell.val];\n\t\t\tYYSTYPE yyval = (p.rhsLen > 0) ? valStack[top - p.rhsLen + 1] : (YYSTYPE)0;\n\t\t\tswitch (cell.val) {\n");
	for (int i = 0; i < pTables->countProds; i++) {
		if (pTables->aProds[i].strAction && *pTables->aProds[i].strAction) {
			int rlen = pTables->aProds[i].count - 1;
			char fmt[64]; snprintf(fmt, sizeof(fmt), "valStack[top - %d + %%d]", rlen);
			char *tr = TranslateSemanticAction(pTables->aProds[i].strAction, rlen, "yyval", fmt);
			fprintf(fp, "\t\t\t\tcase %d: {\n%s\n\t\t\t\t\tbreak;\n\t\t\t\t}\n", i, tr ? tr : "");
			free(tr);
		}
	}
	fprintf(fp, "\t\t\t\tdefault: break;\n\t\t\t}\n\t\t\ttop -= p.rhsLen;\n\t\t\tint prevSt = stateStack[top];\n\t\t\ttop++; stateStack[top] = gotoTable[prevSt][p.lhs]; valStack[top] = yyval;\n\t\t}\n");
	fprintf(fp, "\t\telse if (cell.type == ACT_ACCEPT) { if (pResult && top >= 1) *pResult = valStack[top]; return 1; }\n");
	fprintf(fp, "\t\telse { yyerror(yyscanner, \"Syntax error\"); return 0; }\n\t}\n}\n");
	if (pTables->strEpilogue && *pTables->strEpilogue) fprintf(fp, "\n/* --- Epilogo --- */\n%s\n", pTables->strEpilogue);
	fclose(fp);
}
