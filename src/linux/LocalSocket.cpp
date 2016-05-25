#include "LocalSocket.h"

#include <sys/socket.h>
#include <unistd.h>

LocalSocket::LocalSocket() : m_socket(-1) {
}

LocalSocket::LocalSocket(int socket) : m_socket(socket) {
}

LocalSocket::~LocalSocket() {
    close();
}

void LocalSocket::shutdown() {
    ::shutdown(m_socket, SHUT_RDWR);
}

void LocalSocket::close() {
    if (m_socket > -1) {
        ::close(m_socket);
        m_socket = -1;
    }
}
