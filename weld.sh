mkdir -p Build/Cache
docker run --rm --user `id -u`:`id -g` -v"`pwd`/Build/Cache":'/cache/' -v`pwd`:`pwd` -w`pwd` welderengine/build "$@"
