# XXSH — A tiny shell + mini interpreter in C

XXSH is a small, educational command-line shell written in C. It demonstrates a minimal interactive shell loop with line editing and history (via GNU readline), simple built-in commands, piping between processes, and a script mode for running command scripts. The project also includes a lightweight interpreter module used by script-based features.

## Features

- Interactive prompt with history (readline)
- Built-in helpers: `exit`, `show` (license/help), and a few easter-egg commands
- Support for pipelines (e.g. `ls | grep foo`)
- Script mode: run a plain-text script file of commands
- Loads `~/.xxshrc` at startup if present
- Includes an interpreter subsystem (`interpreter/`) with parsing, variables, and evaluation logic

## Requirements

- A POSIX-compatible system (Linux/macOS)
- `gcc` (or compatible C compiler)
- GNU Readline development headers (package name often `libreadline-dev` or similar)

On Debian/Ubuntu you can install dependencies with:

```bash
sudo apt-get update
sudo apt-get install build-essential libreadline-dev
```

## Build

From the project root run:

```bash
make
```

This produces the `xxsh` executable.

To clean build artifacts:

```bash
make clean
```

## Usage

- Interactive mode:

```bash
./xxsh
```

- Script mode (run commands from a file):

```bash
./xxsh test.xx
```

`test.xx` in this repository contains a tiny example script.

## Running included test scripts

Several sample scripts are provided under `tests/`:

- `tests/01_easter_eggs.xx`
- `tests/02_license.xx`
- `tests/03_if.xx`
- ... up to `tests/13_redirecting.xx`

Run any of them with:

```bash
./xxsh tests/03_if.xx
```

## Built-in commands and notes

- `exit` — quit the shell
- `show` — shows license and help
  - `show w` — warranty / disclaimer text
  - `show c` — conditions for redistribution
- Special short codes (fun messages): `45510`, `86`, `02`

The shell supports pipelines (using `|`) and executes external commands using `execvp`.

If a file named `~/.xxshrc` exists, XXSH will run its commands at startup.

## Project layout (high level)

- `main.c`, `executor.c`, `executor.h` — shell loop and command execution
- `interpreter/` — parser, evaluator, variables, and shared types
- `tests/` — script examples for features and edge cases
- `Makefile` — build rules
- `LICENSE` — GPLv3 text

## Contributing

This project is intended as a learning example. Contributions are welcome: open an issue or submit a pull request.

## License

This project is licensed under the GNU General Public License v3. See the `LICENSE` file for full terms.

---

Anyone using or studying this code should treat it as an educational reference for building minimal shells in C.
