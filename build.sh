#!/bin/sh

./install-deps.sh

./bootstrap.sh
./configure "$@"
make
