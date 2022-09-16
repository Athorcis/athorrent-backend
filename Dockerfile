FROM debian:11.2-slim AS build

RUN set -ex ;\
    apt-get update ;\
    apt-get install -y --no-install-recommends \
      autoconf \
      automake \
      ca-certificates \
      git \
      g++ \
      libboost-chrono-dev \
      libboost-random-dev \
      libboost-system-dev \
      libssl-dev \
      libtool \
      make \
      pkg-config

RUN set -ex ;\
    git clone --depth 1 --branch v1.2.15 https://github.com/arvidn/libtorrent ;\
    cd libtorrent ;\
    ./autotool.sh ;\
    ./configure ;\
    make -j $(nproc) ;\
    make install

RUN set -ex ;\
    apt-get install -y --no-install-recommends \
      cmake ;\
    git clone https://github.com/Tencent/rapidjson/ ;\
    cd rapidjson ;\
    cmake . ;\
    make install ;\
    apt-get remove --purge -y \
      cmake

RUN set -ex ;\
    apt-get install -y --no-install-recommends \
      libboost-filesystem-dev \
      libboost-program-options-dev \
      libboost-thread-dev

COPY . dist

RUN set -ex ;\
    cd dist ;\
    autoreconf -i ;\
    ./configure CXXFLAGS="-std=c++14" ;\
    make -j $(nproc)

FROM debian:11.2-slim

RUN set -ex ;\
    apt-get update ;\
    apt-get install -y --no-install-recommends \
        libboost-chrono1.74.0 \
        libboost-filesystem1.74.0 \
        libboost-program-options1.74.0 \
        libboost-random1.74.0 \
        libboost-system1.74.0 \
        libboost-thread1.74.0 \
        socat ;\
    rm -rf /var/lib/apt/lists/*

ENV LD_LIBRARY_PATH=/usr/local/lib

COPY --from=build /dist/src/athorrent-backend /usr/local/bin/athorrent-backend
COPY --from=build /usr/local/lib/libtorrent-rasterbar.so.10 /usr/local/lib/libtorrent-rasterbar.so.10

ENTRYPOINT ["/usr/local/bin/athorrent-backend"]

CMD ["--port", "6881"]

HEALTHCHECK --interval=5s --timeout=1s \
    CMD echo '{"action":"ping","parameters":{}}' | socat - UNIX-CONNECT:/var/lib/athorrent-backend/athorrentd.sck | grep -q '{"status":"success","data":"pong"}'