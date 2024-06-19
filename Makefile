ifeq ($(HOSTTYPE),)
	HOSTTYPE := $(shell uname -m)_$(shell uname -s)
endif

NAME = libft_malloc_$(HOSTTYPE).so

CC = gcc
RM = rm -rf

INC_DIR = inc
SRC_DIR = src
BUILD_DIR = .build
OBJS_DIR = $(BUILD_DIR)/objs
DEPS_DIR = $(BUILD_DIR)/deps

CFLAGS = -I $(INC_DIR) -Wall -Wextra -pedantic -fPIC -ggdb
SOURCES = $(shell find src/ -type f -name '*.c')
OBJECTS = $(SOURCES:$(SRC_DIR)/%.c=$(OBJS_DIR)/%.o)
DEPS = $(SOURCES:$(SRC_DIR)/%.c=$(DEPS_DIR)/%.d)

.PHONY: all
all: $(NAME)

.PHONY: clean
clean: 
	$(RM) $(BUILD_DIR)

.PHONY: fclean
fclean: clean
	$(RM) $(NAME)

.PHONY: re
re: fclean
	$(MAKE) all

$(NAME): $(OBJECTS)
	$(CC) -o $@ --shared $^
	strip $@ -K malloc -K realloc -K free -K show_alloc_mem
	ln -sf $@ libft_malloc.so

$(OBJS_DIR)/%.o: $(SRC_DIR)/%.c
	@ mkdir -p $(@D)
	$(CC) -o $@ $(CFLAGS) -c $<

$(DEPS_DIR)/%.d: $(SRC_DIR)/%.c
	@ mkdir -p $(@D)
	$(CC) -o $@ $< $(CFLAGS) -MM -MG -MT $(patsubst $(DEPS_DIR)/%.d, $(OBJS_DIR)/%.o, $@)

-include $(DEPS)
