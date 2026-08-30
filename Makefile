################################################################################
# Makefile
################################################################################

.PHONY: build release plugins test check count clean help

#-------------------------------------------------------------------------------
# Compile settings
#-------------------------------------------------------------------------------

# Software name.
EXBASH_PATH  := build/exbash
READCMD_PATH := build/exbash_readcmd

# Version number (extracted from source/bash/exbash_body).
VERSION := $(shell grep 'exbash_version="' source/bash/exbash_body | grep -o '[0-9]\+\.[0-9]\+\.[0-9]\+')

# Object directory.
SRC_DIR := source/cxx
OBJ_DIR := build/objects

# Source files.
CXX_FILES := $(wildcard $(SRC_DIR)/*.cxx)
HXX_FILES := $(wildcard $(SRC_DIR)/*.hxx)
OBJ_FILES := $(patsubst $(SRC_DIR)/%,$(OBJ_DIR)/%,$(CXX_FILES:.cxx=.o))

# Default build target.
TARGET := split

#-------------------------------------------------------------------------------
# C++ build settings
#-------------------------------------------------------------------------------

# Compile command.
CC     := g++ -std=c++23
CFLAGS := -O3 -march=native -flto=auto -Wall -Wextra -I/usr/local/include
LIBS   := -L/usr/local/lib -llua

# Commands for Docker-based static build.
DOCKER_IMAGE := tiskw/exbash:alpine3.23
DOCKER_BASE  := run --rm -it -u `id -u`:`id -g` -v `pwd`:/work -w /work
DOCKER_ARGS  := $(DOCKER_BASE) $(DOCKER_IMAGE) sh -c 'make -j4 $(EXBASH_PATH)'

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
	@echo "    build      Build ExBash"
	@echo "    plugins    Build all plugins"
	@echo "    release    Create a release package"
	@echo ""
	@echo "Test commands:"
	@echo "    test       Run tests and measure code coverage"
	@echo ""
	@echo "Code check commands:"
	@echo "    check      Check the code quality"
	@echo "    count      Count the lines of code"
	@echo ""
	@echo "Other commands:"
	@echo "    clean      Cleanup cache files"
	@echo "    help       Show this message"

#-------------------------------------------------------------------------------
# Build commands
#-------------------------------------------------------------------------------

build: $(OBJ_DIR)
	@printf "$(GRAY)------------------------------------------------------------$(NC)\n"
	@printf "$(GRAY)Building ExBash$(NC)\n"
	@printf "$(GRAY)------------------------------------------------------------$(NC)\n"
	@printf "$(YELLOW)[CMD] $(GREEN)docker $(MAGENTA)$(DOCKER_ARGS)$(NC)\n"
	@docker $(DOCKER_ARGS)
	@printf "$(GRAY)------------------------------------------------------------$(NC)\n\n"

$(OBJ_DIR):
	@printf "$(YELLOW)[CMD] $(GREEN)mkdir $(NC)-p $(BLUE)$(OBJ_DIR)$(NC)\n"
	@mkdir -p $(OBJ_DIR)

$(EXBASH_PATH): $(READCMD_PATH)
	@printf "$(YELLOW)[CMD] $(GREEN)cat $(BLUE)source/bash/exbash_body $(MAGENTA)$(READCMD_PATH) $(NC)> $(RED)$(EXBASH_PATH)$(NC)\n"
	@cat source/bash/exbash_body $(READCMD_PATH) > $(EXBASH_PATH)
	@printf "$(YELLOW)[CMD] $(GREEN)chmod $(NC)+x $(RED)$(EXBASH_PATH)$(NC)\n"
	@chmod +x $(EXBASH_PATH)

$(READCMD_PATH): $(OBJ_FILES)
	@printf "$(YELLOW)[LNK] $(GREEN)$(OBJ_DIR)/*.o $(NC)-> $(BLUE)$(READCMD_PATH)$(NC)\n"
	@$(CC) $(CFLAGS) -o $(READCMD_PATH) $(OBJ_FILES) $(LIBS) -static
	@printf "$(YELLOW)[CMD] $(GREEN)strip $(BLUE)$(READCMD_PATH)$(NC)\n"
	@strip $(READCMD_PATH)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cxx $(SRC_DIR)/%.hxx
	@printf "$(YELLOW)[C++] $(GREEN)$(<) $(NC)-> $(BLUE)$(@)$(NC)\n"
	@$(CC) $(CFLAGS) -c $(<) -o $(@)

plugins:
	cd plugins && make build

release:
	make build
	make plugins
	mkdir -p release/exbash/bin
	mkdir -p release/exbash/default
	mkdir -p release/exbash/plugins
	cp build/exbash release/exbash/bin
	cp default/* release/exbash/default
	cp plugins/build/* release/exbash/plugins
	cp README.md THIRD_PARTY_LICENSES release/exbash
	cd release && tar cfz exbash_$(VERSION).tar.gz exbash
	rm -rf release/exbash

#-------------------------------------------------------------------------------
# Test commands
#-------------------------------------------------------------------------------

test:
	cd tests; make test

debug:
	cd tests; make debug

#-------------------------------------------------------------------------------
# Code check commands
#-------------------------------------------------------------------------------

check:
	cppcheck --std=c++23 --enable=all -I$(SRC_DIR) --library=posix --suppress=missingIncludeSystem --suppress=useStlAlgorithm --check-level=exhaustive $(CXX_FILES)

count:
	cloc --by-file source/cxx
	cloc --by-file source/bash

#-------------------------------------------------------------------------------
# Other commands
#-------------------------------------------------------------------------------

clean:
	cd plugins && make clean
	cd tests && make clean
	rm -rf build release

cacheclean:
	rm -rf /tmp/exbash /dev/shm/exbash

purge:
	make clean
	make cacheclean

# vim: noexpandtab tabstop=4 shiftwidth=4
