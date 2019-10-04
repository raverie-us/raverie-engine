set -e
mkdir -p Build/Cache
docker run --memory=8g --memory-swap=32g --oom-kill-disable --rm --user `id -u`:`id -g` -v"`pwd`/Build/Cache":'/cache/' -v`pwd`:`pwd` -w`pwd` welder "$@"
