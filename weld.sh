mkdir -p Cache
docker run --rm -v"`pwd`/Cache":'/cache/' -v`pwd`:`pwd` -w`pwd` welder "$@"
