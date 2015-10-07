#include "win32/LocalClientSocket.h"

LocalClientSocket::LocalClientSocket(HANDLE namedPipe) : LocalSocket(namedPipe) {
}

ssize_t LocalClientSocket::read(char * buffer, size_t size) {
    ssize_t bytesRead;
    
    m_ioThread = GetCurrentThread();
    
    if (!ReadFile(m_namedPipe, buffer, size, reinterpret_cast<LPDWORD>(&bytesRead), NULL)) {
        bytesRead = -1;
    }
    
    m_ioThread = NULL;
    
    return bytesRead;
}

ssize_t LocalClientSocket::write(const char * buffer, size_t size) {
    ssize_t bytesWritten;
    
    m_ioThread = GetCurrentThread();
    
    if (WriteFile(m_namedPipe, buffer, size, reinterpret_cast<LPDWORD>(&bytesWritten), NULL)) {
        FlushFileBuffers(m_namedPipe); 
    } else {
        bytesWritten = -1;
    }
    
    m_ioThread = NULL;
    
    return bytesWritten;
}