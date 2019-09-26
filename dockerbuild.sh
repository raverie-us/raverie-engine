set -e
docker build --cache-from welder:latest --build-arg USER_ID=`id -u` -t welder .
