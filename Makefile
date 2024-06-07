# Compiler settings
CC=gcc
CFLAGS=-std=c11 -g -Wall -Wextra
CLIBS=-lraylib -lGL -lm -lpthread -ldl -lrt -lX11

# Folder structure
SRC=src
OBJ=obj
BIN=bin

# Get all source file object names
SRCS=$(wildcard $(SRC)/*.c)
OBJS=$(patsubst $(SRC)/%.c, $(OBJ)/%.o, $(SRCS))
DEPS=$(patsubst $(SRC)/%.c, $(OBJ)/%.d, $(SRCS))

EXEC=$(BIN)/app

all: $(EXEC)

update: clean $(EXEC)

release: CFLAGS=-Wall -O2 -DNDEBUG
release: clean
release: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(CLIBS)

-include $(DEPS)

# Note about this rule checking Makefile
# We might need to clean object files
# after changing settings in this file
$(OBJ)/%.o: $(SRC)/%.c Makefile
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@ $(CLIBS)

clean:
	$(RM) $(BIN)/* $(OBJ)/*
