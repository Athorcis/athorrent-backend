#ifdef _WIN32
#   include "win32/LocalClientSocket.h"
#elif defined __linux__
#   include "linux/LocalClientSocket.h"
#endif