#!/bin/sh

git clone -b libtorrent-1_2_6 https://github.com/arvidn/libtorrent
cd libtorrent
./bootstrap.sh --prefix=/usr CXXFLAGS="-std=c++11"
make
sudo make install
cd ..

wget https://github.com/Tencent/rapidjson/archive/v1.1.0.tar.gz -O rapidjson-1.1.0.tar.gz
tar -xzvf rapidjson-1.1.0.tar.gz
cd rapidjson-1.1.0
cmake .
sudo make install
cd ..
