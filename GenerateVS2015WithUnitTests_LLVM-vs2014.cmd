pushd %cd%

chdir /D CMakeBuild
cmake -G "Visual Studio 14 2015" -Dgenerate_with_unit_tests=ON -DVS_LLVM_2014=ON -DBits_32=ON -TLLVM-vs2014 ..

popd
