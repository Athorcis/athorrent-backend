#include "linux/LocalServerSocket.h"

#include <iostream>

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include <libgen.h>
#include <sys/stat.h>

LocalServerSocket::LocalServerSocket(const std::string & path) : AbstractServerSocket(path) {
    struct sockaddr_un address;
    
    char * tmp = new char[m_path.size() + 1];
    strcpy(tmp, m_path.c_str());
    
    mkdir(dirname(tmp), 0755);
    delete tmp;
    
    address.sun_family = AF_UNIX;
    strcpy(address.sun_path, m_path.c_str());
    
    m_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    bind(m_socket, reinterpret_cast<struct sockaddr *>(&address), sizeof(struct sockaddr_un));
    listen(m_socket, 5);
}

void LocalServerSocket::close() {
    if (m_socket > -1) {
        LocalSocket::close();
        unlink(m_path.c_str());
    }
}

AbstractClientSocket * LocalServerSocket::accept() {
    LocalClientSocket * clientSocket = NULL;
    
    int socket = ::accept(m_socket, NULL, NULL);
    
    if (socket > -1) {
        clientSocket = new LocalClientSocket(socket);
    } else {
        std::cerr << "LocalServerSocket::accept: " << strerror(errno) << std::endl;
    }
    
    return clientSocket;
}