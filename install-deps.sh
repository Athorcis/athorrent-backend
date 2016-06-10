#!/bin/sh

git clone -b RC_1_1 https://github.com/arvidn/libtorrent
cd libtorrent
./bootstrap.sh --prefix=/usr CPPFLAGS="-DTORRENT_EXPORT_EXTRA" CXXFLAGS="-std=c++11"
make
sudo make install
cd ..

wget https://github.com/miloyip/rapidjson/archive/v1.0.2.tar.gz -O rapidjson-1.0.2.tar.gz
tar -xzvf rapidjson-1.0.2.tar.gz
cd rapidjson-1.0.2
cmake .
sudo make install
cd ..
