#ifndef VARIABLES_H
#define VARIABLES_H

#include "types.h" // Include the types header for Variable and Loop definitions

Variable get_variable(Variable *variables, int * var_count, char * src);

void set_variable(const char *line, Variable *variables, int *var_count);

#endif // VARIABLES_H