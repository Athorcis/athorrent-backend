#include "linux/LocalClientSocket.h"

#include <unistd.h>
#include <cstdio>

LocalClientSocket::LocalClientSocket(int socket) : LocalSocket(socket) {
}

ssize_t LocalClientSocket::read(char * buffer, size_t length) {
    return ::read(m_socket, buffer, length);
}

ssize_t LocalClientSocket::write(const char * buffer, size_t length) {
    return ::write(m_socket, buffer, length);
}

void LocalClientSocket::flush()
{
    ::fflush(m_socket);
}
