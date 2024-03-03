#include "win32/LocalServerSocket.h"

LocalServerSocket::LocalServerSocket(const std::string & path) : AbstractServerSocket(path) {
    createNamedPipe();
}

AbstractClientSocket * LocalServerSocket::accept() {
    LocalClientSocket * clientSocket = nullptr;

    if (m_namedPipe) {
        if (ConnectNamedPipe(m_namedPipe, nullptr)) {
            clientSocket = new LocalClientSocket(m_namedPipe);
        }

        createNamedPipe();
    }

    return clientSocket;
}

void LocalServerSocket::createNamedPipe() {
    m_namedPipe = CreateNamedPipe(m_path.c_str(), PIPE_ACCESS_DUPLEX, PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT, PIPE_UNLIMITED_INSTANCES, 1024, 1024, 0, nullptr);
}
