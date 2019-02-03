pushd %~dp0

set folder_name="VS_MSVC_Windows"
rd %folder_name% /s/q

mkdir %folder_name%

cd %folder_name%

cmake -DWELDER_TOOLCHAIN=MSVC -DWELDER_PLATFORM=Windows ../..

popd
