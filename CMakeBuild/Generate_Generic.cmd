pushd %~dp0

set folder_name="Generic"
rd %folder_name% /s/q

mkdir %folder_name%

cd %folder_name%

cmake ../..

popd
