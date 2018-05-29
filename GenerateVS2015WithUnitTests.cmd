
pushd %cd%

chdir /D CMakeBuild
cmake -G "Visual Studio 14 2015" -Dgenerate_with_unit_tests=ON ..

popd