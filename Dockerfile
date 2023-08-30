FROM ghcr.io/webassembly/wasi-sdk:sha-e85250b

RUN apt-get update
RUN apt-get install -y --no-install-recommends \
    wget \
    curl \
    p7zip-full \
    ccache \
    doxygen \
    dumb-init \
    git \
    gnupg2

RUN curl -sL https://deb.nodesource.com/setup_20.x | bash -
RUN apt-get install -y nodejs
RUN npm install -g npm@latest

ENV NODE_PATH="/node_modules"
COPY package*.json ./
RUN npm install --omit=optional --no-progress --no-audit --unsafe-perm --global

ARG USER_ID
RUN useradd -m -s /bin/bash -u $USER_ID user
USER $USER_ID
ENV HOME="/home/user"

ENV CCACHE_COMPRESS=1
ENV CCACHE_COMPRESSLEVEL=9
ENV CCACHE_DIR=/cache/
ENV CCACHE_SLOPPINESS=pch_defines,time_macros

ENTRYPOINT ["dumb-init", "--"]

CMD echo '----------ccache:' && \
    ccache --version && \
    echo '----------cmake:' && \
    cmake --version && \
    echo '----------doxygen:' && \
    doxygen --version && \
    echo '----------git:' && \
    git --version && \
    echo '----------ninja:' && \
    ninja --version && \
    echo '----------node:' && \
    node --version && \
    echo '----------npm:' && \
    npm --version
