FROM debian:12.9-slim AS base

FROM base AS builder

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
      libboost-filesystem-dev \
      libboost-program-options-dev \
      libboost-random-dev \
      libboost-stacktrace-dev \
      libboost-system-dev \
      libboost-thread-dev \
      libssl-dev \
      libtool \
      make \
      pkg-config

FROM builder AS debug-build

RUN set -ex ;\
    git clone --recurse-submodules --depth 1 --branch v2.0.10 https://github.com/arvidn/libtorrent ;\
    cd libtorrent ;\
    cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_STANDARD=14 . ;\
    cmake --build . -- -j $(nproc) ;\
    cmake --install .

RUN set -ex ;\
    git clone https://github.com/Tencent/rapidjson/ ;\
    cd rapidjson ;\
    cmake -DCMAKE_BUILD_TYPE=Debug . ;\
    cmake --install .

COPY . project

RUN set -ex ;\
    cd project ;\
    cmake -DCMAKE_BUILD_TYPE=Debug . ;\
    cmake --build . -- -j $(nproc)

FROM builder AS release-build

RUN set -ex ;\
    git clone --recurse-submodules --depth 1 --branch v2.0.10 https://github.com/arvidn/libtorrent ;\
    cd libtorrent ;\
    cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_STANDARD=14 . ;\
    cmake --build . -- -j $(nproc) ;\
    cmake --install .

RUN set -ex ;\
    git clone https://github.com/Tencent/rapidjson/ ;\
    cd rapidjson ;\
    cmake -DCMAKE_BUILD_TYPE=Release . ;\
    cmake --install .

COPY . project

RUN set -ex ;\
    cd project ;\
    cmake -DCMAKE_BUILD_TYPE=Release . ;\
    cmake --build . -- -j $(nproc)

FROM base AS final-base

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

ENTRYPOINT ["/usr/local/bin/athorrent-backend"]

CMD ["--port", "6881"]

HEALTHCHECK --interval=5s --timeout=1s \
    CMD echo '{"action":"ping","parameters":{}}' | socat - UNIX-CONNECT:/var/lib/athorrent-backend/athorrentd.sck | grep -q '{"status":"success","data":"pong"}'

FROM final-base AS debug

COPY --from=debug-build /dist/src/athorrent-backend /usr/local/bin/athorrent-backend
COPY --from=debug-build /usr/local/lib/libtorrent-rasterbar.so.2.0 /usr/local/lib/libtorrent-rasterbar.so.2.0

FROM final-base AS release

COPY --from=release-build /dist/src/athorrent-backend /usr/local/bin/athorrent-backend
COPY --from=release-build /usr/local/lib/libtorrent-rasterbar.so.2.0 /usr/local/lib/libtorrent-rasterbar.so.2.0
