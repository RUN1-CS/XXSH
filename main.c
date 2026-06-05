/*
 *  XXSH - A simple shell written in C
 *  Copyright (C) 2026  RUN1/RUN1-CS
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.

 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef MAIN_C
#define MAIN_C

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <fcntl.h>

#include <sys/wait.h>
#include <sys/types.h>

#define bool int
#define true 1
#define false 0
#define baka -1

#define VAR_LIMIT 100 // Later will be dynamic, I am just too lazy to implement that right now

#include <readline/readline.h>
#include <readline/history.h>

#define MAX_COMMAND_LENGTH 1024

// License notice, GPL 3.0 compatible
void license_notice(){
    printf("XXSH  Copyright (C) 2026  RUN1/RUN1-CS\n");
    printf("This program comes with ABSOLUTELY NO WARRANTY; for details type `show w'.\n");
    printf("This is free software, and you are welcome to redistribute it\n");
    printf("under certain conditions; type `show c' for details.\n");
}

// Signal handler (e.g., for Ctrl+C)
void handle_sig(int sig){
    if(sig == SIGINT){
        printf("XX> ");
        fflush(stdout);
    }
}

// Setup signal handlers for the shell
void setup_signals() {
    struct sigaction sa;
    sa.sa_handler = handle_sig;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART; // Keeps system calls like read() from failing

    // Handle Ctrl+C (Interrupt)
    sigaction(SIGINT, &sa, NULL);
    
    // Handle 'kill' (Termination) - usually you want a clean exit here
    sa.sa_handler = SIG_DFL; // Or a custom cleanup function
    sigaction(SIGTERM, &sa, NULL);
}

enum VariableType {
    VAR_STRING,
    VAR_NUMBER,
    VAR_BOOL
};

union VariableValue {
    char string_value[256];
    double number_value;
    bool bool_value;
};

typedef struct Variable {
    char name[64];
    enum VariableType type;
    union VariableValue value;
} Variable;

void set_variable(const char *line, Variable *variables, int *var_count){
    FILE *DEBUG_LOG = fopen("debug.log", "a");
    if(DEBUG_LOG) {
        fprintf(DEBUG_LOG, "DEBUG: Setting variable with line: %s\n", line);
    }

    char name[64];
    char value[256];
    char type[16];

    // Syntax for setting variables: set <name> <type> = <value>
    if(sscanf(line, "set %63s %15s = %[^\n]", name, type, value) != 3){
        printf("\033[1;31mUsage: set <variable> <type> = <value>\033[0m\n");
        return;
    }
    if(strcmp(type, "string") == 0){
        strncpy(type, "string", sizeof(type) - 1);
    }else if(strcmp(type, "number") == 0){
        strncpy(type, "number", sizeof(type) - 1);
    }else if(strcmp(type, "bool") == 0){
        strncpy(type, "bool", sizeof(type) - 1);
    }else{
        printf("\033[1;31mUnsupported variable type: %s\033[0m\n", type);
        return;
    }

    // Check correct type of value
    if(strcmp(type, "number") == 0){
        char *endptr;
        strtod(value, &endptr);
        if(*endptr != '\0' && !isspace((unsigned char)*endptr)){
            printf("\033[1;31mInvalid number value: %s\033[0m\n", value);
            return;
        }
    } else if(strcmp(type, "bool") == 0){
        if(strcmp(value, "true") != 0 && strcmp(value, "false") != 0){
            printf("\033[1;31mInvalid boolean value: %s\033[0m\n", value);
            return;
        }
    }

    // Check if variable already exists
    for(int i = 0; i < *var_count; i++){
        if(strcmp(variables[i].name, name) == 0){
            // Update existing variable
            switch(variables[i].type){
                case VAR_STRING:
                    strncpy(variables[i].value.string_value, value, sizeof(variables[i].value.string_value));
                    break;
                case VAR_NUMBER:
                    variables[i].value.number_value = atof(value);
                    break;
                case VAR_BOOL:
                    variables[i].value.bool_value = (strcmp(value, "true") == 0);
                    break;
            }
            fprintf(DEBUG_LOG, "DEBUG: Updated variable '%s' to '%s' with type %d\n", name, value, variables[i].type);
            return;
        }
    }
    fprintf(DEBUG_LOG, "DEBUG: Adding new variable '%s' with value '%s' and type %s\n", name, value, type);
    // Add new variable
    if(*var_count < VAR_LIMIT){
        strncpy(variables[*var_count].name, name, sizeof(variables[*var_count].name));
        switch(type[0]){
            case 's':
                variables[*var_count].type = VAR_STRING;
                strncpy(variables[*var_count].value.string_value, value, sizeof(variables[*var_count].value.string_value));
                break;
            case 'n':
                variables[*var_count].type = VAR_NUMBER;
                variables[*var_count].value.number_value = atof(value);
                break;
            case 'b':
                variables[*var_count].type = VAR_BOOL;
                variables[*var_count].value.bool_value = (strcmp(value, "true") == 0);
                break;
        }
        fprintf(DEBUG_LOG, "DEBUG: Set variable '%s' to '%s' with type %d\n", name, value, variables[*var_count].type);
        (*var_count)++;
    } else {
        printf("\033[1;31mVariable limit reached (%d)\033[0m\n", VAR_LIMIT);
    }
    fclose(DEBUG_LOG);
}

// Check for special built-in commands that don't require forking
bool specials(const char * command, const char *arg, bool *running){
    if(strcmp(command, "exit") == 0) { *running = false; return true; } // Exit command

    // Oshi no Ko Easter Egg
    if(strcmp(command, "45510") == 0){
        printf("\033[1;35m[Even if it's a lie, I wanted it to be the truth.]\033[0m\n");
        return true;
    }

    // Eighty Six Easter Egg
    if(strcmp(command, "86") == 0){
        printf("\033[1;31m[We will fight, and we will move forward.]\033[0m\n");
        return true;
    }

    // Darling in the Franxx Easter Egg
    if(strcmp(command, "02") == 0){
        int rnd = rand() % 2;
        if(rnd == 0) printf("\033[1;95m[If you don't take a risk, you can't create a future.]\033[0m\n");
        else printf("\033[1;95m[If you don't belong here, just build a place where you do.]\033[0m\n");
        return true;
    }

    // Show command for license and credits (GPL 3.0 compatible)
    if(strcmp(command, "show") == 0){
        if(arg == NULL){
            license_notice();
        } else if(strcmp(arg, "w") == 0){
            printf("\033[1;34mThis program is distributed in the hope that it will be useful,\n");
            printf("but WITHOUT ANY WARRANTY; without even the implied warranty of\n");
            printf("MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n");
            printf("GNU General Public License for more details.\033[0m\n");
        } else if(strcmp(arg, "c") == 0){
            printf("\033[1;36mYou can redistribute it and/or modify,\n");
            printf("under these conditions:\n");
            printf("1. You must give credit to the original author (RUN1/RUN1-CS).\n");
            printf("2. You must include this license notice in any copies or substantial portions of the software.\n");
            printf("\033[0m");
        } else {
            printf("\033[1;31mUnknown argument for show: %s\033[0m\n", arg);
        }
        return true;
    }
    return false;
}

// Greeting message based on time of day
void greeting(){
    // Get current time
    time_t now = time(NULL);
    // Convert to local time
    struct tm *t = localtime(&now);
    // We just need hours
    int hour = t->tm_hour;
    // Print greeting based on time of day
    if(hour >= 5 && hour < 12){
        printf("\033[1;32mGood morning! Welcome to XXSH.\033[0m\n");
    } else if(hour >= 12 && hour < 18){
        printf("\033[1;33mGood afternoon! Welcome to XXSH.\033[0m\n");
    } else {
        printf("\033[1;34mGood evening! Welcome to XXSH.\033[0m\n");
    }
}

// Execute a command with optional piping
int execute_command(char *command, bool *running){
    char *saveptr = NULL;
    char *segment = strtok_r(command, ";", &saveptr);

    while(segment != NULL){
        while(*segment == ' ' || *segment == '\t') segment++;
        char *end = segment + strlen(segment);
        while(end > segment && (end[-1] == ' ' || end[-1] == '\t' || end[-1] == '\n')) *--end = '\0';

        if(*segment != '\0'){
            char *args[MAX_COMMAND_LENGTH / 2 + 1];
            int argc = 0;
            char *token = strtok(segment, " \t");

            while(token != NULL && argc < (MAX_COMMAND_LENGTH / 2)){
                args[argc++] = token;
                token = strtok(NULL, " \t");
            }
            args[argc] = NULL;

            if(argc > 0){
                if(specials(args[0], (argc > 1) ? args[1] : NULL, running)){
                    segment = strtok_r(NULL, ";", &saveptr);
                    continue;
                }else if(strncmp(args[0], "cd", 2) == 0){
                    const char *path = (argc > 1) ? args[1] : getenv("HOME");
                    if(chdir(path) != 0){
                        perror("cd");
                    }
                    segment = strtok_r(NULL, ";", &saveptr);
                    continue;
                }

                int fd = -1;

                int pipe_indices[argc];
                int pipe_count = 0;
                for(int i = 0; i < argc; i++){
                    if(strcmp(args[i], "|") == 0){
                        pipe_indices[pipe_count++] = i;
                        args[i] = NULL;
                    }else if(strcmp(args[i], ">>") == 0){
                        fd = open(args[i + 1], O_WRONLY | O_CREAT | O_APPEND, 0644);
                        if(fd == -1){
                            perror("open");
                            return -1;
                        }
                    }else if(strcmp(args[i], ">") == 0){
                        fd = open(args[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
                        if(fd == -1){
                            perror("open");
                            return -1;
                        }
                    }
                }

                if(pipe_count == 0){
                    pid_t pid = fork();
                    if(pid == 0){
                        if(fd != -1){
                            dup2(fd, STDOUT_FILENO);
                            close(fd);
                        }
                        execvp(args[0], args);
                        printf("\033[1;31mCommand not found: %s\033[0m\n", args[0]);
                        exit(EXIT_FAILURE);
                    } else if(pid > 0){
                        int status = 0;
                        waitpid(pid, &status, 0);
                        if(!WIFEXITED(status) || WEXITSTATUS(status) != 0){
                            return -1;
                        }
                    } else {
                        perror("fork");
                        return -1;
                    }
                } else {
                    int cmd_start = 0;
                    int prev_read = -1;
                    pid_t pids[pipe_count + 1];
                    int pid_count = 0;

                    for(int i = 0; i <= pipe_count; i++){
                        int fd[2] = {-1, -1};
                        if(i < pipe_count && pipe(fd) == -1){
                            perror("pipe");
                            return -1;
                        }

                        pid_t pid = fork();
                        if(pid == 0){
                            signal(SIGINT, SIG_DFL);
                            signal(SIGQUIT, SIG_DFL);
                            signal(SIGTSTP, SIG_DFL);
                            signal(SIGTTIN, SIG_DFL);
                            signal(SIGTTOU, SIG_DFL);
                            signal(SIGCHLD, SIG_DFL);

                            if(prev_read != -1) dup2(prev_read, STDIN_FILENO);
                            if(i < pipe_count) dup2(fd[1], STDOUT_FILENO);

                            if(prev_read != -1) close(prev_read);
                            if(i < pipe_count){ close(fd[0]); close(fd[1]); }

                            execvp(args[cmd_start], &args[cmd_start]);
                            perror("execvp");
                            exit(EXIT_FAILURE);
                        } else if(pid > 0){
                            pids[pid_count++] = pid;
                            if(prev_read != -1) close(prev_read);
                            if(i < pipe_count){
                                close(fd[1]);
                                prev_read = fd[0];
                            } else {
                                prev_read = -1;
                            }
                            cmd_start = (i < pipe_count) ? pipe_indices[i] + 1 : cmd_start;
                        } else {
                            perror("fork");
                            if(prev_read != -1) close(prev_read);
                            if(i < pipe_count){ close(fd[0]); close(fd[1]); }
                            return -1;
                        }
                    }

                    for(int i = 0; i < pid_count; i++){
                        int status = 0;
                        waitpid(pids[i], &status, 0);
                        if(!WIFEXITED(status) || WEXITSTATUS(status) != 0){
                            return -1;
                        }
                    }
                }
            }
        }

        segment = strtok_r(NULL, ";", &saveptr);
    }

    return 0;
}

bool handle_operators(char *op, Variable left, Variable right){
    if(strcmp(op, "==") == 0){
        if(left.type == VAR_NUMBER || right.type == VAR_NUMBER){
            return left.value.number_value == right.value.number_value;
        }
        if(left.type == VAR_BOOL || right.type == VAR_BOOL){
            return left.value.bool_value == right.value.bool_value;
        }
        return strcmp(left.value.string_value, right.value.string_value) == 0;
    }
    if(strcmp(op, "!=") == 0){
        return !handle_operators("==", left, right);
    }
    if(strcmp(op, ">") == 0) return left.value.number_value > right.value.number_value;
    if(strcmp(op, "<") == 0) return left.value.number_value < right.value.number_value;
    if(strcmp(op, ">=") == 0) return left.value.number_value >= right.value.number_value;
    if(strcmp(op, "<=") == 0) return left.value.number_value <= right.value.number_value;
    return false;
}

Variable get_variable(Variable *variables, int * var_count, char * src){
    // It's a variable, we can look it up
    for(int i = 0; i < *var_count; i++){
        if(strcmp(variables[i].name, src) == 0){
            return variables[i];
        }
    }
    FILE *DEBUG_LOG = fopen("debug.log", "a");
    if(DEBUG_LOG) {
        fprintf(DEBUG_LOG, "DEBUG: Variable '%s' not found, returning NULL\n", src);
        fclose(DEBUG_LOG);
    }
    
    return (Variable){0}; // Return NULL if variable not found
}

void parse_operand(char *operand, Variable *value, Variable *variables, int *var_count){
    if(operand[0] >= '0' && operand[0] <= '9'){
        // It's a number, we can convert it to a string for comparison
        char num_str[64];
        snprintf(num_str, sizeof(num_str), "%d", atoi(operand));
        value->value.number_value = atof(num_str);
    }else if(strcmp(operand, "true") == 0 || strcmp(operand, "false") == 0){
        // It's a boolean, convert to 1 or 0
        int bool_val = (strcmp(operand, "true") == 0) ? 1 : 0;
        value->value.bool_value = bool_val;
    }else if(operand[0] == '"' || operand[0] == '\'') {
        // It's a string literal, we can remove the quotes for comparison
        char str_literal[256];
        strncpy(str_literal, operand + 1, sizeof(str_literal) - 1);
        str_literal[strlen(str_literal) - 1] = '\0'; // Remove closing quote
        strcpy(operand, str_literal);
    }else{
        // It's a variable, we can look it up
        *value = get_variable(variables, var_count, operand);
    }
}

bool is_literal(const char *str) {
    if (str[0] >= '0' && str[0] <= '9') return true;
    if (strcmp(str, "true") == 0 || strcmp(str, "false") == 0) return true;
    if (str[0] == '"' || str[0] == '\'') return true;
    return false;
}

bool handle_condition(char line[MAX_COMMAND_LENGTH], Variable *variables, int *var_count){
    char type[8] = {0};
    char left[256] = {0};
    char op[8] = {0};
    char right[256] = {0};

    if(sscanf(line, "%7s %255s %7s %255s", type, left, op, right) < 1) return false;

    if(strcmp(type, "else") == 0){
        return true;
    }else if(strcmp(type, "if") == 0 || strcmp(type, "elif") == 0){
        if(left[0] == '\0' || op[0] == '\0' || right[0] == '\0') return false;
        Variable left_value = {0};
        if (!is_literal(left)) {
            left_value = get_variable(variables, var_count, left);
        }
        parse_operand(left, &left_value, variables, var_count);
        Variable right_value = {0};
        if (!is_literal(right)) {
            right_value = get_variable(variables, var_count, right);
        }
        parse_operand(right, &right_value, variables, var_count);
        return handle_operators(op, left_value, right_value);
    }
    return false;
}

bool handle_loop(char line[MAX_COMMAND_LENGTH], Variable *variables, int *var_count){
    char type[8] = {0};
    char left[256] = {0};
    char op[8] = {0};
    char right[256] = {0};

    if(sscanf(line, "%7s %255s %7s %255s", type, left, op, right) < 1) return false;

    if(strcmp(type, "loop") == 0){
        if(left[0] == '\0' || op[0] == '\0' || right[0] == '\0') return false;
        Variable left_value = {0};
        if (!is_literal(left)) {
            left_value = get_variable(variables, var_count, left);
        }
        parse_operand(left, &left_value, variables, var_count);
        Variable right_value = {0};
        if (!is_literal(right)) {
            right_value = get_variable(variables, var_count, right);
        }
        parse_operand(right, &right_value, variables, var_count);
        return handle_operators(op, left_value, right_value);
    }
    return false;
}

typedef struct Loop{
    int depth;
    int start_line;
    long file_pos;
    char condition[MAX_COMMAND_LENGTH];
} Loop;

// Interpret (WIP) and run a script file
void run_script(const char *filename, bool *running, Variable *variables, int *var_count){
    FILE *file = fopen(filename, "r");
    if(!file) return;
    FILE *DEBUG_LOG = fopen("debug.log", "a");
    if(DEBUG_LOG) {
        fprintf(DEBUG_LOG, "Running script: %s\n", filename);
    }

    char line[MAX_COMMAND_LENGTH];
    bool script_running = *running; // Temporary flag for the script session

    bool handle_elif = false;
    bool loop_active = false;
    bool skip_until_endif = false;
    Loop loops[100]; // Assuming a maximum of 100 nested loops
    int loop_count = 0;

    int line_counter = 0;

    int opened_ifs = 0;

    while(fgets(line, sizeof(line), file)){
        line_counter++;
        char *saveptr = NULL;
        char *segment = strtok_r(line, "\n", &saveptr);
        while(segment != NULL){
            char *cmd_save = NULL;
            char *command = strtok_r(segment, ";", &cmd_save);
            while(command != NULL){
                char *trimmed = command;
                while(*trimmed && isspace((unsigned char)*trimmed)) trimmed++;

                if(trimmed[0] == '\0' || trimmed[0] == '#'){
                    command = strtok_r(NULL, ";", &cmd_save);
                    continue;
                }

                fprintf(DEBUG_LOG, "DEBUG: Processing line: %s\n", trimmed);

                if(strcmp(trimmed, "endif") == 0){
                    skip_until_endif = false;
                    handle_elif = false;
                    opened_ifs--;
                    command = strtok_r(NULL, ";", &cmd_save);
                    continue;
                }
                if(strncmp(trimmed, "#!", 2) == 0){
                    command = strtok_r(NULL, ";", &cmd_save);
                    continue;
                }
                if(skip_until_endif){
                    command = strtok_r(NULL, ";", &cmd_save);
                    continue;
                }
                if(strncmp(trimmed, "set", 3) == 0){ set_variable(trimmed, variables, var_count); command = strtok_r(NULL, ";", &cmd_save); continue; }
                if(strncmp(trimmed, "increase", 8) == 0 || strncmp(trimmed, "inc", 3) == 0){
                    char var_name[64];
                    if(sscanf(trimmed, "%*s %63s", var_name) == 1){
                        for(int i = 0; i < *var_count; i++) if(strcmp(variables[i].name, var_name) == 0 && variables[i].type == VAR_NUMBER){ variables[i].value.number_value++; break; }
                    }
                    command = strtok_r(NULL, ";", &cmd_save); continue;
                }
                if(strncmp(trimmed, "decrease", 8) == 0 || strncmp(trimmed, "dec", 3) == 0){
                    char var_name[64];
                    if(sscanf(trimmed, "%*s %63s", var_name) == 1){
                        for(int i = 0; i < *var_count; i++) if(strcmp(variables[i].name, var_name) == 0 && variables[i].type == VAR_NUMBER){ variables[i].value.number_value--; break; }
                    }
                    command = strtok_r(NULL, ";", &cmd_save); continue;
                }
                if(strncmp(trimmed, "echo", 4) == 0){
                    char *echo_content = trimmed + 4;
                    while(*echo_content && isspace((unsigned char)*echo_content)) echo_content++;
                    if(*echo_content == '\0'){
                        // nothing to echo
                        command = strtok_r(NULL, ";", &cmd_save);
                        continue;
                    }
                    if(echo_content[0] == '"' || echo_content[0] == '\''){
                        char str_literal[256] = {0};
                        size_t len = strnlen(echo_content + 1, sizeof(str_literal) - 1);
                        // copy up to closing quote if present
                        strncpy(str_literal, echo_content + 1, len);
                        if(len > 0){
                            char last = str_literal[len - 1];
                            if(last == '"' || last == '\'') str_literal[len - 1] = '\0';
                        }
                        printf("%s\n", str_literal);
                    } else {
                        char var_name[64];
                        if(sscanf(echo_content, "%63s", var_name) == 1){
                            Variable var = get_variable(variables, var_count, var_name);
                            switch(var.type){
                                case VAR_STRING:
                                    if(var.value.string_value[0]) printf("%s\n", var.value.string_value);
                                    break;
                                case VAR_NUMBER:
                                    printf("%g\n", var.value.number_value);
                                    break;
                                case VAR_BOOL:
                                    printf("%s\n", var.value.bool_value ? "true" : "false");
                                    break;
                                default:
                                    break;
                            }
                        }
                    }
                    command = strtok_r(NULL, ";", &cmd_save);
                    continue;
                }

                if(strncmp(trimmed, "export_path", 11) == 0){
                    char value[256];
                    if(sscanf(trimmed, "export_path %[^\n]", value) == 1){
                        char *current_path = getenv("PATH");
                        char new_path[2048];
                        if(current_path){
                            snprintf(new_path, sizeof(new_path), "%s:%s", current_path, value);
                        } else {
                            strncpy(new_path, value, sizeof(new_path) - 1);
                            new_path[sizeof(new_path) - 1] = '\0';
                        }
                        if(setenv("PATH", new_path, 1) != 0){
                            perror("setenv");
                        }
                    } else {
                        printf("\033[1;31mUsage: export_path <new_path>\033[0m\n");
                    }
                    command = strtok_r(NULL, ";", &cmd_save);
                    continue;
                }

                if(strncmp(trimmed, "if ", 3) == 0 || (handle_elif && strncmp(trimmed, "elif", 4) == 0)){
                    if(strncmp(trimmed, "if ", 3) == 0) opened_ifs++;
                    bool condition_ok = handle_condition(trimmed, variables, var_count);
                    if(!condition_ok) {
                        skip_until_endif = true;
                    } else {
                        char *body = NULL;
                        char *cursor = trimmed;
                        if(strncmp(cursor, "if", 2) == 0){
                            cursor += 2;
                        } else if(strncmp(cursor, "elif", 4) == 0){
                            cursor += 4;
                        }
                        while(*cursor && isspace((unsigned char)*cursor)) cursor++;

                        for(int token = 0; token < 3 && *cursor; token++){
                            while(*cursor && !isspace((unsigned char)*cursor)) cursor++;
                            while(*cursor && isspace((unsigned char)*cursor)) cursor++;
                        }
                        if(*cursor != '\0') body = cursor;

                        if(body && *body){
                            execute_command(body, &script_running);
                        }
                    }
                    handle_elif = false;
                    command = strtok_r(NULL, ";", &cmd_save);
                    continue;
                }
                if(strncmp(trimmed, "loop", 4) == 0){
                    if(!handle_loop(trimmed, variables, var_count)){
                        skip_until_endif = true;
                    }else{
                        loops[loop_count].depth = loop_count;
                        loops[loop_count].start_line = line_counter;
                        loops[loop_count].file_pos = ftell(file);
                        strncpy(loops[loop_count].condition, trimmed, sizeof(loops[loop_count].condition) - 1);
                        loops[loop_count].condition[sizeof(loops[loop_count].condition) - 1] = '\0';
                        loop_count++;
                        loop_active = true;
                    }
                    command = strtok_r(NULL, ";", &cmd_save);
                    continue;
                }
                if(loop_active && strncmp(trimmed, "endloop", 7) == 0){
                    if(handle_loop(loops[loop_count - 1].condition, variables, var_count)){
                        fseek(file, loops[loop_count - 1].file_pos, SEEK_SET);
                        line_counter = loops[loop_count - 1].start_line;
                    } else {
                        loop_count--;
                        if(loop_count == 0) loop_active = false;
                    }
                    command = strtok_r(NULL, ";", &cmd_save);
                    continue;
                }

                if(execute_command(trimmed, &script_running) != 0){
                    break;
                }

                command = strtok_r(NULL, ";", &cmd_save);
            }
            segment = strtok_r(NULL, "\n", &saveptr);
        }
    }
    if(opened_ifs > 0){
        printf("\033[1;31mError: %d unclosed 'if' statement(s) detected in script %s\033[0m\n", opened_ifs, filename);
    }
    fclose(DEBUG_LOG);
    fclose(file);
}

int main(int argc, char **argv){
    // 1. Startup logic (.xxshrc)
    bool running = true;

    Variable variables[VAR_LIMIT]; // Assuming a maximum of 100 variables
    int var_count = 0;

    char rc_path[256];
    snprintf(rc_path, sizeof(rc_path), "%s/.xxshrc", getenv("HOME"));
    if(access(rc_path, F_OK) != -1){
        run_script(rc_path, &running, variables, &var_count);
    }
    getenv("PATH");

    // 2. SCRIPT MODE: Check if a script was passed as an argument
    if (argc > 1) {
        run_script(argv[1], &running, variables, &var_count);
        return 0; // Exit after running the script
    }

    // 3. INTERACTIVE MODE
    greeting();
    srand((unsigned int)time(NULL));
    
    char * command = malloc(1);
    if(command == NULL) {
        fprintf(stderr, "Error: Failed to allocate memory for command\n");
        return 1;
    }
    *command = '\0';

    setup_signals();

    while(running){
        char prompt[256];
        char *user = getenv("USER");
        char hostname[256];
        gethostname(hostname, sizeof(hostname));
        char cwd[256];
        getcwd(cwd, sizeof(cwd));
        snprintf(prompt, sizeof(prompt), "\033[1;36m%.31s@%.63s[XX]:\033[1;34m%.127s\033[0m$ ",
             user ? user : "user", hostname, cwd);

        printf("%s", prompt);
        fflush(stdout);

        char * input = readline("");
        if(input == NULL) { printf("\n"); break; }
        if(strlen(input) == 0) { free(input); continue; }
        if(input && *input) add_history(input);
        
        char *saveptr = NULL;
        char *segment = strtok_r(input, ";", &saveptr);
        while(segment != NULL){
            char *trimmed = segment;
            while(*trimmed && isspace((unsigned char)*trimmed)) trimmed++;

            if(*trimmed != '\0'){
                command = realloc(command, strlen(trimmed) + 1);
                if(command == NULL) {
                    fprintf(stderr, "Error: Failed to allocate memory for command\n");
                    break;
                }
                strcpy(command, trimmed);
                execute_command(command, &running);
            }

            segment = strtok_r(NULL, ";", &saveptr);
        }
        free(input);
    }
    free(command);
    return 0;
}
#endif // MAIN_C