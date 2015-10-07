#ifndef LOCAL_SOCKET_H
#define LOCAL_SOCKET_H

#include "AbstractSocket.h"

class LocalSocket : public virtual AbstractSocket {
    public:
        LocalSocket();
        LocalSocket(int socket);
        ~LocalSocket();
        
        void shutdown();
        void close();
    
    protected:
        int m_socket;
};

#endif /* LOCAL_SOCKET_H */