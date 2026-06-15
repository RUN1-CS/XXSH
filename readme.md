# XXSH

XXSH is a stripped-down shell written in C. This repository focuses on the core shell flow and command execution logic, split into a small set of source files.

## Layout

```text
./apps/xxsh/
├── executor.c
├── executor.h
├── LICENSE
├── readme.md
├── shell.c
└── shell.h
```

## Files

- `shell.c` / `shell.h` — shell loop and command parsing
- `executor.c` / `executor.h` — process execution helpers
- `LICENSE` — license text
- `readme.md` — project documentation

## Build

Build the project from the repository root using the normal C build command or Makefile target provided by the repo.

## Usage

Run the compiled shell binary from a terminal:

```bash
./xxsh
```

## Notes

- This is a minimal shell example.
- Supported behavior is defined by `shell.c` and `executor.c`.

## License

See `LICENSE` for the full license terms.
