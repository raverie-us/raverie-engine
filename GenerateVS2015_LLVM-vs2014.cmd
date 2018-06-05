pushd %cd%

chdir /D CMakeBuild
cmake -G "Visual Studio 14 2015" -DVS_LLVM_2014=ON -DBits_32=ON -TLLVM-vs2014 ..

popd
