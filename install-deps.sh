#!/bin/sh

(
  git clone --recurse-submodules --depth 1 --branch v2.0.9 https://github.com/arvidn/libtorrent
  cd libtorrent
  cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_STANDARD=14 .
  cmake --build .
  cmake --install .
)

(
  git clone https://github.com/Tencent/rapidjson/
  cd rapidjson
  cmake .
  cmake --install .
)
