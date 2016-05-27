#ifndef LOCAL_CLIENT_SOCKET_H
#define LOCAL_CLIENT_SOCKET_H

#include "linux/LocalSocket.h"
#include "AbstractClientSocket.h"

class LocalClientSocket : public virtual LocalSocket, public virtual AbstractClientSocket {
    public:
        LocalClientSocket(int socket);

        ssize_t read(char * buffer, size_t size);
        ssize_t write(const char * buffer, size_t size);

        void flush();
};

#endif /* LOCAL_CLIENT_SOCKET_H */
