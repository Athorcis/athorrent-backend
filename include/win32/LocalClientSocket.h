#ifndef LOCAL_CLIENT_SOCKET_H
#define LOCAL_CLIENT_SOCKET_H

#include "win32/LocalSocket.h"
#include "AbstractClientSocket.h"

class LocalClientSocket : public virtual LocalSocket, public virtual AbstractClientSocket {
    public:
        LocalClientSocket(HANDLE namedPipe);

        ssize_t read(char * buffer, size_t size);
        ssize_t write(const char * buffer, size_t size);
};

#endif /* LOCAL_CLIENT_SOCKET_H */
