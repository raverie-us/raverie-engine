FROM ubuntu:18.10

RUN apt-get update && apt-get install -y --no-install-recommends \
    ccache \
    cmake \
    doxygen \
    dumb-init \
    clang \
    clang-format \
    clang-tidy \
    git \
    iwyu \
    mesa-common-dev mesa-utils-extra libglapi-mesa libgl1-mesa-dev libglu1-mesa-dev freeglut3 freeglut3-dev \
    llvm \
    ninja-build \
    nodejs \
    npm \
    pkg-config \
    xorg-dev \
    xscreensaver \
    xutils-dev \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

RUN npm install -g npm@latest

COPY package*.json ./
RUN npm install --no-optional --no-progress --no-audit --global

# There is a problem with clang using the default gcc headers on Ubuntu (remove #error).
RUN sed -i 's/# error.*//g' /usr/include/x86_64-linux-gnu/sys/cdefs.h

USER 1000

ENV CCACHE_COMPRESS=1
ENV CCACHE_COMPRESSLEVEL=9
ENV CCACHE_DIR=/cache/

ENTRYPOINT ["dumb-init", "--"]

CMD echo '----------ccache:' && \
    ccache --version && \
    echo '----------cmake:' && \
    cmake --version && \
    echo '----------doxygen:' && \
    doxygen --version && \
    echo '----------clang:' && \
    clang --version && \
    echo '----------clang-format:' && \
    clang-format --version && \
    echo '----------clang-tidy:' && \
    clang-tidy --version && \
    echo '----------git:' && \
    git --version && \
    echo '----------iwyu:' && \
    iwyu --version && \
    echo '----------ninja:' && \
    ninja --version && \
    echo '----------node:' && \
    node --version && \
    echo '----------npm:' && \
    npm --version
