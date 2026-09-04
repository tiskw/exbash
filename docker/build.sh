#!/bin/sh

if [ ! -e "cxxopts.hpp" ]; then
    wget -O cxxopts-3.3.1.zip https://github.com/jarro2783/cxxopts/archive/refs/tags/v3.3.1.zip
    unzip -j cxxopts-3.3.1.zip cxxopts-3.3.1/include/cxxopts.hpp
    rm -f cxxopts-3.3.1.zip
fi

if [ ! -e "json.hpp" ]; then
    wget https://github.com/nlohmann/json/releases/download/v3.12.0/json.hpp
fi

if [ ! -e "sol.hpp" ]; then
    wget https://github.com/ThePhD/sol2/releases/download/v3.3.0/sol.hpp
fi

if [ ! -e "toml.hpp" ]; then
    wget -O tomlplusplus-3.4.0.zip https://github.com/marzer/tomlplusplus/archive/refs/tags/v3.4.0.zip
    unzip -j tomlplusplus-3.4.0.zip tomlplusplus-3.4.0/toml.hpp
    rm -f tomlplusplus-3.4.0.zip
fi

if [ ! -e "lua-5.5.0.tar.gz" ]; then
    wget https://www.lua.org/ftp/lua-5.5.0.tar.gz
fi

if [ ! -e "carapace-bin_1.7.3_linux_amd64.apk" ]; then
    wget https://github.com/carapace-sh/carapace-bin/releases/download/v1.7.3/carapace-bin_1.7.3_linux_amd64.apk
fi

docker build . -t "tiskw/exbash:alpine3.23"

# vim: expandtab tabstop=4 shiftwidth=4 fdm=marker
