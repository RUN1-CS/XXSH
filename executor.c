#ifndef EXECUTOR_C
#define EXECUTOR_C

#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif

#include <string.h>
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
void license_notice(){
    str_print("XXSH  Copyright (C) 2026  RUN1/RUN1-CS\n");
    str_print("This program comes with ABSOLUTELY NO WARRANTY; for details type `show w'.\n");
    str_print("This is free software, and you are welcome to redistribute it\n");
    str_print("under certain conditions; type `show c' for details.\n");
}

// Check for special built-in commands that don't require forking
bool specials(const char * command, const char *arg, bool *running){
    if(strcmp(command, "exit") == 0) { *running = false; return true; } // Exit command

    // Show command for license and credits (GPL 3.0 compatible)
    if(strcmp(command, "show") == 0){
        if(arg == NULL){
            license_notice();
        } else if(strcmp(arg, "w") == 0){
            str_print("\033[1;34mThis program is distributed in the hope that it will be useful,\n");
            str_print("but WITHOUT ANY WARRANTY; without even the implied warranty of\n");
            str_print("MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n");
            str_print("GNU General Public License for more details.\033[0m\n");
        } else if(strcmp(arg, "c") == 0){
            str_print("\033[1;36mYou can redistribute it and/or modify,\n");
            str_print("under these conditions:\n");
            str_print("1. You must give credit to the original author (RUN1/RUN1-CS).\n");
            str_print("2. You must include this license notice in any copies or substantial portions of the software.\n");
            str_print("\033[0m");
        } else {
            str_print("\033[1;31mUnknown argument for show.\033[0m\n");
        }
        return true;
    }
    return false;
}

// Execute a command with optional piping
int execute_command(char *command, bool *running){
    str_print("Can't execute commands yet, kernel is still being developed.\n");
    /*
    char *saveptr = NULL;
    char *segment = strtok_r(command, ";", &saveptr); // Splitting by ';' to allow multiple commands in one line

    while(segment != NULL){
        // Trim leading and trailing whitespace
        while(*segment == ' ' || *segment == '\t') segment++;
        char *end = segment + strlen(segment);
        while(end > segment && (end[-1] == ' ' || end[-1] == '\t' || end[-1] == '\n')) *--end = '\0';

        if(*segment != '\0'){
            char *args[MAX_COMMAND_LENGTH / 2 + 1];
            int argc = 0;
            char *token = strtok(segment, " \t");

            // Tokenize the command into arguments, respecting the maximum command length
            while(token != NULL && argc < (MAX_COMMAND_LENGTH / 2)){
                args[argc++] = token;
                token = strtok(NULL, " \t");
            }
            args[argc] = NULL;

            if(argc > 0){
                // Check for special built-in commands that don't require forking
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

                // Initialize file descriptor for redirection
                int fd = -1;

                // First pass to handle redirection and find pipe positions
                int pipe_indices[argc];
                int pipe_count = 0;
                for(int i = 0; i < argc; i++){
                    if(strcmp(args[i], "|") == 0){
                        pipe_indices[pipe_count++] = i;
                        args[i] = NULL;
                    }else if(strcmp(args[i], ">>") == 0){
                        if(i + 1 >= argc || args[i + 1] == NULL){
                            fprintf(stderr, "syntax error near unexpected token '>>'\n");
                            return -1;
                        }
                        fd = open(args[i + 1], O_WRONLY | O_CREAT | O_APPEND, 0644);
                        if(fd == -1){
                            perror("open");
                            return -1;
                        }
                        for(int j = i; j + 2 <= argc; j++){
                            args[j] = args[j + 2];
                        }
                        argc -= 2;
                        i--;
                    }else if(strcmp(args[i], ">") == 0){
                        if(i + 1 >= argc || args[i + 1] == NULL){
                            fprintf(stderr, "syntax error near unexpected token '>'\n");
                            return -1;
                        }
                        fd = open(args[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
                        if(fd == -1){
                            perror("open");
                            return -1;
                        }
                        for(int j = i; j + 2 <= argc; j++){
                            args[j] = args[j + 2];
                        }
                        argc -= 2;
                        i--;
                    }
                }

                // If there are no pipes, we can execute the command directly
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
                    // Handle piped commands
                    int cmd_start = 0;
                    int prev_read = -1;
                    pid_t pids[pipe_count + 1];
                    int pid_count = 0;

                    for(int i = 0; i <= pipe_count; i++){
                        int pipefd[2] = {-1, -1};
                        if(i < pipe_count && pipe(pipefd) == -1){
                            perror("pipe");
                            return -1;
                        }

                        pid_t pid = fork();
                        if(pid == 0){
                            // Child process
                            // Reset signal handlers to default in child processes to allow proper signal handling (e.g., Ctrl+C to kill the process)
                            signal(SIGINT, SIG_DFL);
                            signal(SIGQUIT, SIG_DFL);
                            signal(SIGTSTP, SIG_DFL);
                            signal(SIGTTIN, SIG_DFL);
                            signal(SIGTTOU, SIG_DFL);
                            signal(SIGCHLD, SIG_DFL);

                            // Set up input and output for the current command
                            if(prev_read != -1) dup2(prev_read, STDIN_FILENO);
                            if(i < pipe_count) dup2(pipefd[1], STDOUT_FILENO);
                            if(i == pipe_count && fd != -1) dup2(fd, STDOUT_FILENO);

                            if(prev_read != -1) close(prev_read);
                            if(i < pipe_count){ close(pipefd[0]); close(pipefd[1]); }
                            if(i == pipe_count && fd != -1) close(fd);

                            // Execute the command
                            execvp(args[cmd_start], &args[cmd_start]);
                            perror("execvp");
                            exit(EXIT_FAILURE);
                        } else if(pid > 0){
                            // Parent process
                            pids[pid_count++] = pid;
                            if(prev_read != -1) close(prev_read);
                            if(i < pipe_count){
                                close(pipefd[1]);
                                prev_read = pipefd[0];
                            } else {
                                prev_read = -1;
                            }
                            // Update cmd_start for the next command in the pipeline
                            cmd_start = (i < pipe_count) ? pipe_indices[i] + 1 : cmd_start;
                        } else {
                            perror("fork");
                            if(prev_read != -1) close(prev_read);
                            if(i < pipe_count){ close(pipefd[0]); close(pipefd[1]); }
                            return -1;
                        }
                    }

                    // Wait for all child processes in the pipeline to finish
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

        // Move to the next command segment separated by ';'
        segment = strtok_r(NULL, ";", &saveptr);
    }

    */
    return 0;
}

#endif // EXECUTOR_C