#ifndef EXECUTOR_C
#define EXECUTOR_C

#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif

#include "../../kernel/drivers/keyboard.h"
#include "../../kernel/drivers/power.h"
#include "../../kernel/drivers/string.h"
#include "../../kernel/drivers/video.h"

#ifndef MAX_COMMAND_LENGTH
#define MAX_COMMAND_LENGTH 1024
#endif

#ifndef bool
#define bool int
#define true 1
#define false 0
#endif

// License notice, GPL 3.0 compatible
void license_notice() {
  str_print("XXSH  Copyright (C) 2026  RUN1/RUN1-CS\n");
  str_print("This program comes with ABSOLUTELY NO WARRANTY; for details type "
            "`show w'.\n");
  str_print("This is free software, and you are welcome to redistribute it\n");
  str_print("under certain conditions; type `show c' for details.\n");
}

// Check for special built-in commands that don't require forking
bool commands(const char *command, const char *arg/*, bool *running */) {
  if (strcmp(command, "shutdown") == 0) {
    if (arg == NULL) {
      str_print("Shutting down...\n");
      shutdown();
    }
    return true;
  }

  // Show command for license and credits (GPL 3.0 compatible)
  if (strcmp(command, "show") == 0) {
    if (arg == NULL) {
      license_notice();
    } else if (strcmp(arg, "w") == 0) {
      str_print(
          "This program is distributed in the hope that it will be useful,\n");
      str_print(
          "but WITHOUT ANY WARRANTY; without even the implied warranty of\n");
      str_print(
          "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n");
      str_print("GNU General Public License for more details.\n");
    } else if (strcmp(arg, "c") == 0) {
      str_print("You can redistribute it and/or modify,\n");
      str_print("under these conditions:\n");
      str_print(
          "1. You must give credit to the original author (RUN1/RUN1-CS).\n");
      str_print("2. You must include this license notice in any copies or "
                "substantial portions of the software.\n");
    } else {
      str_print("Unknown argument for show.\n");
    }
    return true;
  } else if (strcmp(command, "clear") == 0) {
    // Clear the screen
    clear_screen();
    return true;
  } else if (strcmp(command, "echo") == 0) {
    // Echo the argument back to the screen
    if (arg != NULL) {
      str_print((char *)arg);
      str_print("\n");
    } else {
      str_print("No argument provided for echo.\n");
    }
    return true;
  } else {
    return false; // Not a built-in command
  }

  return false;
}

// Execute a command with optional piping
int execute_command(char *command/*, bool *running*/) {
  char *saveptr = NULL;
  char *segment = strtok_r(command, ";", &saveptr);

  while (segment != NULL) {
    // 1. Trim leading whitespace
    while (*segment == ' ' || *segment == '\t')
      segment++;

    // 2. Trim trailing whitespace
    char *end = segment + strlen(segment);
    while (end > segment &&
           (end[-1] == ' ' || end[-1] == '\t' || end[-1] == '\n')) {
      *--end = '\0';
    }

    if (*segment != '\0') {
      char *args[MAX_COMMAND_LENGTH / 2 + 1];
      int argc = 0;
      char *token = strtok_r(segment, " \t", &saveptr);

      // 3. Tokenize args
      while (token != NULL && argc < (MAX_COMMAND_LENGTH / 2)) {
        args[argc++] = token;
        token = strtok_r(NULL, " \t", &saveptr);
      }
      args[argc] = NULL;

      if (argc > 0) {
        // 4. Catch features that kernel doesn't have yet so it doesn't crash
        int has_pipe = 0;
        for (int i = 0; i < argc; i++) {
          if (strcmp(args[i], "|") == 0 || strcmp(args[i], ">") == 0 ||
              strcmp(args[i], ">>") == 0) {
            has_pipe = 1;
            break;
          }
        }

        if (has_pipe) {
          str_print(
              "Neon-Kernel: Pipes and redirection are not supported yet.\n");
        } else if (commands(args[0], args[1]/*, running*/)) {
          // Built-in command executed
        } else {
          str_print("Neon-Kernel: Command not found: ");
          str_print(args[0]);
          str_print("\n");
        }
      }
    }
    // Move to the next semicolon-separated command
    segment = strtok_r(NULL, ";", &saveptr);
  }
  return 0;
}

#endif // EXECUTOR_C
