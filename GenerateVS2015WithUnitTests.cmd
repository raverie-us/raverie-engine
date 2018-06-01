pushd %cd%

chdir /D CMakeBuild
cmake -G "Visual Studio 14 2015" -Dgenerate_with_unit_tests=ON -DWindows_VS_2015=ON -DBits_32=ON ..

popd
