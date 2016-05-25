#ifndef LOCAL_SERVER_SOCKET_H
#define LOCAL_SERVER_SOCKET_H

#include "linux/LocalClientSocket.h"
#include "AbstractServerSocket.h"

class LocalServerSocket : public virtual LocalSocket, public virtual AbstractServerSocket {
    public:
        LocalServerSocket(const std::string & path);

        AbstractClientSocket * accept();

        void close();
};

#endif /* LOCAL_SERVER_SOCKET_H */
