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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include <sys/wait.h>

#define bool int
#define true 1
#define false 0
#define baka -1

#include <readline/readline.h>
#include <readline/history.h>

#define MAX_COMMAND_LENGTH 1024

void license_notice(){
    printf("XXSH  Copyright (C) 2026  RUN1/RUN1-CS\n");
    printf("This program comes with ABSOLUTELY NO WARRANTY; for details type `show w'.\n");
    printf("This is free software, and you are welcome to redistribute it\n");
    printf("under certain conditions; type `show c' for details.\n");
}

bool specials(const char * command, const char *arg, bool *running){
    if(strcmp(command, "exit") == 0) { *running = false; return true; } // Exit command

    if(strcmp(command, "45510") == 0){
        printf("\033[1;35m[Even if it's a lie, I wanted it to be the truth.]\033[0m\n");
        return true;
    }

    if(strcmp(command, "86") == 0){
        printf("\033[1;31m[We will fight, and we will move forward.]\033[0m\n");
        return true;
    }

    if(strcmp(command, "02") == 0){
        int rnd = rand() % 2;
        if(rnd == 0) printf("\033[1;95m[If you don't take a risk, you can't create a future.]\033[0m\n");
        else printf("\033[1;95m[If you don't belong here, just build a place where you do.]\033[0m\n");
        return true;
    }

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

void greeting(){
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    int hour = t->tm_hour;
    if(hour >= 5 && hour < 12){
        printf("\033[1;32mGood morning! Welcome to XXSH.\033[0m\n");
    } else if(hour >= 12 && hour < 18){
        printf("\033[1;33mGood afternoon! Welcome to XXSH.\033[0m\n");
    } else {
        printf("\033[1;34mGood evening! Welcome to XXSH.\033[0m\n");
    }
}

int execute_command(char *command, bool *running){
    char *args[MAX_COMMAND_LENGTH / 2 + 1]; // Maximum number of arguments
    int argc = 0;
    char *token = strtok(command, " \t");

    while(token != NULL && argc < (MAX_COMMAND_LENGTH / 2)){
        args[argc++] = token;
        token = strtok(NULL, " \t");
    }
    args[argc] = NULL;

    if(argc == 0) return 0;

    // Check for special built-in commands like exit/show before exec
    if(specials(args[0], (argc > 1) ? args[1] : NULL, running)) return 0;

    // Pipe support for multiple stages: cmd1 | cmd2 | cmd3
    int pipe_count = 0;
    for(int i = 0; i < argc; ++i){
        if(strcmp(args[i], "|") == 0) ++pipe_count;
    }

    if(pipe_count == 0){
        if(fork() == 0) { // Child process to execute the command
            if(execvp(args[0], args) == -1){
                printf("\033[1;31mBaka, command not found: %s\033[0m\n", args[0]);
                _exit(127);
            }
        }
        wait(NULL); // Wait for the child process to finish
    } else {
        bool cleanup_triggered = false;

        int stage_count = pipe_count + 1;
        char *cmds[stage_count][MAX_COMMAND_LENGTH / 2 + 1];
        int cmd_argc[stage_count];
        int stage = 0;

        for(int i = 0; i < stage_count; ++i) cmd_argc[i] = 0;

        for(int i = 0; i < argc; ++i){
            if(strcmp(args[i], "|") == 0){
                cmds[stage][cmd_argc[stage]] = NULL;
                ++stage;
            } else {
                cmds[stage][cmd_argc[stage]++] = args[i];
            }
        }
        cmds[stage][cmd_argc[stage]] = NULL;

        int pipes[pipe_count][2];
        for(int i = 0; i < pipe_count; ++i){
            if(pipe(pipes[i]) == -1){
                perror("pipe");
                cleanup_triggered = true;
                break;
            }
        }

        if(!cleanup_triggered){
            for(int i = 0; i < stage_count; ++i){
                pid_t pid = fork();
                if(pid == 0){
                    if(i > 0) dup2(pipes[i - 1][0], STDIN_FILENO);
                    if(i < pipe_count) dup2(pipes[i][1], STDOUT_FILENO);

                    for(int j = 0; j < pipe_count; ++j){
                        close(pipes[j][0]);
                        close(pipes[j][1]);
                    }

                    if(execvp(cmds[i][0], cmds[i]) == -1){
                        perror("Baka, command not found: ");
                        _exit(127);
                    }
                }
            }

            for(int i = 0; i < pipe_count; ++i){
                close(pipes[i][0]);
                close(pipes[i][1]);
            }

            for(int i = 0; i < stage_count; ++i) wait(NULL);
        }
    }
    return 0;
}

void run_script(const char *filename){
    FILE *file = fopen(filename, "r");
    if(!file) return;

    char line[MAX_COMMAND_LENGTH];
    bool script_running = true; // Temporary flag for the script session

    while(fgets(line, sizeof(line), file)){
        // 1. Clean the line
        line[strcspn(line, "\r\n")] = 0;
        
        // 2. Skip comments/empty
        if(line[0] == '#' || line[0] == '\0') continue; 

        // 3. Just pass the whole line to execute_command
        // Don't strtok here! Let execute_command handle the parsing.
        execute_command(line, &script_running);
    }
    fclose(file);
}

int main(int argc, char **argv){
    // 1. Startup logic (.xxshrc)
    char rc_path[256];
    snprintf(rc_path, sizeof(rc_path), "%s/.xxshrc", getenv("HOME"));
    if(access(rc_path, F_OK) != -1){
        run_script(rc_path);
    }

    // 2. SCRIPT MODE: Check if a script was passed as an argument
    if (argc > 1) {
        run_script(argv[1]);
        return 0; // Exit after running the script
    }

    // 3. INTERACTIVE MODE
    greeting();
    srand((unsigned int)time(NULL));
    
    char command[MAX_COMMAND_LENGTH];
    bool running = true;

    while(running){
        printf("XX> ");
        fflush(stdout);

        char * input = readline("");
        if(input == NULL) { printf("\n"); break; }
        if(strlen(input) == 0) { free(input); continue; }
        if(input && *input) add_history(input);
        
        strcpy(command, input);
        free(input);

        execute_command(command, &running);
    }
    return 0;
}
#endif // MAIN_C