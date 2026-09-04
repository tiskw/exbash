################################################################################
# Makefile for plugins
################################################################################

.PHONY: help build build-in-docker check count clean

#-------------------------------------------------------------------------------
# Constants
#-------------------------------------------------------------------------------

# Name of the software.
SOFTWARE_PATH := omnipicker

# Source code directory.
SRC_DIR := source
OBJ_DIR := objects

# Target C++ source files.
CXX_FILES := $(wildcard $(SRC_DIR)/*.cxx)
OBJ_FILES := $(patsubst $(SRC_DIR)/%,$(OBJ_DIR)/%,$(CXX_FILES:.cxx=.o))

#-------------------------------------------------------------------------------
# C++ build settings
#-------------------------------------------------------------------------------

# Compile command.
CC     := g++ -std=c++23
CFLAGS := -O3 -march=native -flto=auto -Wall -Wextra -I/usr/local/include
LIBS   := -lncurses

# Commands for Docker-based static build.
DOCKER_IMAGE := tiskw/exbash:alpine3.23
DOCKER_BASE  := run --rm -it -u `id -u`:`id -g` -v `pwd`:/work -w /work
DOCKER_ARGS  := $(DOCKER_BASE) $(DOCKER_IMAGE) sh -c 'make -j4 $(SOFTWARE_PATH)'

# Colors.
RED     := \033[38;2;204;102;102m
GREEN   := \033[38;2;181;189;104m
YELLOW  := \033[38;2;240;198;116m
BLUE    := \033[38;2;129;162;190m
MAGENTA := \033[38;2;178;148;187m
CYAN    := \033[38;2;138;190;183m
GRAY    := \033[38;2;197;200;198m
NC      := \033[0m

#-------------------------------------------------------------------------------
# Help message
#-------------------------------------------------------------------------------

help:
	@echo "Usage:"
	@echo "    make <command>"
	@echo ""
	@echo "Build commands:"
	@echo "    build             Build C++ plugin via Docker"
	@echo "    build-in-docker   Build command called inside Docker container"
	@echo ""
	@echo "Code check commands:"
	@echo "    check             Check the code quality"
	@echo "    count             Count the lines of code"
	@echo ""
	@echo "Other commands:"
	@echo "    clean             Cleanup cache files"
	@echo "    help              Show this message"

#-------------------------------------------------------------------------------
# Build commands
#-------------------------------------------------------------------------------

build: $(OBJ_DIR)
	@printf "$(GRAY)------------------------------------------------------------$(NC)\n"
	@printf "$(GRAY)Building omnipicker$(NC)\n"
	@printf "$(GRAY)------------------------------------------------------------$(NC)\n"
	@printf "$(YELLOW)[CMD] $(GREEN)docker $(MAGENTA)$(DOCKER_ARGS)$(NC)\n"
	@docker $(DOCKER_ARGS)
	@printf "$(YELLOW)[CMD] $(GREEN)strip $(BLUE)$(SOFTWARE_PATH)$(NC)\n"
	@strip $(SOFTWARE_PATH)
	@printf "$(GRAY)------------------------------------------------------------$(NC)\n\n"

$(OBJ_DIR):
	@printf "$(YELLOW)[CMD] $(GREEN)mkdir $(NC)-p $(BLUE)$(OBJ_DIR)$(NC)\n"
	@mkdir -p $(OBJ_DIR)

$(SOFTWARE_PATH): $(OBJ_FILES)
	@printf "$(YELLOW)[LNK] $(GREEN)$(OBJ_DIR)/*.o $(NC)-> $(BLUE)$(SOFTWARE_PATH)$(NC)\n"
	@$(CC) $(CFLAGS) -o $(SOFTWARE_PATH) $(OBJ_FILES) $(LIBS) -static

objects/%.o: source/%.cxx
	@printf "$(YELLOW)[C++] $(GREEN)$(<) $(NC)-> $(BLUE)$(@)$(NC)\n"
	@$(CC) $(CFLAGS) -c $(<) -o $(@)

#-------------------------------------------------------------------------------
# Test commands
#-------------------------------------------------------------------------------

test:
	cd tests && make test

#-------------------------------------------------------------------------------
# Code check commands
#-------------------------------------------------------------------------------

check:
	cppcheck --enable=all --std=c++23 $(CXX_FILES) --suppress=missingIncludeSystem
	@echo "Check completed!"

count:
	cloc --by-file $(SRC_DIR)

#-------------------------------------------------------------------------------
# Other commands
#-------------------------------------------------------------------------------

clean:
	rm -f $(SOFTWARE_PATH)
	rm -rf $(OBJ_DIR)
	cd tests && make clean

# vim: noexpandtab tabstop=4 shiftwidth=4
