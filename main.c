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

#include <stdio.h> // Main header for input/output functions
#include <stdlib.h> // For memory management, process control, conversions
#include <string.h> // For string manipulation functions (Mainly for the parser)
#include <unistd.h> // For POSIX API access (e.g., fork, exec, chdir)
#include <time.h> // For greeting message based on time of day
#include <signal.h> // For handling signals (e.g., Ctrl+C)
#include <fcntl.h> // For file control options (e.g., open flags)

#include <sys/wait.h> // For waiting on child processes
#include <sys/types.h> // For data types used in system calls (e.g., pid_t)

// QoL just for me and jokes
#define bool int
#define true 1
#define false 0
#define baka -1

#define VAR_LIMIT 100 // Later will be dynamic, I am just too lazy to implement that right now

#include <readline/readline.h> // For command line editing and history
#include <readline/history.h> // For maintaining command history

#define MAX_COMMAND_LENGTH 1024 // Max length of a command line

#include "interpreter/parser.h"
#include "interpreter/types.h"
#include "interpreter/variables.h"
#include "interpreter/evaulation.h"

#include "executor.h"

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

    // Main loop for interactive mode
    while(running){
        char *prompt = get_prompt();
        char * input = readline(prompt);
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
// Home sweet home. Btw if you read all the comments, find better things to do in your life.