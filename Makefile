# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -Wextra -O2 -g -MMD -MP

# Linker flags
LDFLAGS = -lreadline

# Source files
SOURCES = main.c executor.c \
	interpreter/evaulation.c \
	interpreter/parser.c \
	interpreter/variables.c

# Object files
OBJECTS = $(SOURCES:.c=.o)

# Dependency files
DEPS = $(SOURCES:.c=.d)

all: xxsh

xxsh: $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

-include $(DEPS)

%.o: %.c
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

interpreter/%.o: interpreter/%.c
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

clean:
	rm -f $(OBJECTS) $(DEPS) xxsh