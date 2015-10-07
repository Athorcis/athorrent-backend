#ifndef LOCAL_SOCKET_H
#define LOCAL_SOCKET_H

#include "AbstractSocket.h"
#include <windows.h>

class LocalSocket : public virtual AbstractSocket {
    public:
        LocalSocket();
        LocalSocket(HANDLE namedPipe);
        ~LocalSocket();
        
        void shutdown();
        void close();
    
    protected:
        HANDLE m_namedPipe;
        HANDLE m_ioThread;
};

#endif /* LOCAL_SOCKET_H */