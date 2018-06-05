pushd %cd%

chdir /D CMakeBuild
cmake -G "Visual Studio 14 2015" -DWindows_VS_2015=ON -DBits_32=ON ..

popd
