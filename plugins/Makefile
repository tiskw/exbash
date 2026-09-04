################################################################################
# Makefile for plugins
################################################################################

.PHONY: help build clean

#-------------------------------------------------------------------------------
# Constants
#-------------------------------------------------------------------------------

targets := $(shell ls -d */ | sed 's|/||g' | grep -v build | sed 's/^/build\//')
sources := $(shell ls -d */ | sed 's|/||g' | grep -v build )

#-------------------------------------------------------------------------------
# Help message
#-------------------------------------------------------------------------------

help:
	@echo "Usage:"
	@echo "    make <command>"
	@echo ""
	@echo "Build commands:"
	@echo "    build    Build all plugins"
	@echo ""
	@echo "Other commands:"
	@echo "    clean    Cleanup build files"
	@echo "    help     Show this message"

#-------------------------------------------------------------------------------
# Build commands
#-------------------------------------------------------------------------------

build: $(targets)
	@echo "Build completed!"

build/%:
	mkdir -p build
	cd $(notdir $@) && make build && mv $(notdir $@) ../build/

#-------------------------------------------------------------------------------
# Clean command
#-------------------------------------------------------------------------------

clean:
	$(foreach srcdir,$(sources),cd $(srcdir) && make clean && cd ..;)
	rm -rf build

# vim: noexpandtab tabstop=4 shiftwidth=4
