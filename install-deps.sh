#!/bin/bash

git clone -b RC_1_1 https://github.com/arvidn/libtorrent
pushd libtorrent
./bootstrap.sh CPPFLAGS="-DTORRENT_EXPORT_EXTRA" CXXFLAGS="-std=c++11"
make
sudo make install
popd

wget https://github.com/miloyip/rapidjson/archive/v1.0.2.tar.gz -O rapidjson-1.0.2.tar.gz
tar -xzvf rapidjson-1.0.2.tar.gz
pushd rapidjson-1.0.2
cmake .
sudo make install
popd
