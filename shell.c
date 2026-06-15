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

#include <string.h> // For string manipulation functions (Mainly for the parser)
#include "../../kernel/drivers/video.h"

// QoL just for me and jokes
#define bool int
#define true 1
#define false 0
#define baka -1

#define MAX_COMMAND_LENGTH 1024 // Max length of a command line

#include "shell.h"
#include "executor.h"

// Greeting message
void greeting(){
    // Print greeting
    str_print("Welcome to XXSH.\n");
}

int xxsh_loop(){
    // 1. Startup logic
    //bool running = true;

    //getenv("PATH");

    // 2. INTERACTIVE MODE
    greeting();
    
    //char command[MAX_COMMAND_LENGTH];
    //command[0] = '\0';

    // Main loop for interactive mode
    /*
    while(running){
        char * input = readline("\033[1;34mXXSH> \033[0m");
        if(input == NULL) { str_print("\n"); break; }
        if(strlen(input) == 0) { free(input); continue; }
        if(input && *input) add_history(input);
        
        char *saveptr = NULL;
        char *segment = strtok_r(input, ";", &saveptr);
        while(segment != NULL){
            char *trimmed = segment;
            while(*trimmed && isspace((unsigned char)*trimmed)) trimmed++;

            if(*trimmed != '\0'){
                strcpy(command, trimmed);
                execute_command(command, &running);
            }

            segment = strtok_r(NULL, ";", &saveptr);
        }
        free(input);
    }
    */
    return 0;
}
#endif // MAIN_C
// Home sweet home. Btw if you read all the comments, find better things to do in your life.