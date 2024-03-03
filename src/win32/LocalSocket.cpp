#include "win32/LocalSocket.h"

LocalSocket::LocalSocket() : m_namedPipe(nullptr), m_ioThread(nullptr) {
}

LocalSocket::LocalSocket(HANDLE namedPipe) : m_namedPipe(namedPipe), m_ioThread(nullptr) {
}

LocalSocket::~LocalSocket() {
    LocalSocket::close();
}

void LocalSocket::shutdown() {
    if (m_ioThread) {
        CancelSynchronousIo(m_ioThread);
        m_ioThread = nullptr;
    }
}

void LocalSocket::close() {
    if (m_namedPipe) {
        DisconnectNamedPipe(m_namedPipe);
        CloseHandle(m_namedPipe);
        m_namedPipe = nullptr;
    }
}
