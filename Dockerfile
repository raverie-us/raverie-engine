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
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

RUN npm install -g npm@latest

#WORKDIR /home/cmake/
#RUN wget -q -Otemp.tar.gz "https://cmake.org/files/v3.13/cmake-3.13.4-Linux-x86_64.tar.gz"
#RUN tar -xf temp.tar.gz --strip-components=1
#RUN rm temp.tar.gz
#ENV PATH=/home/cmake/bin:$PATH
#
#WORKDIR /home/doxygen
#RUN wget -q -Otemp.tar.gz "http://doxygen.nl/files/doxygen-1.8.15.linux.bin.tar.gz"
#RUN tar -xf temp.tar.gz --strip-components=1
#RUN rm temp.tar.gz
#ENV PATH=/home/doxygen/bin:$PATH
#
#WORKDIR /home/llvm/
#RUN wget -q -Otemp.tar.xz "http://releases.llvm.org/7.0.0/clang+llvm-7.0.0-x86_64-linux-gnu-ubuntu-16.04.tar.xz"
#RUN tar -xf temp.tar.xz --strip-components=1
#RUN rm temp.tar.xz
#ENV PATH=/home/llvm/bin:$PATH
#
#WORKDIR /home/ninja/bin
#RUN wget -q -Otemp.zip "https://github.com/ninja-build/ninja/releases/download/v1.8.2/ninja-linux.zip"
#RUN unzip temp.zip
#RUN rm temp.zip
#ENV PATH=/home/ninja/bin:$PATH

WORKDIR /home
COPY package*.json ./
RUN npm install --no-optional --no-progress --no-audit

RUN chown -R 1000 /home/

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
