#include "win32/LocalSocket.h"

LocalSocket::LocalSocket() : m_namedPipe(NULL), m_ioThread(NULL) {
}

LocalSocket::LocalSocket(HANDLE namedPipe) : m_namedPipe(namedPipe), m_ioThread(NULL) {
}

LocalSocket::~LocalSocket() {
    close();
}

void LocalSocket::shutdown() {
    if (m_ioThread) {
        CancelSynchronousIo(m_ioThread);
        m_ioThread = NULL;
    }
}

void LocalSocket::close() {
    if (m_namedPipe) {
        DisconnectNamedPipe(m_namedPipe);
        CloseHandle(m_namedPipe);
        m_namedPipe = NULL;
    }
}
