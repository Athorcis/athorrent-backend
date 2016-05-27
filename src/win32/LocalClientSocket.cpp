#include "win32/LocalClientSocket.h"

LocalClientSocket::LocalClientSocket(HANDLE namedPipe) : LocalSocket(namedPipe) {
}

ssize_t LocalClientSocket::read(char * buffer, size_t size) {
    DWORD bytesRead;
    ssize_t result;

    m_ioThread = GetCurrentThread();

    if (ReadFile(m_namedPipe, buffer, size, &bytesRead, NULL)) {
        result = bytesRead;
    } else {
        result = -1;
    }

    m_ioThread = NULL;

    return result;
}

ssize_t LocalClientSocket::write(const char * buffer, size_t size) {
    DWORD bytesWritten;
    ssize_t result;

    m_ioThread = GetCurrentThread();

    if (WriteFile(m_namedPipe, buffer, size, &bytesWritten, NULL)) {
        FlushFileBuffers(m_namedPipe);
        result = bytesWritten;
    } else {
        result = -1;
    }

    m_ioThread = NULL;

    return result;
}

