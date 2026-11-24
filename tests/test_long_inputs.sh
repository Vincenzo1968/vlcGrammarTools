#!/bin/bash
set -e

# test_long_inputs.sh
#
# Verifica che i parser generati da vlcGrammarTools (LL(1) e LALR(1))
# non vadano in crash o memory leak su input molto lunghi o profondi.
#
# Uso:
#   ./tests/test_long_inputs.sh                          # grammatica default
#   ./tests/test_long_inputs.sh grammar_src/expr_01.grm  # grammatica specifica
#
# Richiede: gcc, python3, valgrind (opzionale ma consigliato).
# Crea una directory temporanea test_long_tmp/ (gitignored).
#
# Exit code: 0 se tutti i test passano, 1 altrimenti.

# --- 1. Risoluzione percorso grammatica ---
GRAMMAR_ARG=${1:-grammar_src/calc_actions.grm}
if [[ "$GRAMMAR_ARG" != /* ]]; then
    GRAMMAR="$PWD/$GRAMMAR_ARG"
else
    GRAMMAR="$GRAMMAR_ARG"
fi

if [ ! -f "$GRAMMAR" ]; then
    echo "Errore: file grammatica non trovato: $GRAMMAR"
    exit 1
fi
echo "Grammatica: $GRAMMAR"

# --- 2. Preparazione workdir ---
WORKDIR="$PWD/test_long_tmp"
rm -rf "$WORKDIR"
mkdir -p "$WORKDIR"
cd "$WORKDIR"

# --- 3. Generazione parser C ---
echo "=== Generazione parser C ==="
vlcGrammarTools "$GRAMMAR" --make_c_parser

BASENAME=$(basename "$GRAMMAR" .grm)
GENERATED_C="$WORKDIR/output/${BASENAME}.c"
if [ ! -f "$GENERATED_C" ]; then
    echo "Errore: file C generato non trovato: $GENERATED_C"
    exit 1
fi
echo "File generato: $GENERATED_C"

# --- 4. Determina funzione di parsing ---
if grep -q "ParseLALR1" "$GENERATED_C"; then
    PARSE_FUNC="ParseLALR1"
elif grep -q "ParseLL1" "$GENERATED_C"; then
    PARSE_FUNC="ParseLL1"
else
    echo "Errore: nessuna funzione ParseLALR1/ParseLL1 trovata"
    exit 1
fi
echo "Funzione di parsing: $PARSE_FUNC"

# --- 5. Crea il driver che legge da file ---
cat > driver.c << MAIN
#include <stdio.h>
#include <stdlib.h>

typedef struct { const char *input; size_t pos; } LexerState;
extern int $PARSE_FUNC(void *yyscanner, double *pResult);

int main(int argc, char **argv) {
    if (argc < 2) { fprintf(stderr, "Uso: %s <file_input>\\n", argv[0]); return 1; }
    FILE *f = fopen(argv[1], "r");
    if (!f) { perror("fopen"); return 1; }
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    rewind(f);
    char *buf = malloc(sz + 1);
    if (!buf) { fclose(f); return 1; }
    fread(buf, 1, sz, f);
    buf[sz] = '\\0';
    fclose(f);

    /* Strip trailing whitespace (newline inclusa) */
    while (sz > 0 && (buf[sz-1] == '\\n' || buf[sz-1] == '\\r' ||
                      buf[sz-1] == ' '  || buf[sz-1] == '\\t')) {
        buf[--sz] = '\\0';
    }

    LexerState st = { buf, 0 };
    double result = 0.0;
    int ret = $PARSE_FUNC(&st, &result);
    if (ret) printf("Risultato = %.2f\\n", result);
    else     printf("Errore di parsing\\n");
    free(buf);
    return ret ? 0 : 1;
}
MAIN

# --- 6. Rinomina 'int main(' in 'int original_main(' nel file generato ---
echo "=== Rinomina main nel parser generato ==="
sed -E 's/^int main[[:space:]]*\(/int original_main(/' "$GENERATED_C" > parser_nomain.c

if grep -q '^int original_main' parser_nomain.c; then
    echo "OK: 'main' rinominato in 'original_main'"
else
    echo "ATTENZIONE: sed non ha trovato 'int main(' nel file generato!"
    grep -n 'int main' "$GENERATED_C" || echo "(nessuna occorrenza)"
    exit 1
fi

# --- 7. Compilazione in tre fasi ---
echo "=== Compilazione ==="
gcc -Wall -c -o parser_gen.o parser_nomain.c
gcc -Wall -c -o driver.o     driver.c
gcc -o test_parser parser_gen.o driver.o -lm

# --- 8. Generazione input di test ---
echo "=== Generazione input di test ==="
for N in 100 1000 5000 10000; do
    python3 -c "print('1' + '+1'*$N)" > "input_long_$N.txt"
done
for N in 100 1000 5000; do
    python3 -c "print('('*$N + '1' + ')'*$N)" > "input_deep_$N.txt"
done

# --- 9. Esecuzione test (senza valgrind) ---
echo "=== Esecuzione test ==="
FAIL=0
for f in input_*.txt; do
    echo -n "Test $f ... "
    if ./test_parser "$f" > /dev/null 2>&1; then
        echo "OK"
    else
        echo "FALLITO (exit $?)"
        FAIL=$((FAIL+1))
    fi
done

# --- 10. Valgrind su un input rappresentativo (medio) ---
if command -v valgrind >/dev/null; then
    echo "=== Valgrind su input_long_1000.txt ==="
    if valgrind --error-exitcode=1 --leak-check=full --errors-for-leak-kinds=definite \
        ./test_parser input_long_1000.txt > /dev/null 2>valgrind.log; then
        echo "Valgrind: OK (nessun leak definito)"
    else
        echo "Valgrind: FALLITO"
        tail -30 valgrind.log
        FAIL=$((FAIL+1))
    fi
else
    echo "Valgrind non installato, salto il controllo dei leak."
fi

# --- 11. Riepilogo ---
echo ""
if [ $FAIL -eq 0 ]; then
    echo "=== TUTTI I TEST SUPERATI ==="
    exit 0
else
    echo "=== $FAIL TEST FALLITI ==="
    exit 1
fi
