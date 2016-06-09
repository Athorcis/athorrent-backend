#!/bin/bash

wget https://github.com/arvidn/libtorrent/releases/download/libtorrent-1_1/libtorrent-rasterbar-1.1.0.tar.gz
tar -xzvf libtorrent-rasterbar-1.1.0.tar.gz
pushd libtorrent-rasterbar-1.1.0
./configure CPPFLAGS="-DTORRENT_EXPORT_EXTRA" CXXFLAGS="-std=c++11"
make
sudo make install
popd

wget https://github.com/miloyip/rapidjson/archive/v1.0.2.tar.gz -O rapidjson-1.0.2.tar.gz
tar -xzvf rapidjson-1.0.2.tar.gz
pushd rapidjson-1.0.2
cmake .
sudo make install
popd
