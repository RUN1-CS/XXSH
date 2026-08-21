#ifndef EXECUTOR_H
#define EXECUTOR_H

void license_notice();

char *get_prompt();

bool specials(const char *command, const char *arg, bool *running);

int execute_command(char *command, bool *running);

void handle_sig(int sig);

void setup_signals();

#endif // EXECUTOR_H
