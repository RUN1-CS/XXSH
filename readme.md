# XXSH — A tiny shell implemented in C

XXSH is a small, educational command-line shell written in C. It demonstrates a minimal interactive shell loop with line editing and history (via GNU readline), simple built-in commands, piping between processes, and a script mode for running command scripts.

## Features

- Interactive prompt with history (readline)
- Built-in helpers: `exit`, `show` (license/help), and a few easter-egg commands
- Support for pipelines (e.g. `ls | grep foo`)
- Script mode: run a plain-text script file of commands
- Loads `~/.xxshrc` at startup if present

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

## Built-in commands and notes

- `exit` — quit the shell
- `show` — shows license and help
  - `show w` — warranty / disclaimer text
  - `show c` — conditions for redistribution
- Special short codes (fun messages): `45510`, `86`, `02`

The shell supports pipelines (using `|`) and executes external commands using `execvp`.

If a file named `~/.xxshrc` exists, XXSH will run its commands at startup.

## Contributing

This project is intended as a learning example. Contributions are welcome: open an issue or submit a pull request.

## License

This project is licensed under the GNU General Public License v3. See the `LICENSE` file for full terms.

---

Anyone using or studying this code should treat it as an educational reference for building minimal shells in C.
