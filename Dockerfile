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
    gnupg2 \
    clang-format \
    clang-tidy

RUN mkdir -p /etc/apt/keyrings
RUN curl -fsSL https://deb.nodesource.com/gpgkey/nodesource-repo.gpg.key | gpg --dearmor -o /etc/apt/keyrings/nodesource.gpg
RUN echo "deb [signed-by=/etc/apt/keyrings/nodesource.gpg] https://deb.nodesource.com/node_18.x nodistro main" | tee /etc/apt/sources.list.d/nodesource.list

RUN apt-get update
RUN apt-get install -y nodejs
RUN npm install -g npm@latest

# This is needed for Chrome/puppeteer to run
RUN wget -q -O - https://dl-ssl.google.com/linux/linux_signing_key.pub | apt-key add - && \
    sh -c 'echo "deb [arch=amd64] http://dl.google.com/linux/chrome/deb/ stable main" >> /etc/apt/sources.list.d/google.list' && \
    apt-get update && \
    apt-get install -y --no-install-recommends \
    google-chrome-unstable fonts-ipafont-gothic fonts-wqy-zenhei fonts-thai-tlwg fonts-kacst fonts-freefont-ttf

ENV NODE_PATH="/node_modules"
COPY package*.json ./
RUN npm install --omit=optional --no-progress --no-audit --unsafe-perm --global

# We intentionally do not compile with Emscripten, however Emscripten made changes to libcxxabi
# that supports its own style of exception handling (which clang can utilize with -fwasm-exceptions
# At some point I imagine these will be upstreamed into LLVM and wasi-sdk
# https://github.com/WebAssembly/wasi-sdk/issues/334
COPY External/Emscripten/Repo/system/lib/libcxxabi/include/cxxabi.h /wasi-sysroot/include/c++/v1/cxxabi.h

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
