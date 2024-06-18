ifeq ($(HOSTTYPE),)
	HOSTTYPE := $(shell uname -m)_$(shell uname -s)
endif

NAME = libft_malloc_$(HOSTTYPE).so

CC = gcc
RM = rm -rf

INC_DIR = inc
SRC_DIR = src
OBJ_DIR = .obj

CFLAGS = -I $(INC_DIR) -Wall -Wextra -pedantic -fPIC -ggdb
SOURCES = $(shell find src/ -type f -name '*.c')
OBJECTS = $(SOURCES:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

.PHONY: all
all: $(NAME)

.PHONY: clean
clean: 
	$(RM) $(OBJ_DIR)

.PHONY: fclean
fclean: clean
	$(RM) $(NAME)

.PHONY: re
re: fclean
	$(MAKE) all

$(NAME): $(OBJECTS)
	$(CC) -o $@ --shared $^
	ln -sf $@ libft_malloc.so

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@ mkdir -p $(@D)
	$(CC) -o $@ $(CFLAGS) -c $^
