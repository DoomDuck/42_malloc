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

CFLAGS = -I $(INC_DIR) -Wall -Wextra -pedantic -fPIC
SOURCES = $(shell find src/ -type f -name '*.c')
OBJECTS = $(SOURCES:$(SRC_DIR)/%.c=$(OBJS_DIR)/%.o)
DEPS = $(SOURCES:$(SRC_DIR)/%.c=$(DEPS_DIR)/%.d)
LINK = libft_malloc.so

.PHONY: all
all: $(LINK)

.PHONY: clean
clean: 
	$(RM) $(BUILD_DIR)

.PHONY: fclean
fclean: clean
	$(RM) $(NAME) $(LINK)

.PHONY: re
re: fclean
	$(MAKE) all

.PHONY: fmt
fmt:
	clang-format -i `find \( -name '*.c' -o -name '*.h' \)`

$(NAME): $(OBJECTS)
	$(CC) -o $@ --shared $^
	strip $@ -K malloc -K realloc -K free -K show_alloc_mem -K show_alloc_mem_ex

$(LINK): $(NAME)
	ln -sf $^ $@

$(OBJS_DIR)/%.o: $(SRC_DIR)/%.c
	@ mkdir -p $(@D)
	$(CC) -o $@ $(CFLAGS) -c $<

$(DEPS_DIR)/%.d: $(SRC_DIR)/%.c
	@ mkdir -p $(@D)
	$(CC) -o $@ $< $(CFLAGS) -MM -MG -MT $(patsubst $(DEPS_DIR)/%.d, $(OBJS_DIR)/%.o, $@)

-include $(DEPS)
