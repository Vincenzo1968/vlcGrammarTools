/* Parser LALR(1) con azioni semantiche - vlcGrammarTools */
#include <stdio.h>
#include <stdlib.h>

/* --- Prologo --- */

#include <ctype.h>
#include <math.h>
#define YYSTYPE double

typedef struct {
    const char *input;
    size_t pos;
} LexerState;


#ifndef YYSTYPE
#define YYSTYPE double
#endif
#ifndef YYSCAN_T
typedef void *yyscan_t;
#endif
int yylex(YYSTYPE *yylval_param, yyscan_t yyscanner);
void yyerror(yyscan_t yyscanner, const char *s);

#define LALR_NUM_STATES 16
#define LALR_NUM_TERMINALS 8
#define LALR_NUM_NONTERM 3
#define LALR_NUM_PRODUCTIONS 8
#define LALR_TOKEN_DOLLAR 7

enum LalrTokens {
	TOK_SYM_0 = 0, /* + */
	TOK_SYM_1 = 1, /* - */
	TOK_SYM_2 = 2, /* * */
	TOK_SYM_3 = 3, /* / */
	TOK_SYM_4 = 4, /* ( */
	TOK_SYM_5 = 5, /* ) */
	TOK_NUM = 6,
	TOK_EOF = 7
};

typedef enum { ACT_ERR = 0, ACT_SHIFT = 1, ACT_REDUCE = 2, ACT_ACCEPT = 3 } ActionType;
typedef struct { ActionType type; int val; } LalrCell;
typedef struct { int lhs; int rhsLen; } LalrProd;

static const LalrProd aProductions[LALR_NUM_PRODUCTIONS] = {
	{ 0, 3 },
	{ 0, 3 },
	{ 0, 1 },
	{ 1, 3 },
	{ 1, 3 },
	{ 1, 1 },
	{ 2, 3 },
	{ 2, 1 }
};

static const LalrCell actionTable[LALR_NUM_STATES][LALR_NUM_TERMINALS] = {
	{ {ACT_ERR, 0}, {ACT_ERR, 0}, {ACT_ERR, 0}, {ACT_ERR, 0}, {1, 1}, {ACT_ERR, 0}, {1, 2}, {ACT_ERR, 0} },
	{ {ACT_ERR, 0}, {ACT_ERR, 0}, {ACT_ERR, 0}, {ACT_ERR, 0}, {1, 1}, {ACT_ERR, 0}, {1, 2}, {ACT_ERR, 0} },
	{ {2, 7}, {2, 7}, {2, 7}, {2, 7}, {ACT_ERR, 0}, {2, 7}, {ACT_ERR, 0}, {2, 7} },
	{ {1, 7}, {1, 8}, {ACT_ERR, 0}, {ACT_ERR, 0}, {ACT_ERR, 0}, {ACT_ERR, 0}, {ACT_ERR, 0}, {3, 0} },
	{ {2, 2}, {2, 2}, {1, 9}, {1, 10}, {ACT_ERR, 0}, {2, 2}, {ACT_ERR, 0}, {2, 2} },
	{ {2, 5}, {2, 5}, {2, 5}, {2, 5}, {ACT_ERR, 0}, {2, 5}, {ACT_ERR, 0}, {2, 5} },
	{ {1, 7}, {1, 8}, {ACT_ERR, 0}, {ACT_ERR, 0}, {ACT_ERR, 0}, {1, 11}, {ACT_ERR, 0}, {ACT_ERR, 0} },
	{ {ACT_ERR, 0}, {ACT_ERR, 0}, {ACT_ERR, 0}, {ACT_ERR, 0}, {1, 1}, {ACT_ERR, 0}, {1, 2}, {ACT_ERR, 0} },
	{ {ACT_ERR, 0}, {ACT_ERR, 0}, {ACT_ERR, 0}, {ACT_ERR, 0}, {1, 1}, {ACT_ERR, 0}, {1, 2}, {ACT_ERR, 0} },
	{ {ACT_ERR, 0}, {ACT_ERR, 0}, {ACT_ERR, 0}, {ACT_ERR, 0}, {1, 1}, {ACT_ERR, 0}, {1, 2}, {ACT_ERR, 0} },
	{ {ACT_ERR, 0}, {ACT_ERR, 0}, {ACT_ERR, 0}, {ACT_ERR, 0}, {1, 1}, {ACT_ERR, 0}, {1, 2}, {ACT_ERR, 0} },
	{ {2, 6}, {2, 6}, {2, 6}, {2, 6}, {ACT_ERR, 0}, {2, 6}, {ACT_ERR, 0}, {2, 6} },
	{ {2, 0}, {2, 0}, {1, 9}, {1, 10}, {ACT_ERR, 0}, {2, 0}, {ACT_ERR, 0}, {2, 0} },
	{ {2, 1}, {2, 1}, {1, 9}, {1, 10}, {ACT_ERR, 0}, {2, 1}, {ACT_ERR, 0}, {2, 1} },
	{ {2, 3}, {2, 3}, {2, 3}, {2, 3}, {ACT_ERR, 0}, {2, 3}, {ACT_ERR, 0}, {2, 3} },
	{ {2, 4}, {2, 4}, {2, 4}, {2, 4}, {ACT_ERR, 0}, {2, 4}, {ACT_ERR, 0}, {2, 4} }
};

static const int gotoTable[LALR_NUM_STATES][LALR_NUM_NONTERM] = {
	{   3,   4,   5 },
	{   6,   4,   5 },
	{  -1,  -1,  -1 },
	{  -1,  -1,  -1 },
	{  -1,  -1,  -1 },
	{  -1,  -1,  -1 },
	{  -1,  -1,  -1 },
	{  -1,  12,   5 },
	{  -1,  13,   5 },
	{  -1,  -1,  14 },
	{  -1,  -1,  15 },
	{  -1,  -1,  -1 },
	{  -1,  -1,  -1 },
	{  -1,  -1,  -1 },
	{  -1,  -1,  -1 },
	{  -1,  -1,  -1 }
};

int ParseLALR1(yyscan_t yyscanner, YYSTYPE *pResult)
{
	int stateStack[1024], top = 0;
	YYSTYPE valStack[1024];
	YYSTYPE yylval = (YYSTYPE)0;
	stateStack[0] = 0;
	int lookahead = yylex(&yylval, yyscanner);
	while (1) {
		int st = stateStack[top];
		LalrCell cell = (lookahead >= 0 && lookahead < LALR_NUM_TERMINALS) ? actionTable[st][lookahead] : (LalrCell){ACT_ERR, 0};
		if (cell.type == ACT_SHIFT) { top++; stateStack[top] = cell.val; valStack[top] = yylval; lookahead = yylex(&yylval, yyscanner); }
		else if (cell.type == ACT_REDUCE) {
			LalrProd p = aProductions[cell.val];
			YYSTYPE yyval = (p.rhsLen > 0) ? valStack[top - p.rhsLen + 1] : (YYSTYPE)0;
			switch (cell.val) {
				case 0: {
 yyval = valStack[top - 3 + 1] + valStack[top - 3 + 3]; 
					break;
				}
				case 1: {
 yyval = valStack[top - 3 + 1] - valStack[top - 3 + 3]; 
					break;
				}
				case 3: {
 yyval = valStack[top - 3 + 1] * valStack[top - 3 + 3]; 
					break;
				}
				case 4: {
 yyval = (valStack[top - 3 + 3] != 0.0) ? (valStack[top - 3 + 1] / valStack[top - 3 + 3]) : 0.0; 
					break;
				}
				case 6: {
 yyval = valStack[top - 3 + 2]; 
					break;
				}
				default: break;
			}
			top -= p.rhsLen;
			int prevSt = stateStack[top];
			top++; stateStack[top] = gotoTable[prevSt][p.lhs]; valStack[top] = yyval;
		}
		else if (cell.type == ACT_ACCEPT) { if (pResult && top >= 1) *pResult = valStack[top]; return 1; }
		else { yyerror(yyscanner, "Syntax error"); return 0; }
	}
}

/* --- Epilogo --- */


int yylex(YYSTYPE *yylval_param, yyscan_t yyscanner)
{
    LexerState *st = (LexerState *)yyscanner;
    while (st->input[st->pos] == ' ' || st->input[st->pos] == '\t') st->pos++;
    char c = st->input[st->pos];
    if (c == '\0') return TOK_EOF;
    if (isdigit((unsigned char)c)) {
        char *endp = NULL;
        *yylval_param = strtod(st->input + st->pos, &endp);
        st->pos = (size_t)(endp - st->input);
        return TOK_NUM;
    }
    st->pos++;
    if (c == '+') return TOK_SYM_0;
    if (c == '-') return TOK_SYM_1;
    if (c == '*') return TOK_SYM_2;
    if (c == '/') return TOK_SYM_3;
    if (c == '(') return TOK_SYM_4;
    if (c == ')') return TOK_SYM_5;
    return -1;
}

void yyerror(yyscan_t yyscanner, const char *s)
{
    (void)yyscanner;
    fprintf(stderr, "Error: %s\n", s);
}

int main(void)
{
    LexerState st = { "5 + 8 * 2 / (4 + 4)", 0 };
    YYSTYPE res = 0.0;
    if (ParseLALR1(&st, &res))
        printf("Input: %s => Result = %.2f\n", st.input, res);
    return 0;
}

