# vlcGrammarTools

**vlcGrammarTools** is a command-line utility written in C for analyzing, converting, and generating parsers for **LL(1)** and **LALR(1)** context-free grammars.

## Features

- **Automatic Grammar Classification**: Parses a BNF-style grammar file (`.grm`) and automatically detects whether it belongs to the **LL(1)** or **LALR(1)** class (reporting syntax/lexical errors or unsupported grammars).
- **Bidirectional Conversion (`LL(1)` ↔ `LALR(1)`)**:
  - **LALR(1) → LL(1)**: Performs indirect and immediate left-recursion elimination followed by iterative left factoring, verifying the resulting LL(1) parsing table.
  - **LL(1) → LALR(1)**: Builds the canonical LR(0) item sets, propagates LALR(1) lookaheads, and constructs the `ACTION` / `GOTO` parsing tables.
- **Rich HTML Reports**: Generates standalone HTML documents with embedded CSS showing productions, `Nullable` / `FIRST` / `FOLLOW` sets, canonical LALR(1) states, parsing tables, and step-by-step conversion logs.
- **C Code Generation**: Outputs ready-to-use C parsing tables and driver routines for both LL(1) and LALR(1) grammars.
- **Safe Output Management**: Automatically saves generated `.html` and `.c` files into the `../output` directory, appending `(2)`, `(3)`, etc. instead of overwriting existing files.

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
| `-c` | `--convert` | Automatically convert `LL(1)` ↔ `LALR(1)` to the opposite format |
| | `--to-ll1` | Force conversion and code generation for an **LL(1)** parser |
| | `--to-lalr1` | Force conversion and code generation for an **LALR(1)** parser |

### Examples

1. **Analyze a grammar and generate HTML/C files using its native class (`LL(1)` or `LALR(1)`):**
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

## Running Memory Tests (Valgrind)

To verify that there are no memory leaks or invalid memory accesses across all parsing and conversion paths, run the included Valgrind test suite:

```bash
./tests/valgrind_test.sh
```

## License

This project is licensed under the GNU General Public License — see the [LICENSE](LICENSE) file for details.
