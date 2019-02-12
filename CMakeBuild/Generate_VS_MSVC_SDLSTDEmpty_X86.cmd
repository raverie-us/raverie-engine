pushd %~dp0

set folder_name="VS_MSVC_SDLSTDEmpty_X86"
rd %folder_name% /s/q

mkdir %folder_name%

cd %folder_name%

cmake -DWELDER_TOOLCHAIN=MSVC -DWELDER_PLATFORM=SDLSTDEmpty -DPLATFORM_ARCHITECTURE=X86 ../..

popd
