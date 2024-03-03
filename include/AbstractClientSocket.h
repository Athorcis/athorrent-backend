#ifndef ABSTRACT_LOCAL_CLIENT_SOCKET_H
#define ABSTRACT_LOCAL_CLIENT_SOCKET_H

#include "AbstractSocket.h"
#include <cstdlib>

class AbstractClientSocket : public virtual AbstractSocket {
    public:
        ~AbstractClientSocket() override = default;

        virtual ssize_t read(char * buffer, size_t size) = 0;
        virtual ssize_t write(const char * buffer, size_t size) = 0;
};

#endif /* ABSTRACT_LOCAL_CLIENT_SOCKET_H */
