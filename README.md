# About
The Raverie Engine is a light-weight game engine that aims to recreate the Macromedia/Adobe Flash experience of old.

# Getting started
Checkout the repository and all submodules:

```bash
git clone --recurse-submodules https://github.com/raverie-us/raverie-engine.git
```

Or if you already checked the repo out without cloning submodules, be sure to run:
```bash
git submodule update --init --recursive
```

# Docker Setup
To build Raverie in a consistent build environment, we use Docker containers.

Windows only, install the WSL and run the rest of the commands inside the WSL/bash:
```bash
wsl --install
```

To install Docker:
```bash
sudo snap install docker
```

We require docker to be usable without sudo (https://docs.docker.com/engine/install/linux-postinstall/):
```bash
sudo groupadd docker
sudo usermod -aG docker $USER
newgrp docker
```

# Building
Whilst the Raverie Engine can run on any platform, it is currently only built on Linux and requires Docker, but may work on other *nix platforms.

Start by building the docker image:

```bash
# Only need to run this once, or any time the Dockerfile changes
./dockerbuild.sh
```

The script `run.sh` runs any command inside Docker in the root of the repo.

To build, we use CMake, however we run through a script that handles bundling in pre-built content in the final build:

```bash
# The 'all' command will run cmake, build the project,
# pre-build all content, and then package that into a final build
./run.sh npm start -- all --config=Release
```

To run just the build manually (without prebuilt content):

```bash
# Clean and generate a new cmake, only do this once
./run.sh npm start -- cmake --config=Release

# Run this after any change to build a new WASM file
./run.sh npm start -- build --config=Release
```

# Testing

To run a newly created build, the easiest way is to run in a browser:
```bash
./run.sh npm run browser-dev
```

This will print out a link that you can visit in the browser which will run the engine, for example http://172.17.0.*:8080/

Note that the `localhost` link will not work as it's only accessible from inside the Docker container.

# Architecture
The Raverie Engine is built to run on any platform and targets pure WASM as its only output. This means that the output does not include executables, glue code, etc (we do not use Emscripten). The list of imports and exports is cleanly defined in `PlatformCommunication.hpp` and can easily be implemented by any platform, including the browser.
