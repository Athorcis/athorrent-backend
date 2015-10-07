#ifdef _WIN32
#   include "win32/LocalClientSocket.cpp"
#elif defined __linux__
#   include "linux/LocalClientSocket.cpp"
#endif