cmake_minimum_required(VERSION 3.23)
project(athorrent_backend)

set(TARGET_NAME athorrent_backend)
set(CMAKE_CXX_STANDARD 14)

if (WIN32)
    set(BoostStacktraceSuffix "_windbg")
endif()

if (LINUX)
    set(BoostStacktraceSuffix "_backtrace")
endif()

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost 1.47.0 COMPONENTS REQUIRED filesystem program_options stacktrace${BoostStacktraceSuffix} thread)

include_directories(include)

add_definitions(-DRAPIDJSON_HAS_STDSTRING=1 -DBOOST_BIND_NO_PLACEHOLDERS)
set(CMAKE_VERBOSE_MAKEFILE ON)
add_executable(${TARGET_NAME}
        src/AlertManager.cpp
        src/AthorrentService.cpp
        src/BadJsonRequestException.cpp
        src/JsonRequest.cpp
        src/JsonRequestFailedException.cpp
        src/JsonResponse.cpp
        src/LocalClientSocket.cpp
        src/LocalServerSocket.cpp
        src/LocalSocket.cpp
        src/main.cpp
        src/ResumeDataManager.cpp
        src/TorrentManager.cpp)

add_compile_options("-Wpedantic" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wno-unused-parameter"
        "$<$<CONFIG:DEBUG>:-O0;-g3;-ggdb>"
        "$<$<CONFIG:RELEASE>:-O3>"
        )
TARGET_LINK_LIBRARIES(${TARGET_NAME} ${Boost_LIBRARIES} -ltorrent-rasterbar -lcrypto -lssl)

include_directories(${Boost_INCLUDE_DIRS})

add_link_options("${Boost_LIBRARIES} -ltorrent-rasterbar -lcrypto -lssl")

if (WIN32)
    TARGET_LINK_LIBRARIES(${TARGET_NAME} -lws2_32 -lcrypt32 -lwsock32 -lIphlpapi)
endif()

if (LINUX)
    TARGET_LINK_LIBRARIES(${TARGET_NAME} -ldl -lbacktrace)
endif()
