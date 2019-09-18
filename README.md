# Welder Engine

Building on Windows:
- Install CMake and Node.js
- Run the following in cmd:

```shell
git clone https://github.com/WelderFoundation/WelderEngineRevamp.git
cd WelderEngineRevamp
git submodule init
git submodule update
npm install

# Specify builder if not using the default. Ex: --builder="Visual Studio 16 2019"
node index.js cmake

.\Build\<target>\Welder.sln
```
