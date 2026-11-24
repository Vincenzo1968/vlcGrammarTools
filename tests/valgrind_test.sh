#!/usr/bin/env bash
set -u

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"

# Il binario è installato globalmente
BIN="/usr/local/bin/vlcGrammarTools"
VG="valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --error-exitcode=42"

if ! command -v valgrind >/dev/null 2>&1; then
    echo "Error: valgrind is not installed."
    exit 1
fi

if [ ! -x "$BIN" ]; then
    echo "Error: $BIN not found. Run 'sudo make install' first."
    exit 1
fi

cd "$ROOT_DIR" || exit 1

PASS=0
FAIL=0

run_vg() {
    local desc="$1"
    shift
    echo "------------------------------------------------------------"
    echo "[TEST] $desc"
    $VG "$BIN" "$@" > /dev/null 2> /tmp/vg_out.txt
    local rc=$?
    if [ $rc -eq 42 ]; then
        echo "  --> [FAIL] Valgrind detected memory errors or leaks!"
        grep -E "ERROR SUMMARY|definitely lost|indirectly lost|Invalid read|Invalid write|uninitialised" /tmp/vg_out.txt
        FAIL=$((FAIL + 1))
    else
        echo "  --> [OK] Clean (exit code: $rc)"
        PASS=$((PASS + 1))
    fi
}

# 1. CLI Options & Error Handling
run_vg "Option --help" --help
run_vg "Option --version" --version
run_vg "Option --info" --info
run_vg "Non-existent input file" grammar_src/non_existent_file.grm

# 2. Native LL(1) & LL(1) -> LALR(1) Conversion
run_vg "Native LL(1) analysis (grammar_001.grm)" -o output/vg_test_ll1 grammar_src/grammar_001.grm
run_vg "Convert LL(1) -> LALR(1) (grammar_001.grm)" -c -o output/vg_test_to_lalr grammar_src/grammar_001.grm

# 3. Native LALR(1) & LALR(1) -> LL(1) Conversion
run_vg "Native LALR(1) analysis (grammar_022.grm)" -o output/vg_test_lalr grammar_src/grammar_022.grm
run_vg "Convert LALR(1) -> LL(1) (grammar_022.grm)" -c -o output/vg_test_to_ll1 grammar_src/grammar_022.grm
run_vg "Convert LALR(1) -> LL(1) (grammar_024.grm)" -c -o output/vg_test_024 grammar_src/grammar_024.grm

# 4. Syntax Errors & Unsupported Grammars
run_vg "Lexical/Syntax error handling (grammar_003.grm)" grammar_src/grammar_003.grm
run_vg "Syntax error handling (grammar_011.grm)" grammar_src/grammar_011.grm
run_vg "Unsupported grammar handling (grammar_023.grm)" grammar_src/grammar_023.grm

# 5. Semantic Actions (with and without --make_c_parser)
run_vg "Semantic actions ignored without --make_c_parser (calc_actions.grm)" -o output/vg_test_no_c grammar_src/calc_actions.grm
run_vg "Semantic actions C parser generation (calc_actions.grm)" --make_c_parser -o output/vg_test_with_c grammar_src/calc_actions.grm

# Cleanup temporary output artifacts
rm -f output/vg_test_* /tmp/vg_out.txt

echo "============================================================"
echo "VALGRIND SUMMARY: $PASS passed, $FAIL failed."
echo "============================================================"

[ $FAIL -eq 0 ]
