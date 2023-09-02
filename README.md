This project is an experimental fork of [Welder Engine](https://github.com/WelderFoundation/WelderEngineRevamp).
It is not recommended for production use.

# History
Welder Engine is itself a fork of [Zero Engine](https://zero.digipen.edu). 

**Zero Engine** was initially a proprietary game engine from the DigiPen Institute of Technology.
The code for Zero Engine was made available under the MIT license.
After becoming open source, one of the original developers created the Welder Engine fork as a revamp of Zero Engine but with cross platform support (Linux, Windows, Emscripten).
The developers of both code-bases became inactive and both projects are essentially not being developed anymore.
The engine itself is well designed and works great for the most part. It is a shame that it is no longer being developed.

I've decided to create an experimental fork to see what I can create with it as a base.
The overall goal is to modernize the engine in some areas and to update/replace some systems in others.
Since Welder Engine and Zero Engine have diverged a bit, I had to decide on which one to fork.
Welder Engine has made quite a few changes over Zero Engine, reorganizing the code structure and changed the formatting of the code.
This made it very difficult to try to merge the changes back into Zero Engine. I've attempted to do this and gave about halfway because of the sheer amount of changes across probably hundreds of commits. The manual process introduced too many opportunities for errors. 
Zero Engine on the other hand made only conservative changes to the common base.
It was feasible to manually apply the changes from Zero Engine into Welder Engine to get the best of both worlds; Welder Engine as a cross platform engine and the improvements from Zero Engine.
The changes from Zero Engine that were applied to this fork can be tracked on the [zerocore](https://github.com/WelderUpdates/WelderEngineRevamp/tree/zerocore) branch. This branch will remain mostly unchanged and only possible future changes from Zero Core will be added there. In an effort to update the engine dependencies, a second branch [zerocore_updates](https://github.com/WelderUpdates/WelderEngineRevamp/tree/zerocore_updates) was created to track updates to the third party dependencies. This branch is based on the zerocore branch and will continuously be rebased on it if necessary. Finally, there's the [evolution](https://github.com/WelderUpdates/WelderEngineRevamp) branch where additional bug fixes and experimental changes are made. 

# What will be changed

## Folder Structure
I personally think that the repository structure of Zero Engine makes more sense as it offers a clear separation between different systems instead of all systems and libraries being placed in a single Libraries folder. The first change here will be to reorganize the code to more closely follow the structure of Zero Engine (with some minor changes).

## Build System Updates
The Welder Engine build system is based on CMake. However, it is currently invoked through JavaScript. 
The JavaScript script runs several processes:
- Automatic code formatting
- CMake project generation
- Building the project
- Packaging prebuilt resources
- Builds the project a second time (so platforms with a VFS will have the prebuilt content)
- Generates the documentation
- Finally pack everything up in a redistributable format.

This is a complex process and results in long build times of a distributable package.
This will be simplified and will be invoked from shell/bash scripts instead of JavaScript, doing away with JavaScript as part of the build process.

## Future and more radical changes
 - Introduce a low level cross platform RHI and adapt the high level renderer to work with it.
 - Integrate Detour (and possibly recast) for navigation.
 - Completely separate the game process from the editor process.
 - Improve or replace the GUI toolkit (this is still an area of active investigation)
 - Change the content system to allowing using the file system hierarchy for organization instead of just tags.
 - Introduce the concept of Data directories to replace content libraries.
 - Lots of usability improvements to the editor.
 - Support deploying games to Android and possibly iOS.
 - Make ZilchPlugins (Integrate with CMake instead of VS directly).
 - Investigate integrating another scripting language: While Zilch works great, it is not currently actively developed and will likely not receive any further improvements. Even if another scripting language is added, Zilch will still be kept around.
*Other changes will be announced in the future*

# Old Welder Engine readme
 
## Welder Engine

Building on Windows:
- Install CMake and Node.js
- Run the following in cmd:

```shell
git clone https://github.com/WelderUpdates/WelderEngineRevamp.git
cd WelderEngineRevamp
git submodule update --init --recursive
npm install

node index.js cmake --config=Release

.\Build\<target>\Welder.sln
```
