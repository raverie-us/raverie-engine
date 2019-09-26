set -e
docker load -i ./Build/Cache/welder.tar || true
./dockerbuild.sh
./weld.sh node index.js cmake --alias=$ALIAS
./weld.sh node index.js build --alias=$ALIAS
docker save -o ./Build/Cache/welder.tar welder
