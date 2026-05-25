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

#include <sys/wait.h>

#define bool int
#define true 1
#define false 0
#define baka -1

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
    char *args[MAX_COMMAND_LENGTH / 2 + 1]; // Maximum number of arguments
    int argc = 0;
    char *token = strtok(command, " \t");

    // Tokenize the command into arguments
    while(token != NULL && argc < (MAX_COMMAND_LENGTH / 2)){
        args[argc++] = token;
        token = strtok(NULL, " \t");
    }
    args[argc] = NULL;

    // If no command was entered, just return
    if(argc == 0) return 0;

    // Check for special built-in commands like exit/show before exec
    if(specials(args[0], (argc > 1) ? args[1] : NULL, running)) return 0;

    // Check for pipes
    int pipe_indices[argc];
    int pipe_count = 0;
    // Identify pipe positions and split it into separate commands
    for(int i = 0; i < argc; i++){
        if(strcmp(args[i], "|") == 0){
            pipe_indices[pipe_count++] = i;
            args[i] = NULL;
        }
    }

    // No pipes - simple command
    if(pipe_count == 0){
        // Handle signals in the child process
        pid_t pid = fork();
        if(pid == 0){
            execvp(args[0], args);
            perror("execvp");
            exit(EXIT_FAILURE);
        } else if(pid > 0){
            wait(NULL);
        } else {
            perror("fork");
            return -1;
        }
        return 0;
    }

    // Handle pipes without changing the shell's own stdin/stdout
    int cmd_start = 0;
    int prev_read = -1;
    // Store child PIDs to wait for them later
    pid_t pids[pipe_count + 1];
    int pid_count = 0;

    // Loop through each command segment separated by pipes
    for(int i = 0; i <= pipe_count; i++){
        int fd[2] = {-1, -1};
        // Create a pipe for all but the last command
        if(i < pipe_count && pipe(fd) == -1){
            perror("pipe");
            return -1;
        }

        // Fork a child process for the current command segment
        pid_t pid = fork();
        if(pid == 0){ // Child process

            // Reset signal handlers to default in the child process
            signal(SIGINT, SIG_DFL);
            signal(SIGQUIT, SIG_DFL);
            signal(SIGTSTP, SIG_DFL);
            signal(SIGTTIN, SIG_DFL);
            signal(SIGTTOU, SIG_DFL);
            signal(SIGCHLD, SIG_DFL);

            // If this is not the first command, set stdin to the previous pipe's read end
            if(prev_read != -1){
                dup2(prev_read, STDIN_FILENO);
            }
            // If this is not the last command, set stdout to the current pipe's write end
            if(i < pipe_count){
                dup2(fd[1], STDOUT_FILENO);
            }

            // Close unused file descriptors in the child process
            if(prev_read != -1) close(prev_read);
            if(i < pipe_count){
                close(fd[0]);
                close(fd[1]);
            }

            // Execute the command segment
            execvp(args[cmd_start], &args[cmd_start]);
            perror("execvp");
            exit(EXIT_FAILURE);
        } else if(pid > 0){ // Parent process
            pids[pid_count++] = pid;

            // Close unused file descriptors in the parent process
            if(prev_read != -1) close(prev_read);
            if(i < pipe_count){
                close(fd[1]);
                prev_read = fd[0];
            } else {
                prev_read = -1;
            }

            // Update cmd_start for the next command segment
            cmd_start = (i < pipe_count) ? pipe_indices[i] + 1 : cmd_start;
        } else {
            // Fork failed
            perror("fork");
            if(prev_read != -1) close(prev_read);
            if(i < pipe_count){
                close(fd[0]);
                close(fd[1]);
            }
            return -1;
        }
    }
    
    // Wait for all child processes
    for(int i = 0; i < pid_count; i++){
        waitpid(pids[i], NULL, 0);
    }
    
    return 0;
}

// Interpret (WIP) and run a script file
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