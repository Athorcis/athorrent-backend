#ifdef _WIN32
#   include "win32/LocalSocket.cpp"
#elif defined __linux__
#   include "linux/LocalSocket.cpp"
#endif
