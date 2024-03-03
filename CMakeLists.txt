cmake_minimum_required(VERSION 3.23)
project(athorrent_backend)

set(CMAKE_CXX_STANDARD 14)

include_directories(include)

add_definitions(-DRAPIDJSON_HAS_STDSTRING=1 -DBOOST_BIND_NO_PLACEHOLDERS)
set(CMAKE_VERBOSE_MAKEFILE ON)
add_executable(athorrent_backend
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

TARGET_LINK_LIBRARIES(athorrent_backend -ltorrent-rasterbar -lboost_filesystem-mt -lboost_program_options-mt -lboost_stacktrace_windbg-mt -lboost_thread-mt -lws2_32 -lcrypt32 -lwsock32 -lIphlpapi -lcrypto -lssl)