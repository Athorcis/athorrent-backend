#ifndef ABSTRACT_LOCAL_SERVER_SOCKET_H
#define ABSTRACT_LOCAL_SERVER_SOCKET_H

#include "AbstractClientSocket.h"
#include <string>

class AbstractServerSocket : public virtual AbstractSocket {
    public:
        AbstractServerSocket(const std::string & path) : m_path(path) {}
        virtual ~AbstractServerSocket() {}

        virtual AbstractClientSocket * accept() = 0;

    protected:
        std::string m_path;
};

#endif /* ABSTRACT_LOCAL_SERVER_SOCKET_H */
