FROM ubuntu:18.10

RUN apt-get update && apt-get install -y \
    cmake \
    doxygen \
    dumb-init \
    clang \
    clang-tidy \
    clang-format \
    git \
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

WORKDIR /Welder/
COPY package*.json ./
RUN npm install --no-optional --no-progress --no-audit

RUN chown -R 1000 /Welder/

# There is a problem with clang using the default gcc headers on Ubuntu (remove #error).
RUN sed -i 's/# error.*//g' /usr/include/x86_64-linux-gnu/sys/cdefs.h

USER 1000

ENTRYPOINT ["dumb-init", "--", "node", "index.js"]

#CMD cmake --version && \
#    doxygen --version && \
#    clang --version && \
#    clang-format --version && \
#    clang-tidy --version && \
#    ninja --version && \
#    node --version && \
#    npm --version
