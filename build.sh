#!/bin/sh

./install-deps.sh

./bootstrap
./configure "$@"
make
