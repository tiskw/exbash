################################################################################
# Makefile - Test for C/C++ code
################################################################################

.PHONY: help test debug clean

#-------------------------------------------------------------------------------
# Constants
#-------------------------------------------------------------------------------

# C/C++ source directory.
ORG_DIR := ../source/cxx
SRC_DIR := ./source

# C/C++ source files (excluding header files and main.cxx).
CXX_FILES := $(shell find $(SRC_DIR) -type f | grep -v hxx | grep -v main.cxx | sort)

#-------------------------------------------------------------------------------
# C++ build settings
#-------------------------------------------------------------------------------

# Compile command.
CC      := g++ -std=c++23
CFLAGS  := -O0 -g -Wall -Wextra -I/usr/local/include -I$(SRC_DIR)
LIBS    := -L/usr/local/lib -lncurses
COVOPTS := -fPIC -coverage -fprofile-update=atomic -DGCOV_BUILD

# Commands for Docker-based static build.
DOCKER_IMAGE := tiskw/exbash:alpine3.23
DOCKER_BASE  := run --rm -it -u `id -u`:`id -g` -v `pwd`:/work -w /work
DOCKER_ARGS  := $(DOCKER_BASE) $(DOCKER_IMAGE) sh -c 'make -j4 -i test_inside_docker'

#-------------------------------------------------------------------------------
# Help message
#-------------------------------------------------------------------------------

help:
	@echo "Usage:"
	@echo "    make <command>"
	@echo ""
	@echo "Test commands:"
	@echo "    test    Run tests for C/C++ files and measure code coverage"
	@echo ""
	@echo "Other commands:"
	@echo "    clean   Cleanup cache files"
	@echo "    help    Show this message"

#-------------------------------------------------------------------------------
# Test commands
#-------------------------------------------------------------------------------

test:
	make clean
	cp -r $(ORG_DIR) $(SRC_DIR)
	git init
	@docker $(DOCKER_ARGS)
	rm -rf .git

test_inside_docker:
	make run_test
	mkdir -p /dev/shm/exbash
	./run_test --run print_path_cmds > /dev/shm/exbash/cmnd_info.txt
	echo "ls -l" > /tmp/bash_history
	./run_test
	./run_test --config 
	lcov --quiet --gcov-tool gcov --ignore-errors gcov,gcov,inconsistent,range,corrupt -c -d . -o lcov.info
	genhtml --quiet --ignore-errors gcov,gcov,inconsistent,range,corrupt lcov.info -o ./lcov

run_test: test_main.cxx $(CXX_FILES)
	$(CC) $(CFLAGS) $(COVOPTS) -o run_test -I$(SRC_DIR) test_main.cxx $(CXX_FILES)

#-------------------------------------------------------------------------------
# Other commands
#-------------------------------------------------------------------------------

clean:
	rm -f *.gcda *.gcno *.gcov lcov.info run_test
	rm -rf lcov source

# vim: noexpandtab shiftwidth=4 tabstop=4 fdm=marker
