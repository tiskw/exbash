################################################################################
# Makefile - Test for C/C++ code
################################################################################

.PHONY: help test debug clean

#-------------------------------------------------------------------------------
# Constants
#-------------------------------------------------------------------------------

# C/C++ source directory.
ORG_DIR := ../source
SRC_DIR := ./source

# Target C++ files.
CXX_TEST_MAIN     := $(shell find source | grep cxx)
CXX_TEST_MODEL    := $(shell find source | grep cxx | grep -v main)
CXX_TEST_STREXTRA := $(shell find source | grep cxx | grep -v main)
CXX_TEST_UTILS    := $(shell find source | grep cxx | grep -v main)

# Compile command for tests.
CC      := g++ -std=c++23
CFLAGS  := -O0 -g -Wall -Wextra -I/usr/local/include -I$(SRC_DIR)
LIBS    := -L/usr/local/lib -lncurses
COVOPTS := -fPIC -coverage -fprofile-update=atomic -DGCOV_BUILD

# Commands for Docker-based static build.
DOCKER_IMAGE := tiskw/exbash:alpine3.23
DOCKER_BASE  := run --rm -it -u `id -u`:`id -g` -v `pwd`:/work -w /work
DOCKER_ARGS  := $(DOCKER_BASE) $(DOCKER_IMAGE) sh -c

#-------------------------------------------------------------------------------
# Help message
#-------------------------------------------------------------------------------

help:
	@echo "Usage:"
	@echo "    make <command>"
	@echo ""
	@echo "Test commands:"
	@echo "    test    Run tests for C/C++ files and measure code coverage"
	@echo "    debug   Compile tests with debug flags and run on GDB"
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
	docker $(DOCKER_ARGS) 'make -j4 test_inside_docker'

test_inside_docker:
	# make test_main test_model test_strextra test_utils
	make test_main test_strextra test_utils
	sh test_main.sh
	# ./test_model
	./test_strextra
	./test_utils
	lcov --quiet --ignore-errors corrupt,gcov,gcov,inconsistent,range --gcov-tool gcov -c -d . -o lcov.info
	lcov --quiet --ignore-errors corrupt,inconsistent --remove lcov.info '/usr/include/*' --output-file lcov_filtered.info
	genhtml --quiet --ignore-errors inconsistent,corrupt lcov_filtered.info -o ./lcov

test_main:
	$(CC) $(CFLAGS) $(COVOPTS) -o $(@) $(CXX_TEST_MAIN) $(LIBS)

test_model:
	$(CC) $(CFLAGS) $(COVOPTS) -o $(@) $(@).cxx $(CXX_TEST_MODEL) $(LIBS)

test_strextra:
	$(CC) $(CFLAGS) $(COVOPTS) -o $(@) $(@).cxx $(CXX_TEST_STREXTRA) $(LIBS)

test_utils:
	$(CC) $(CFLAGS) $(COVOPTS) -o $(@) $(@).cxx $(CXX_TEST_UTILS) $(LIBS)

#-------------------------------------------------------------------------------
# Other commands
#-------------------------------------------------------------------------------

clean:
	rm -f *.gcda *.gcno *.gcov lcov.info test_main test_model test_strextra test_utils
	rm -fr lcov source

# vim: noexpandtab shiftwidth=4 tabstop=4 fdm=marker
