# Welder Engine

Building on Windows:
- Install CMake and Node.js
- Run Windows cmd as admin for symlink to work and navigate to repo dir.
- `git submodule init`
- `git submodule update`
- `npm install`
  - Can ignore errors probably.
- `node index.js cmake`
  - Specify builder if not using the default. Ex: --builder="Visual Studio 16 2019"
- `.\Build\<target>\Welder.sln`
