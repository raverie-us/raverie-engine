pushd %cd%

set folder_name="VS2015_LLVM-vs2014_Windows"
rd %folder_name% /s/q

mkdir %folder_name%

cd %folder_name%

cmake -G "Visual Studio 14 2015" -Dgenerate_with_unit_tests=ON -DVS_LLVM_2014=ON -DBits_32=ON -TLLVM-vs2014 ../..

popd
