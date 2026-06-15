#ifndef VARIABLES_C
#define VARIABLES_C

#ifndef VAR_LIMIT
#define VAR_LIMIT 100 // Later will be dynamic, I am just too lazy to implement that right now
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


#include "variables.h" // Include the variables header for get_variable function

// Function to get a variable by its name
Variable get_variable(Variable *variables, int * var_count, char * src){
    // It's a variable, we can look it up
    for(int i = 0; i < *var_count; i++){
        if(strcmp(variables[i].name, src) == 0){
            return variables[i];
        }
    }
    
    return (Variable){0}; // Return NULL if variable not found
}

// Function to set a variable based on a command line input
void set_variable(const char *line, Variable *variables, int *var_count){
    // Temporary buffers to hold parsed name, type, and value
    char name[64];
    char value[256];
    char type[16];

    // Syntax for setting variables: set <name> <type> = <value>
    if(sscanf(line, "set %63s %15s = %[^\n]", name, type, value) != 3){
        printf("\033[1;31mUsage: set <variable> <type> = <value>\033[0m\n");
        return;
    }
    // Checking types
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
            return;
        }
    }
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
        (*var_count)++;
    } else {
        printf("\033[1;31mVariable limit reached (%d)\033[0m\n", VAR_LIMIT);
    }
}

#endif // VARIABLES_C