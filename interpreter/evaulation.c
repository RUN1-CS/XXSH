#ifndef EVAULATION_C
#define EVAULATION_C

#include "evaulation.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Handling logical operators for conditions and loops
bool handle_operators(char *op, Variable left, Variable right){
    // Ofc we have to respect data types, otherwise it would be a mess
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

// Function to parse an operand, which can be a literal or a variable, and store its value in a Variable struct
void parse_operand(char *operand, Variable *value, Variable *variables, int *var_count){
    if(operand[0] >= '0' && operand[0] <= '9'){
        // It's a number, we can convert it to a string for comparison
        char num_str[64];
        snprintf(num_str, sizeof(num_str), "%d", atoi(operand));
            value->type = VAR_NUMBER;
            value->value.number_value = atof(num_str);
    }else if(strcmp(operand, "true") == 0 || strcmp(operand, "false") == 0){
        // It's a boolean, convert to 1 or 0
        int bool_val = (strcmp(operand, "true") == 0) ? 1 : 0;
            value->type = VAR_BOOL;
            value->value.bool_value = bool_val;
    }else if(operand[0] == '"' || operand[0] == '\'') {
        // It's a string literal, we can remove the quotes for comparison
        char str_literal[256];
        strncpy(str_literal, operand + 1, sizeof(str_literal) - 1);
        str_literal[strlen(str_literal) - 1] = '\0'; // Remove closing quote
            value->type = VAR_STRING;
            strncpy(value->value.string_value, str_literal, sizeof(value->value.string_value));
        strcpy(operand, str_literal);
    }else{
        // It's a variable, we can look it up
        *value = get_variable(variables, var_count, operand);
    }
}

// Checking for literals (numbers, booleans, and string literals) to differentiate them from variable names
bool is_literal(const char *str) {
    if (str[0] >= '0' && str[0] <= '9') return true;
    if (strcmp(str, "true") == 0 || strcmp(str, "false") == 0) return true;
    if (str[0] == '"' || str[0] == '\'') return true;
    return false;
}

// Handling conditions for if/elif/else statements
bool handle_condition(char line[MAX_COMMAND_LENGTH], Variable *variables, int *var_count){
    char type[8] = {0};
    char left[256] = {0};
    char op[8] = {0};
    char right[256] = {0};

    // Syntax for conditions: if <left> <operator> <right>
    if(sscanf(line, "%7s %255s %7s %255s", type, left, op, right) < 1) return false;

    // Else branch is simple
    if(strcmp(type, "else") == 0){
        return true;
    }else if(strcmp(type, "if") == 0 || strcmp(type, "elif") == 0){ // but these mfs ain't
        if(left[0] == '\0' || op[0] == '\0' || right[0] == '\0') return false;
        Variable left_value = {0};
        parse_operand(left, &left_value, variables, var_count);
        Variable right_value = {0};
        parse_operand(right, &right_value, variables, var_count);
        return handle_operators(op, left_value, right_value);
    }
    return false;
}

// Loop handling, syntax: loop <left> <operator> <right>
bool handle_loop(char line[MAX_COMMAND_LENGTH], Variable *variables, int *var_count){
    char type[8] = {0};
    char left[256] = {0};
    char op[8] = {0};
    char right[256] = {0};

    if(sscanf(line, "%7s %255s %7s %255s", type, left, op, right) < 1) return false;

    if(strcmp(type, "loop") == 0){
        if(left[0] == '\0' || op[0] == '\0' || right[0] == '\0') return false;
        Variable left_value = {0};
        parse_operand(left, &left_value, variables, var_count);
        Variable right_value = {0};
        parse_operand(right, &right_value, variables, var_count);
        return handle_operators(op, left_value, right_value);
    }
    return false;
}

#endif // EVAULATION_C