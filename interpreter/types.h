#ifndef TYPES_H
#define TYPES_H

#ifndef MAX_COMMAND_LENGTH
#define MAX_COMMAND_LENGTH 1024
#endif

#ifndef bool
#define bool int
#define true 1
#define false 0
#endif

// Loop struct to keep track of loop state for nested loops
typedef struct Loop{
    int depth;
    int start_line;
    long file_pos;
    char condition[MAX_COMMAND_LENGTH];
} Loop;

// Variable types
enum VariableType {
    VAR_STRING,
    VAR_NUMBER,
    VAR_BOOL
};

// Union to hold different types of variable values
union VariableValue {
    char string_value[256];
    double number_value;
    bool bool_value;
};

// Struct to represent a variable with its name, type, and value
typedef struct Variable {
    char name[64];
    enum VariableType type;
    union VariableValue value;
} Variable;

#endif // TYPES_H