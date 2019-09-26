set -e
docker load -i ./Build/Cache/welder.tar || true
./dockerbuild.sh
docker save -o ./Build/Cache/welder.tar welder
./weld.sh node index.js cmake "$@"
./weld.sh node index.js build "$@"
