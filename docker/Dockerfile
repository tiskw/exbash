# Dockerfile

FROM alpine:3.23

COPY cxxopts.hpp /usr/local/include/cxxopts.hpp
COPY json.hpp    /usr/local/include/json.hpp
COPY sol.hpp     /usr/local/include/sol.hpp
COPY toml.hpp    /usr/local/include/toml.hpp

COPY lua-5.5.0.tar.gz /root/lua-5.5.0.tar.gz
COPY carapace-bin_1.7.3_linux_amd64.apk /root/carapace-bin_1.7.3_linux_amd64.apk

RUN apk update \
 && apk add --no-cache build-base gzip lcov ncurses-dev ncurses-static git bash bash-completion shared-mime-info \
 && apk add --no-cache --allow-untrusted /root/carapace-bin_1.7.3_linux_amd64.apk \
 && rm -rf /var/cache/apk/*

RUN cd /root \
 && tar -xzf lua-5.5.0.tar.gz \
 && cd lua-5.5.0 \
 && make linux test \
 && make install \
 && rm -rf /root/lua-5.5.0 /root/lua-5.5.0.tar.gz

