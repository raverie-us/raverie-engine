set -e
docker load -i ./Build/Cache/raverie.tar || true
./dockerbuild.sh
docker save -o ./Build/Cache/raverie.tar raverie-engine
