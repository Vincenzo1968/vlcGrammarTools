#!/bin/bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"

if [ -x "$ROOT_DIR/src/vlcGrammarTools" ]; then
    VLC_BIN="$ROOT_DIR/src/vlcGrammarTools"
elif command -v vlcGrammarTools >/dev/null 2>&1; then
    VLC_BIN="$(command -v vlcGrammarTools)"
else
    echo "ERRORE: vlcGrammarTools non trovato (compila con ./configure && make)."
    exit 1
fi

TMP_DIR="$(mktemp -d /tmp/vlc_edge_cases.XXXXXX)"
trap 'rm -rf "$TMP_DIR"' EXIT

PASS=0
FAIL=0

run_vg() {
    valgrind --leak-check=full --show-leak-kinds=all \
             --errors-for-leak-kinds=definite,indirect,possible \
             --error-exitcode=99 --log-file="$TMP_DIR/vg.log" "$@"
}

pass() {
    echo "  --> [OK] $1"
    PASS=$((PASS + 1))
}

fail() {
    echo "  --> [FAIL] $1"
    if [ -f "$TMP_DIR/vg.log" ]; then
        cat "$TMP_DIR/vg.log"
    fi
    FAIL=$((FAIL + 1))
}

echo "Binario testato: $VLC_BIN"
echo "Workdir temporanea: $TMP_DIR"
cd "$TMP_DIR"

# ============================================================================
# 1. Azione semantica sull'ultima alternativa prima di ';' (LALR(1) e LL(1))
# ============================================================================
echo "[TEST 1] Azione semantica sull'ultima alternativa prima di ';' (LALR + LL)"
cat << 'EOF' > last_alt_lalr.grm
%{
#include <ctype.h>
#define YYSTYPE double
typedef struct { const char *input; size_t pos; } LexerState;
%}
expr : fact             { $$ = $1; }
     | '-' fact         { $$ = -$2; }
     | expr '+' fact    { $$ = $1 + $3; }
     ;
fact : NUM              { $$ = $1; }
     | '(' expr ')'     { $$ = $2; }
     ;
%%
int yylex(YYSTYPE *yylval_param, yyscan_t yyscanner) {
    LexerState *st = (LexerState *)yyscanner;
    while (isspace((unsigned char)st->input[st->pos])) st->pos++;
    char c = st->input[st->pos];
    if (c == '\0') return TOK_EOF;
    if (isdigit((unsigned char)c)) {
        char *ep = NULL;
        *yylval_param = strtod(st->input + st->pos, &ep);
        st->pos = (size_t)(ep - st->input);
        return TOK_NUM;
    }
    st->pos++;
    if (c == '-') return TOK_SYM_0;
    if (c == '+') return TOK_SYM_1;
    if (c == '(') return TOK_SYM_3;
    if (c == ')') return TOK_SYM_4;
    return -1;
}
void yyerror(yyscan_t yyscanner, const char *s) { (void)yyscanner; (void)s; }
int main(void) {
    LexerState st = { "-(3 + 4)", 0 };
    YYSTYPE res = 0.0;
    if (!ParseLALR1(&st, &res)) return 1;
    return (res == -7.0) ? 0 : 2;
}
EOF

if run_vg "$VLC_BIN" --make_c_parser -o last_alt_lalr last_alt_lalr.grm >/dev/null && \
   gcc -std=c11 -Wall -Wextra -pedantic -O2 output/last_alt_lalr.c -o last_alt_lalr_bin && \
   run_vg ./last_alt_lalr_bin; then
    pass "LALR(1): ultima alternativa prima di ';'"
else
    fail "LALR(1): ultima alternativa prima di ';'"
fi

cat << 'EOF' > last_alt_ll1.grm
%{
#include <ctype.h>
#define YYSTYPE double
typedef struct { const char *input; size_t pos; } LexerState;
%}
s : '-' fact { $$ = -$2; } ;
fact : NUM          { $$ = $1; }
     | '(' s ')'    { $$ = $2; }
     ;
%%
int yylex(YYSTYPE *yylval_param, yyscan_t yyscanner) {
    LexerState *st = (LexerState *)yyscanner;
    while (isspace((unsigned char)st->input[st->pos])) st->pos++;
    char c = st->input[st->pos];
    if (c == '\0') return TOK_EOF;
    if (isdigit((unsigned char)c)) {
        char *ep = NULL;
        *yylval_param = strtod(st->input + st->pos, &ep);
        st->pos = (size_t)(ep - st->input);
        return TOK_NUM;
    }
    st->pos++;
    if (c == '-') return TOK_SYM_0;
    if (c == '(') return TOK_SYM_2;
    if (c == ')') return TOK_SYM_3;
    return -1;
}
void yyerror(yyscan_t yyscanner, const char *s) { (void)yyscanner; (void)s; }
int main(void) {
    LexerState st = { "-(-5)", 0 };
    YYSTYPE res = 0.0;
    if (!ParseLL1(&st, &res)) return 1;
    return (res == 5.0) ? 0 : 2;
}
EOF

if run_vg "$VLC_BIN" --make_c_parser -o last_alt_ll1 last_alt_ll1.grm >/dev/null && \
   gcc -std=c11 -Wall -Wextra -pedantic -O2 output/last_alt_ll1.c -o last_alt_ll1_bin && \
   run_vg ./last_alt_ll1_bin; then
    pass "LL(1): ultima alternativa prima di ';'"
else
    fail "LL(1): ultima alternativa prima di ';'"
fi

# ============================================================================
# 2. ParseLL1 valStack overflow su produzioni epsilon (vtop >= cap mentre stop < cap)
# ============================================================================
echo "[TEST 2] ParseLL1: crescita valStack su produzioni epsilon (130x 'x a b')"
cat << 'EOF' > ll1_eps_overflow.grm
%{
#define YYSTYPE double
typedef struct { const char *input; size_t pos; } LexerState;
%}
s : r ;
r : 'x' a b r { $$ = $1 + $2 + $3 + $4; }
  |           { $$ = 0.0; }
  ;
a : { $$ = 1.0; } ;
b : { $$ = 2.0; } ;
%%
int yylex(YYSTYPE *yylval_param, yyscan_t yyscanner) {
    LexerState *st = (LexerState *)yyscanner;
    char c = st->input[st->pos];
    if (c == '\0') return TOK_EOF;
    st->pos++;
    if (c == 'x') { *yylval_param = 10.0; return TOK_x; }
    return -1;
}
void yyerror(yyscan_t yyscanner, const char *s) { (void)yyscanner; (void)s; }
int main(void) {
    char buf[131];
    for (int i = 0; i < 130; i++) buf[i] = 'x';
    buf[130] = '\0';
    LexerState st = { buf, 0 };
    YYSTYPE res = 0.0;
    if (!ParseLL1(&st, &res)) return 1;
    /* 130 * (10 + 1 + 2) = 1690 */
    return (res == 1690.0) ? 0 : 2;
}
EOF

if run_vg "$VLC_BIN" --make_c_parser -o ll1_eps ll1_eps_overflow.grm >/dev/null && \
   gcc -std=c11 -Wall -Wextra -pedantic -O2 output/ll1_eps.c -o ll1_eps_bin && \
   run_vg ./ll1_eps_bin; then
    pass "ParseLL1: valStack realloc su epsilon (valgrind clean, risultato esatto)"
else
    fail "ParseLL1: valStack realloc su epsilon"
fi

# ============================================================================
# 3. Conversione LALR(1) -> LL(1) (-c --make_c_parser): startSymbolId e reset azioni
# ============================================================================
echo "[TEST 3] Conversione -c su calc_actions.grm -> ParseLL1 parte da NT_expr"
if run_vg "$VLC_BIN" -c --make_c_parser -o calc_to_ll1 "$ROOT_DIR/grammar_src/calc_actions.grm" >/dev/null; then
    START_NT=$(awk -F'= ' '/NT_expr =/{gsub(/,/,"",$2); print $2}' output/calc_to_ll1.c)
    if grep -q "LL_SYM_NT_BASE + ${START_NT};" output/calc_to_ll1.c && \
       ! grep -q "case " output/calc_to_ll1.c; then
        sed -E 's/^int main[[:space:]]*\(/int original_main(/; s/ParseLALR1/ParseLL1/g' output/calc_to_ll1.c > calc_to_ll1_lib.c
        cat << 'EOF' > test_conv.c
#include <stddef.h>
typedef struct { const char *input; size_t pos; } LexerState;
extern int ParseLL1(void *yyscanner, double *pResult);
int main(void) {
    LexerState st1 = { "5 + 8 * 2 / (4 + 4)", 0 };
    LexerState st2 = { "5 + * 2", 0 };
    double r = 0.0;
    if (!ParseLL1(&st1, &r)) return 1;
    if (ParseLL1(&st2, &r)) return 2;
    return 0;
}
EOF
        if gcc -std=c11 -Wall -Wextra -pedantic -O2 calc_to_ll1_lib.c test_conv.c -lm -o test_conv_bin && \
           run_vg ./test_conv_bin; then
            pass "Conversione -c: NT_expr iniziale, azioni resettate, ParseLL1 valida 5+8*2/(4+4)"
        else
            fail "Conversione -c: compilazione o esecuzione ParseLL1 fallita"
        fi
    else
        fail "Conversione -c: startSymbolId errato o azioni residue presenti in calc_to_ll1.c"
    fi
else
    fail "Conversione -c su calc_actions.grm"
fi

# ============================================================================
# 4. Nullable / FIRST / FOLLOW con dipendenze in ordine inverso
# ============================================================================
echo "[TEST 4] Punto fisso Nullable / FIRST / FOLLOW"
cat << 'EOF' > fixed_point.grm
%{
#define YYSTYPE double
typedef struct { const char *input; size_t pos; } LexerState;
%}
s : a b ;
a : d 'c' | b ;
d : e ;
e : 'x' ;
b : ;
%%
int yylex(YYSTYPE *yylval_param, yyscan_t yyscanner) {
    (void)yylval_param;
    LexerState *st = (LexerState *)yyscanner;
    char c = st->input[st->pos];
    if (c == '\0') return TOK_EOF;
    st->pos++;
    if (c == 'c') return TOK_c;
    if (c == 'x') return TOK_x;
    return -1;
}
void yyerror(yyscan_t yyscanner, const char *s) { (void)yyscanner; (void)s; }
int main(void) {
    LexerState ok1 = { "xc", 0 };
    LexerState ok2 = { "", 0 };
    LexerState bad = { "c", 0 };
    double r = 0.0;
    if (!ParseLL1(&ok1, &r)) return 1;
    if (!ParseLL1(&ok2, &r)) return 2;
    if (ParseLL1(&bad, &r)) return 3;
    return 0;
}
EOF

if run_vg "$VLC_BIN" --make_c_parser -o fixed_point fixed_point.grm >/dev/null && \
   gcc -std=c11 -Wall -Wextra -pedantic -O2 output/fixed_point.c -o fixed_point_bin && \
   run_vg ./fixed_point_bin; then
    pass "Punto fisso Nullable / FIRST / FOLLOW (incluso input vuoto)"
else
    fail "Punto fisso Nullable / FIRST / FOLLOW"
fi

# ============================================================================
# 5. Token invalido a EOF senza newline (regressione loop infinito)
# ============================================================================
echo "[TEST 5] Token invalido a EOF senza newline ('=')"
printf '=' > invalid_eof.grm
set +e
timeout 5 valgrind --leak-check=full --show-leak-kinds=all \
        --errors-for-leak-kinds=definite,indirect,possible \
        --error-exitcode=99 --log-file="$TMP_DIR/vg.log" \
        "$VLC_BIN" invalid_eof.grm >/dev/null 2>&1
RC=$?
set -e
if [ "$RC" -ne 0 ] && [ "$RC" -ne 124 ] && [ "$RC" -ne 99 ]; then
    pass "Token invalido a EOF: exit=$RC, nessun loop infinito, valgrind clean"
else
    fail "Token invalido a EOF: exit=$RC (124=timeout, 99=valgrind)"
fi

# ============================================================================
# 6. Token speciali: "EOF", "*/", "a-b", "<" (collisioni enum, commenti C, HTML)
# ============================================================================
echo "[TEST 6] Token speciali ('EOF', '*/', 'a-b', '<')"
cat << 'EOF' > special_tokens.grm
%{
#define YYSTYPE int
typedef struct { const char *input; size_t pos; } LexerState;
%}
s : "EOF" "*/" "a-b" "<" ;
%%
int yylex(YYSTYPE *yylval_param, yyscan_t yyscanner) {
    (void)yylval_param;
    LexerState *st = (LexerState *)yyscanner;
    char c = st->input[st->pos];
    if (c == '\0') return TOK_EOF;
    st->pos++;
    if (c == '1') return TOK_SYM_0;
    if (c == '2') return TOK_SYM_1;
    if (c == '3') return TOK_SYM_2;
    if (c == '4') return TOK_SYM_3;
    return -1;
}
void yyerror(yyscan_t yyscanner, const char *s) { (void)yyscanner; (void)s; }
int main(void) {
    LexerState st = { "1234", 0 };
    return ParseLL1(&st, NULL) ? 0 : 1;
}
EOF

if run_vg "$VLC_BIN" --make_c_parser -o special_toks special_tokens.grm >/dev/null && \
   gcc -std=c11 -Wall -Wextra -pedantic -O2 output/special_toks.c -o special_toks_bin && \
   run_vg ./special_toks_bin && \
   grep -q "&lt;" output/special_toks.html; then
    pass "Token speciali ('EOF', '*/', 'a-b', '<'), pResult==NULL ed escaping HTML"
else
    fail "Token speciali o escaping HTML"
fi

# ============================================================================
# 7. Grammatica vuota / dangling else (conflitti) / NT irraggiungibile
# ============================================================================
echo "[TEST 7] Conflitto S/R (dangling else) e NT irraggiungibile"
cat << 'EOF' > dangling_else.grm
stmt : IF expr THEN stmt
     | IF expr THEN stmt ELSE stmt
     | OTHER
     ;
expr : ID ;
EOF

if run_vg "$VLC_BIN" dangling_else.grm >/dev/null; then
    pass "Dangling else gestito senza leak"
else
    fail "Dangling else"
fi

cat << 'EOF' > unreachable_nt.grm
s : b ;
unused : 'u' ;
b : c ;
c : 'x' ;
EOF

if run_vg "$VLC_BIN" --make_c_parser -o unreach unreachable_nt.grm >/dev/null; then
    pass "Ordina() con NT irraggiungibile e ordine sparso (valgrind clean)"
else
    fail "Ordina() con NT irraggiungibile"
fi

# ============================================================================
# 8. Input vuoto, token -1 e pResult==NULL su ParseLALR1
# ============================================================================
echo "[TEST 8] Input vuoto, token -1, pResult==NULL su ParseLALR1"
run_vg "$VLC_BIN" --make_c_parser -o calc_edge "$ROOT_DIR/grammar_src/calc_actions.grm" >/dev/null
sed -E 's/^int main[[:space:]]*\(/int original_main(/' output/calc_edge.c > calc_edge_lib.c
cat << 'EOF' > test_lalr_edge.c
#include <stddef.h>
typedef struct { const char *input; size_t pos; } LexerState;
extern int ParseLALR1(void *yyscanner, double *pResult);
int main(void) {
    LexerState empty_in = { "", 0 };
    LexerState bad_tok  = { "5 @ 2", 0 };
    LexerState ok_in    = { "5+2", 0 };
    if (ParseLALR1(&empty_in, NULL)) return 1;
    if (ParseLALR1(&bad_tok, NULL))  return 2;
    if (!ParseLALR1(&ok_in, NULL))   return 3;
    return 0;
}
EOF
if gcc -std=c11 -Wall -Wextra -pedantic -O2 calc_edge_lib.c test_lalr_edge.c -lm -o test_lalr_edge_bin && \
   run_vg ./test_lalr_edge_bin; then
    pass "ParseLALR1: input vuoto, token -1 e pResult==NULL"
else
    fail "ParseLALR1 edge cases"
fi

# ============================================================================
# 9. ThreadSanitizer + Rientranza su ParseLL1
# ============================================================================
echo "[TEST 9] ThreadSanitizer + rientranza su ParseLL1"
cat << 'EOF' > ll1_reentrant.grm
%{
#include <ctype.h>
#define YYSTYPE double
typedef struct { const char *input; size_t pos; } LexerState;
int ParseLL1(void *yyscanner, YYSTYPE *pResult);
%}
s : expr ;
expr : term exprp { $$ = $1 + $2; } ;
exprp : '+' term exprp { $$ = $2 + $3; }
      |                { $$ = 0.0; }
      ;
term : NUM { $$ = $1; }
     | '(' expr ')' { $$ = $2; }
     ;
%%
int yylex(YYSTYPE *yylval_param, yyscan_t yyscanner) {
    LexerState *st = (LexerState *)yyscanner;
    while (isspace((unsigned char)st->input[st->pos])) st->pos++;
    char c = st->input[st->pos];
    if (c == '\0') return TOK_EOF;
    if (isdigit((unsigned char)c)) {
        char *ep = NULL;
        *yylval_param = strtod(st->input + st->pos, &ep);
        st->pos = (size_t)(ep - st->input);
        return TOK_NUM;
    }
    st->pos++;
    if (c == '+') return TOK_SYM_0;
    if (c == '(') return TOK_SYM_2;
    if (c == ')') return TOK_SYM_3;
    return -1;
}
void yyerror(yyscan_t yyscanner, const char *s) { (void)yyscanner; (void)s; }
int main(void) { return 0; }
EOF

run_vg "$VLC_BIN" --make_c_parser -o ll1_ts ll1_reentrant.grm >/dev/null
sed -E 's/^int main[[:space:]]*\(/int original_main(/' output/ll1_ts.c > ll1_ts_lib.c
cat << 'EOF' > tsan_ll1_test.c
#include <stdio.h>
#include <pthread.h>

typedef struct { const char *input; size_t pos; } LexerState;
extern int ParseLL1(void *yyscanner, double *pResult);

typedef struct {
    const char *expr;
    double expected;
    int ok;
} Task;

static void *worker(void *arg) {
    Task *t = (Task *)arg;
    LexerState st = { t->expr, 0 };
    double res = 0.0;
    LexerState sub = { "1 + 2", 0 };
    double subres = 0.0;
    int ok_sub = ParseLL1(&sub, &subres) && (subres == 3.0);
    t->ok = ok_sub && ParseLL1(&st, &res) && (res == t->expected);
    return NULL;
}

int main(void) {
    Task t1 = { "(10 + 20) + 5 + 7", 42.0, 0 };
    Task t2 = { "1 + (2 + (3 + 4))", 10.0, 0 };
    pthread_t th1, th2;
    pthread_create(&th1, NULL, worker, &t1);
    pthread_create(&th2, NULL, worker, &t2);
    pthread_join(th1, NULL);
    pthread_join(th2, NULL);
    return (t1.ok && t2.ok) ? 0 : 1;
}
EOF

if gcc -std=c11 -Wall -Wextra -pedantic -fsanitize=thread -g ll1_ts_lib.c tsan_ll1_test.c -o tsan_ll1_bin && \
   ./tsan_ll1_bin; then
    pass "ParseLL1: ThreadSanitizer + rientranza OK"
else
    fail "ParseLL1: ThreadSanitizer o rientranza fallita"
fi

echo "============================================================"
echo "EDGE CASES SUMMARY: $PASS passed, $FAIL failed."
echo "============================================================"
[ "$FAIL" -eq 0 ]
