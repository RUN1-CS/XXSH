#ifndef PARSER_H
#define PARSER_H

#include "types.h" // Include the types header for Variable and Loop definitions

void run_script(const char *filename, bool *running, Variable *variables, int *var_count);

#endif // PARSER_H