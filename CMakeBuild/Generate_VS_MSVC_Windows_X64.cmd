pushd %~dp0

set folder_name="VS_MSVC_Windows_X64"
rd %folder_name% /s/q

mkdir %folder_name%

cd %folder_name%

cmake -DWELDER_TOOLCHAIN=MSVC -DWELDER_PLATFORM=Windows -DCMAKE_GENERATOR_PLATFORM=x64 -T host=x64 -DPLATFORM_ARCHITECTURE=X64 ../..

popd
