FROM ubuntu:18.04

RUN apt-get update && \
    apt-get install -y --no-install-recommends \
    wget \
    git-core \
    default-jre \
    python2.7 \
    p7zip-full \
    ccache \
    doxygen \
    dumb-init \
    clang \
    clang-format \
    clang-tidy \
    git \
    gnupg2 \
    iwyu \
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
    mesa-common-dev mesa-utils-extra libglapi-mesa libgl1-mesa-dev libglu1-mesa-dev freeglut3 freeglut3-dev \
    llvm \
    ninja-build \
    nodejs \
    npm \
    pkg-config \
    xorg-dev \
    xscreensaver \
    xutils-dev \
    xvfb

RUN wget -q -O - https://dl-ssl.google.com/linux/linux_signing_key.pub | apt-key add - && \
    sh -c 'echo "deb [arch=amd64] http://dl.google.com/linux/chrome/deb/ stable main" >> /etc/apt/sources.list.d/google.list' && \
    apt-get update && \
    apt-get install -y --no-install-recommends \
    google-chrome-unstable fonts-ipafont-gothic fonts-wqy-zenhei fonts-thai-tlwg fonts-kacst fonts-freefont-ttf

RUN apt-get clean && \
    rm -rf /var/lib/apt/lists/*

RUN wget -q -O cmake.sh https://github.com/Kitware/CMake/releases/download/v3.14.7/cmake-3.14.7-Linux-x86_64.sh && \
    chmod +x cmake.sh && \
    ./cmake.sh --skip-license && \
    rm cmake.sh

ENV DISPLAY=":99.0"

RUN npm install -g npm@latest

ENV NODE_PATH="/node_modules"
COPY package*.json ./
RUN npm install --no-optional --no-progress --no-audit --unsafe-perm --global

# There is a problem with clang using the default gcc headers on Ubuntu (remove #error).
RUN sed -i 's/# error.*//g' /usr/include/x86_64-linux-gnu/sys/cdefs.h

RUN echo 'pcm.!default { type plug slave.pcm "null" }' > /etc/asound.conf

ARG USER_ID
RUN useradd -m -s /bin/bash -u $USER_ID user && \
    addgroup user audio && \
    addgroup user video
USER $USER_ID
ENV HOME="/home/user"

ENV EMSCRIPTEN_VERSION sdk-tag-1.38.47-64bit-upstream

RUN cd /tmp && \
    git clone https://github.com/juj/emsdk.git && \
    cd emsdk && \
    ./emsdk update-tags && \
    ./emsdk install $EMSCRIPTEN_VERSION && \
    ./emsdk activate --embedded $EMSCRIPTEN_VERSION

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
    printf "#include <iostream>\nint main(){ std::cout << 0; }" > build.cpp && \
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
    echo '----------iwyu:' && \
    iwyu --version && \
    echo '----------ninja:' && \
    ninja --version && \
    echo '----------node:' && \
    node --version && \
    echo '----------npm:' && \
    npm --version && \
    echo '----------emcc:' && \
    emcc --version
