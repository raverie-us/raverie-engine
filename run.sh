set -e
mkdir -p Build/Cache

# We could be running this inside a submodule, and unfortunately submodules when mounted are not
# recognized as a git repo because their git information is actually contained inside the parent .git file
# We must mount either this directory or the submodule parent/root directory
if [[ $(git rev-parse --show-superproject-working-tree) ]]; then
  DIR=$(git rev-parse --show-superproject-working-tree)
else
  DIR=$(pwd)
fi

docker run --rm --cap-add=SYS_ADMIN --user `id -u`:`id -g` -v"$HOME":"/home/user" -v"`pwd`/Build/Cache":'/cache/' -v$DIR:$DIR -w`pwd` raverie-engine "$@"
