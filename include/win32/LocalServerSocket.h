#ifndef LOCAL_SERVER_SOCKET_H
#define LOCAL_SERVER_SOCKET_H

#include "win32/LocalClientSocket.h"
#include "AbstractServerSocket.h"

class LocalServerSocket : public virtual LocalSocket, public virtual AbstractServerSocket {
    public:
        explicit LocalServerSocket(const std::string & path);

        AbstractClientSocket * accept() override;

    private:
        void createNamedPipe();
};

#endif /* LOCAL_SERVER_SOCKET_H */
