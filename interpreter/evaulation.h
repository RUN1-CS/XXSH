#ifndef EVAULATION_H
#define EVAULATION_H

#ifndef MAX_COMMAND_LENGTH
#define MAX_COMMAND_LENGTH 1024
#endif

#ifndef bool
#define bool int
#define true 1
#define false 0
#endif

#ifndef VARIABLES_H
#include "variables.h" // Include the variables header for get_variable function
#endif

#include "types.h" // Include the types header for Variable and Loop definitions

bool handle_loop(char line[MAX_COMMAND_LENGTH], Variable *variables, int *var_count);

bool handle_condition(char line[MAX_COMMAND_LENGTH], Variable *variables, int *var_count);

bool is_literal(const char *str);

void parse_operand(char *operand, Variable *value, Variable *variables, int *var_count);

bool handle_operators(char *op, Variable left, Variable right);

#endif // EVAULATION_H