pushd %~dp0

set folder_name="VS_2015_SDLSTLEmpty_Windows"
rd %folder_name% /s/q

mkdir %folder_name%

cd %folder_name%

cmake -G "Visual Studio 14 2015" -DWindows_VS_2015_SDLSTLEmpty=ON -DBits_32=ON ../..

popd
