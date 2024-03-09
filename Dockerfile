FROM debian:12.5-slim AS build

RUN set -ex ;\
    apt-get update ;\
    apt-get install -y --no-install-recommends \
      autoconf \
      automake \
      ca-certificates \
      cmake \
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
    git clone --recurse-submodules --depth 1 --branch v2.0.10 https://github.com/arvidn/libtorrent ;\
    cd libtorrent ;\
    cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_STANDARD=14 . ;\
    cmake --build . -- -j $(nproc) ;\
    cmake --install .

RUN set -ex ;\
    git clone https://github.com/Tencent/rapidjson/ ;\
    cd rapidjson ;\
    cmake . ;\
    cmake --install .

RUN set -ex ;\
    apt-get install -y --no-install-recommends \
      libboost-filesystem-dev \
      libboost-program-options-dev \
      libboost-stacktrace-dev \
      libboost-thread-dev

COPY . dist

RUN set -ex ;\
    cd dist ;\
    autoreconf -i ;\
    ./configure --with-boost-stacktrace=boost_stacktrace_backtrace CXXFLAGS="-std=c++14" ;\
    make -j $(nproc)

FROM debian:12.5-slim

RUN set -ex ;\
    apt-get update ;\
    apt-get install -y --no-install-recommends \
        libboost-chrono1.74.0 \
        libboost-filesystem1.74.0 \
        libboost-program-options1.74.0 \
        libboost-random1.74.0 \
        libboost-stacktrace1.74.0 \
        libboost-system1.74.0 \
        libboost-thread1.74.0 \
        locales \
        socat ;\
    rm -rf /var/lib/apt/lists/* ;\
    sed -i '/en_US.UTF-8/s/^# //g' /etc/locale.gen ;\
    locale-gen

ENV LANG en_US.UTF-8
ENV LANGUAGE en_US:en
ENV LC_ALL en_US.UTF-8

ENV LD_LIBRARY_PATH=/usr/local/lib

COPY --from=build /dist/src/athorrent-backend /usr/local/bin/athorrent-backend
COPY --from=build /usr/local/lib/libtorrent-rasterbar.so.2.0 /usr/local/lib/libtorrent-rasterbar.so.2.0

ENTRYPOINT ["/usr/local/bin/athorrent-backend"]

CMD ["--port", "6881"]

HEALTHCHECK --interval=5s --timeout=1s \
    CMD echo '{"action":"ping","parameters":{}}' | socat - UNIX-CONNECT:/var/lib/athorrent-backend/athorrentd.sck | grep -q '{"status":"success","data":"pong"}'