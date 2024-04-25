NAME = libmalloc.so

CC = gcc
RM = rm -rf

INC_DIR = include
SRC_DIR = src
OBJ_DIR = .obj

CFLAGS = -I $(INC_DIR) -Wall -Wextra -pedantic
SOURCES = $(shell find src/ -type f -name '*.c')
OBJECTS = $(SOURCES:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

.PHONY: all
all: $(NAME)

.PHONY: clean
clean: 
	$(RM) $(OBJ_DIR)

.PHONY: fclean
fclean: 
	$(RM) $(NAME)

.PHONY: re
re: fclean
	$(MAKE) all

%.so: $(OBJECTS)
	$(CC) -o $@ --shared $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@ mkdir -p $(@D)
	$(CC) -o $@ $(CFLAGS) -c $^
