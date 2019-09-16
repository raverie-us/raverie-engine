mkdir -p Build/Cache
docker run --rm -v"`pwd`/Build/Cache":'/cache/' -v`pwd`:`pwd` -w`pwd` welder "$@"
