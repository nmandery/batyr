#!/ubin/bash
# Script to build a deb of the application in a docker container

set -eu

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
DOCKER_BIN=docker.io
#DOCKER_IMG=debian:wheezy
DOCKER_IMG="$1"
#GIT_BRANCH=debian-wheezy
GIT_BRANCH="$2"

PACKAGE_DIR="$SCRIPT_DIR/../packages"
mkdir -p "$PACKAGE_DIR"
cat >"$PACKAGE_DIR/build-$GIT_BRANCH-deb.sh" <<EOF
set -eux

apt-get update
apt-get -y install libpoco-dev libpocofoundation9 libpoconet9 libpocoutil9 libgdal1 \
    libgdal1-dev cmake g++ build-essential libpq-dev discount python \
    libpocofoundation9-dbg libpoconet9-dbg libpocoutil9-dbg git cmake \
    debhelper

git clone -b $GIT_BRANCH /root/src /root/src-cloned
cd /root/src-cloned

make -f Makefile.devel deb
cd ..
cp *.deb /root/src/packages/
EOF
trap "rm -f $PACKAGE_DIR/build-$GIT_BRANCH-deb.sh" EXIT

sudo $DOCKER_BIN run -v "$SCRIPT_DIR/..":/root/src "$DOCKER_IMG" bash "/root/src/packages/build-$GIT_BRANCH-deb.sh"

