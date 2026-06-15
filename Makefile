# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -Wextra -O2 -g -MMD -MP

# Source files
SOURCES = main.c executor.c

# Object files
OBJECTS = $(SOURCES:.c=.o)

# Dependency files
DEPS = $(SOURCES:.c=.d)

all: xxsh

xxsh: $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^

-include $(DEPS)

%.o: %.c
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

clean:
	rm -f $(OBJECTS) $(DEPS) xxsh