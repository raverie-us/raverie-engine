mkdir -p Cache
docker run -v"`pwd`/Cache":'/cache/' -v`pwd`:`pwd` -w`pwd` welder/linux "$@"
