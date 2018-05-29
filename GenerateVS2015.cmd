pushd %cd%

chdir /D CMakeBuild
cmake -G "Visual Studio 14 2015" ..

popd