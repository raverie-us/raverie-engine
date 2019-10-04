set -e
mkdir -p Build/Cache
docker run --user `id -u`:`id -g` -v"`pwd`/Build/Cache":'/cache/' -v`pwd`:`pwd` -w`pwd` welder "$@"
