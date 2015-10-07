#ifdef _WIN32
#   include "win32/LocalSocket.h"
#elif defined __linux__
#   include "linux/LocalSocket.h"
#endif