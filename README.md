# vlcGrammarTools

**vlcGrammarTools** is a command-line utility written in C for analyzing, converting, and generating parsers for **LL(1)** and **LALR(1)** context-free grammars.

## Features

- **Automatic Grammar Classification**: Parses a BNF-style grammar file (`.grm`) and automatically detects whether it belongs to the **LL(1)** or **LALR(1)** class (reporting syntax/lexical errors or unsupported grammars).
- **Bidirectional Conversion (`LL(1)` ↔ `LALR(1)`)**:
  - **LALR(1) → LL(1)**: Performs indirect and immediate left-recursion elimination followed by iterative left factoring, verifying the resulting LL(1) parsing table.
  - **LL(1) → LALR(1)**: Builds the canonical LR(0) item sets, propagates LALR(1) lookaheads, and constructs the `ACTION` / `GOTO` parsing tables.
- **Rich HTML Reports**: Generates standalone HTML documents with embedded CSS showing productions, `Nullable` / `FIRST` / `FOLLOW` sets, canonical LALR(1) states, parsing tables, and step-by-step conversion logs.
- **Reentrant & Thread-Safe C Code Generation**: Outputs pure, global-free C parsers (`ParseLL1(yyscan_t, YYSTYPE*)` / `ParseLALR1(yyscan_t, YYSTYPE*)`) and driver routines for both LL(1) and LALR(1) grammars.
- **Safe Output Management**: Automatically saves generated `.html` and `.c` files into the `../output` directory, appending `(2)`, `(3)`, etc. instead of overwriting existing files.

## Semantic Actions (Bison-Style)

When `--make_c_parser` is specified, `vlcGrammarTools` supports optional C semantic actions embedded in `.grm` files (if `--make_c_parser` is omitted, semantic actions are ignored and only the HTML report is generated):

- **Prologue (`%{ ... %}`)**: Optional C headers, macros (such as `#define YYSTYPE double`), and helper declarations at the top of the file.
- **Rule Actions (`{ ... }`)**: Optional C blocks at the end of any production alternative using `$$` for the left-hand side value and `$1`, `$2`, ..., `$n` for right-hand side symbols.
- **Epilogue (`%% ...`)**: Optional C code (`yylex()`, `yyerror()`, `main()`) placed after a trailing `%%` separator.

See [`grammar_src/calc_actions.grm`](grammar_src/calc_actions.grm) for a complete working calculator example.

## Building from Source (GNU Autotools)

```bash
autoreconf --install
./configure
make
```

## How to Use

Run the executable from the `src/` directory (or specify the path to your grammar file):

```bash
./vlcGrammarTools [OPTIONS] <GRAMMAR_FILE>
```

### Command-Line Options

| Option | Long Option | Description |
| :--- | :--- | :--- |
| `-h` | `--help` | Display usage information and exit |
| `-v` | `--version` | Display program version and exit |
| `-i` | `--info` | Display license information and exit |
| `-o <name>` | `--output <name>` | Set custom output base filename (without extension) |
| | `--make_c_parser` | Generate the C parser source file (including semantic actions) |
| `-c` | `--convert` | Automatically convert `LL(1)` ↔ `LALR(1)` to the opposite format |
| | `--to-ll1` | Force conversion and code generation for an **LL(1)** parser |
| | `--to-lalr1` | Force conversion and code generation for an **LALR(1)** parser |

### Examples

1. **Analyze a grammar and generate the HTML report using its native class (`LL(1)` or `LALR(1)`):**
   ```bash
   cd src
   ./vlcGrammarTools ../grammar_src/grammar_001.grm
   ```

2. **Convert an `LALR(1)` grammar into `LL(1)` and generate a step-by-step HTML report:**
   ```bash
   ./vlcGrammarTools -c -o converted_ll1 ../grammar_src/grammar_022.grm
   ```

3. **Convert an `LL(1)` grammar into `LALR(1)` (`ACTION` / `GOTO` tables):**
   ```bash
   ./vlcGrammarTools --to-lalr1 ../grammar_src/grammar_001.grm
   ```

4. **Generate and compile a standalone C parser with Bison-style semantic actions:**
   ```bash
   ./vlcGrammarTools --make_c_parser -o calc_demo ../grammar_src/calc_actions.grm
   gcc ../output/calc_demo.c -o ../output/calc_demo
   ../output/calc_demo
   ```

## Generated parsers: reentrancy, thread-safety and robustness

The C parsers produced with `--make_c_parser` (both LL(1) and LALR(1)) are:

- **Reentrant** — no global mutable state. All parsing state (`stateStack`,
  `valStack`, `symStack`, `vtop`, `top`, `stop`) is local to the parse function.
- **Thread-safe** — multiple threads can call `ParseLALR1` / `ParseLL1`
  concurrently, each with its own lexer instance (`yyscan_t`). The lexer must
  also be reentrant (as in the shipped examples: state kept in a `LexerState`
  struct passed through `yyscanner`).
- **Memory-safe** — the state stacks grow dynamically via `malloc`/`realloc`.
  There is no fixed limit on input length or nesting depth. All allocated
  memory is freed on both the success and error paths (verified with Valgrind).
- **Bison-compatible semantic actions** — `$$` and `$1..$k` placeholders are
  translated to direct stack accesses.

### Testing generated parsers on long or deep inputs

```bash
./tests/test_long_inputs.sh [path/to/grammar.grm]
```

The script generates inputs with up to 10,000 operators and 5,000 nested
parentheses, compiles the parser, runs the suite, and (if Valgrind is
installed) checks for memory leaks. Example:

```bash
./tests/test_long_inputs.sh                                  # default: calc_actions.grm
./tests/test_long_inputs.sh grammar_src/expr_ll1.grm         # LL(1) parser
```

## Running Memory Tests (Valgrind)

To verify that there are no memory leaks or invalid memory accesses across all parsing and conversion paths, run the included Valgrind test suite:

```bash
./tests/valgrind_test.sh
```

## License

This project is licensed under the GNU General Public License — see the [LICENSE](LICENSE) file for details.
