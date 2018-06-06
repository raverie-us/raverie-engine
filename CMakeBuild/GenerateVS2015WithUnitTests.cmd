pushd %cd%

set folder_name="VS2015_MSVC_Windows"
rd %folder_name% /s/q

mkdir %folder_name%

cd %folder_name%

cmake -G "Visual Studio 14 2015" -Dgenerate_with_unit_tests=ON -DWindows_VS_2015=ON -DBits_32=ON ../..

popd
