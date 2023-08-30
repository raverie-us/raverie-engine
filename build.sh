# Temporary build script until we figure out what to do with node
rm -rf build
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release -GNinja ..
#cmake ..
cmake --build .