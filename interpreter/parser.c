#ifndef MAIN_C
#define MAIN_C

#ifndef MAX_COMMAND_LENGTH
#define MAX_COMMAND_LENGTH 1024
#endif

#ifndef bool
#define bool int
#define true 1
#define false 0
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "types.h" // Include the types header for Variable and Loop definitions
#include "variables.h" // Include the variables header for get_variable function
#include "evaulation.h" // Include the evaluation header for handle_condition and handle_loop functions
#include "../executor.h" // Include the executor header for license_notice function

// Interpreter and run a script file
void run_script(const char *filename, bool *running, Variable *variables, int *var_count){
    // Well, we have to open the file first
    FILE *file = fopen(filename, "r");
    if(!file) return;

    char line[MAX_COMMAND_LENGTH];
    bool script_running = *running; // Temporary flag for the script session

    bool handle_elif = false;
    bool loop_active = false;
    bool skip_until_endif = false;
    Loop loops[100]; // Assuming a maximum of 100 nested loops
    int loop_count = 0;

    // Line counter for error reporting and loop handling
    int line_counter = 0;

    // Counter for opened 'if' statements to detect unclosed blocks at the end of the script
    int opened_ifs = 0;

    // Yay, reading (it's not manga, unfortunately)
    while(fgets(line, sizeof(line), file)){
        // It's the same as normal logic
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

                // But here it changes, we handle conditions and loops here
                if(strcmp(trimmed, "endif") == 0){
                    skip_until_endif = false;
                    handle_elif = false;
                    opened_ifs--;
                    command = strtok_r(NULL, ";", &cmd_save);
                    continue;
                }
                // Speciality ofc
                if(strncmp(trimmed, "#!", 2) == 0){
                    command = strtok_r(NULL, ";", &cmd_save);
                    continue;
                }
                // If we are currently skipping commands until we find an 'endif', we just ignore everything until then
                if(skip_until_endif){
                    command = strtok_r(NULL, ";", &cmd_save);
                    continue;
                }
                // Setting a variable is also a special command that doesn't require forking, so we handle it here as well
                if(strncmp(trimmed, "set", 3) == 0){ set_variable(trimmed, variables, var_count); command = strtok_r(NULL, ";", &cmd_save); continue; }
                // And ofc we have to handle increase and decrease commands for numbers, otherwise it would be a pain to do it with 'set' every time
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
                // Echoes, scawy, especially because of variables
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

                // Exporting a path to PATH variable, it's a shell after all
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

                // Handling conditions and loops
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
    // Please close your ifs
    if(opened_ifs > 0){
        printf("\033[1;31mError: %d unclosed 'if' statement(s) detected in script %s\033[0m\n", opened_ifs, filename);
    }
    fclose(file);
}

#endif // MAIN_C