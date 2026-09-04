#!/bin/sh

echo "ls"                  > /tmp/bash_history
echo "echo 'hello world'" >> /tmp/bash_history
echo "cd /tmp"            >> /tmp/bash_history
echo "git status"         >> /tmp/bash_history

#-------------------------------------------------------------------------------
# File picker
#-------------------------------------------------------------------------------

# Basic tests.
./test_main -l "" -r "" -o /tmp/plugins.out -m file -c misc/config.toml -i "jjkkq"
./test_main -h
./test_main -v

# Movement keys.
./test_main -l "" -r "" -o stdout -m file -c misc/config.toml -i "G0^F^Bhl- .q"

# Grep window.
./test_main -l "" -r "" -o stdout -m file -c misc/config.toml -i "/cxx^Mjk^M"
./test_main -l "" -r "" -o stdout -m file -c misc/config.toml -i "/test_m^H^M^M"
./test_main -l "" -r "" -o stdout -m file -c misc/config_empty.toml -i "/test_m^H^M^M"

# Preview window.
./test_main -l "" -r "" -o stdout -m file -c misc/config.toml -i "^Pjjkk^Pq"

# Insufficient arguments (cause error print).
./test_main -i "q" || true

# Unexisting output path (cause error print).
./test_main -l "" -r "" -o /tmp/unexisting_dir/plugins.out -m file -i "^M" || true

#-------------------------------------------------------------------------------
# Envval/history/PID picker
#-------------------------------------------------------------------------------

# Basic tests.
./test_main -l "" -r "" -o /tmp/plugins.out -m env  -c misc/config.toml -i "jjkkq"
./test_main -l "" -r "" -o /tmp/plugins.out -m hist -c misc/config.toml -i "jjkkq"
./test_main -l "" -r "" -o /tmp/plugins.out -m pid  -c misc/config.toml -i "jjkk ^M"

# Movement keys.
./test_main -l "" -r "" -o stdout -m env -c misc/config.toml -i "G0q"

# Grep window.
./test_main -l "" -r "" -o stdout -m env -c misc/config.toml -i "/cxx^Mjk^M"
./test_main -l "" -r "" -o stdout -m env -c misc/config.toml -i "/test_m^H^M^M"
./test_main -l "" -r "" -o stdout -m env -c misc/config_empty.toml -i "/test_m^H^M^M"

# Pewview window.
./test_main -l "" -r "" -o stdout -m env -c misc/config.toml -i "^Pjjkk^Pq"

# Insufficient arguments (cause error print).
./test_main -i "q" || true

# Unexisting output path (cause error print).
./test_main -l "" -r "" -o /tmp/unexisting_dir/plugins.out -m env -i "^M" || true

#-------------------------------------------------------------------------------
# Value picker
#-------------------------------------------------------------------------------

# Basic tests.
./test_main -l "" -r "" -o /tmp/plugins.out -m val -c misc/config.toml -i "jjkkq"

# Movement keys.
./test_main -l "" -r "" -o stdout -m val -c misc/config.toml -i "G0hl^F^B q"

# Grep window.
./test_main -l "" -r "" -o stdout -m val -c misc/config.toml -i "/cxx^Mjk^M"
./test_main -l "" -r "" -o stdout -m val -c misc/config.toml -i "/test_m^H^M^M"
./test_main -l "" -r "" -o stdout -m val -c misc/config_empty.toml -i "/test_m^H^M^M"

# Pewview window.
./test_main -l "" -r "" -o stdout -m val -c misc/config.toml -i "^Pjjkk^Pq"

# Insufficient arguments (cause error print).
./test_main -i "q" || true

# Unexisting output path (cause error print).
./test_main -l "" -r "" -o /tmp/unexisting_dir/plugins.out -m val -i "^M" || true

# vim: expandtab tabstop=4 shiftwidth=4 fdm=marker
