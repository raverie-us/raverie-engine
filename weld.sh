set -e
mkdir -p Build/Cache
docker run --rm --cap-add=SYS_ADMIN --user `id -u`:`id -g` -v"$HOME":"/home/user" -v"`pwd`/Build/Cache":'/cache/' -v`pwd`:`pwd` -w`pwd` welder "$@"
