FROM ubuntu:22.04 

RUN apt-get update
RUN apt-get install -y --no-install-recommends \
    wget \
    git-core \
    default-jre \
    python2.7 \
    p7zip-full \
    ccache \
    cmake \
    doxygen \
    dumb-init \
    clang \
    clang-format \
    clang-tidy \
    git \
    gnupg2 \
    build-essential \
    libasound2-dev \
    libpulse-dev \
    libudev-dev \
    libopenal-dev \
    libogg-dev \
    libvorbis-dev \
    libaudiofile-dev \
    libpng-dev \
    libfreetype6-dev \
    libusb-dev \
    libdbus-1-dev \
    zlib1g-dev \
    libdirectfb-dev \
    mesa-common-dev \
    mesa-utils-extra \
    libglapi-mesa \
    libgl1-mesa-dev \
    libglu1-mesa-dev \
    freeglut3 \
    freeglut3-dev \
    llvm \
    ninja-build \
    nodejs \
    npm \
    pkg-config \
    python3 \
    xorg-dev \
    xscreensaver \
    xutils-dev \
    xvfb \
    libatk-bridge2.0-0 \
    libgtk-3.0

RUN npm install -g npm@latest

ENV NODE_PATH="/node_modules"
COPY package*.json ./
RUN npm install --no-optional --no-progress --no-audit --unsafe-perm --global

# There is a problem with clang using the default gcc headers on Ubuntu (remove #error).
RUN sed -i 's/# error.*//g' /usr/include/x86_64-linux-gnu/sys/cdefs.h

RUN echo 'pcm.!default { type plug slave.pcm "null" }' > /etc/asound.conf

RUN ln /usr/bin/python3 /usr/bin/python

ARG USER_ID
RUN useradd -m -s /bin/bash -u $USER_ID user && \
    addgroup user audio && \
    addgroup user video
USER $USER_ID
ENV HOME="/home/user"

ENV EMSCRIPTEN_VERSION 3.1.17

RUN cd /tmp && \
    git clone https://github.com/juj/emsdk.git && \
    cd emsdk && \
    ./emsdk install $EMSCRIPTEN_VERSION && \
    ./emsdk activate --embedded $EMSCRIPTEN_VERSION

RUN ls /tmp/emsdk/node/

ENV PATH="/tmp/emsdk:/tmp/emsdk/upstream/emscripten:/tmp/emsdk/node/12.9.1_64bit/bin:${PATH}"
ENV EMSDK="/tmp/emsdk"
ENV EM_CONFIG="/tmp/emsdk/.emscripten"
ENV EM_PORTS="/tmp/emsdk/.emscripten_ports"
ENV EM_CACHE="/tmp/emsdk/.emscripten_cache"
ENV EMSDK_NODE="/tmp/emsdk/node/12.9.1_64bit/bin/node"
ENV EMSCRIPTEN="/tmp/emsdk/upstream/emscripten"
ENV EMCC_WASM_BACKEND=1
ENV EMCC_SKIP_SANITY_CHECK=1

# Compile a program to force emcc caching
RUN mkdir -p /tmp/emcc && \
    cd /tmp/emcc && \
    printf "#include <iostream>\nint main(){ std::cout << 0; malloc(0); }" > build.cpp && \
    emcc -s USE_WEBGL2=1 -s FULL_ES2=1 -s FULL_ES3=1 -s USE_SDL=2 -O2 -s WASM=1 build.cpp && \
    rm -rf /tmp/emcc

ENV CCACHE_COMPRESS=1
ENV CCACHE_COMPRESSLEVEL=9
ENV CCACHE_DIR=/cache/
ENV CCACHE_SLOPPINESS=pch_defines,time_macros

ENTRYPOINT ["dumb-init", "--", "xvfb-run"]

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
    echo '----------ninja:' && \
    ninja --version && \
    echo '----------node:' && \
    node --version && \
    echo '----------npm:' && \
    npm --version && \
    echo '----------emcc:' && \
    emcc --version
