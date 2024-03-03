
#include <boost/bind/bind.hpp>
#include <boost/thread.hpp>

template<typename ServerSocketType, typename ClientSocketType>
JsonServer<ServerSocketType, ClientSocketType>::JsonServer(const std::string & address) {
    m_serverSocket = new ServerSocketType(address);
}

template<typename ServerSocketType, typename ClientSocketType>
JsonServer<ServerSocketType, ClientSocketType>::~JsonServer() {
    stop();
}

template<typename ServerSocketType, typename ClientSocketType>
void JsonServer<ServerSocketType, ClientSocketType>::start() {
    boost::thread thread(boost::bind(&JsonServerType::run, this));
}

template<typename ServerSocketType, typename ClientSocketType>
void JsonServer<ServerSocketType, ClientSocketType>::run() {
    ClientSocketType * clientSocket;

    do {
        clientSocket = reinterpret_cast<ClientSocketType *>(m_serverSocket->accept());

        if (clientSocket) {
            m_clients.insert(new JsonClientType(this, clientSocket));
        } else {
            break;
        }
    } while (true);
}

template<typename ServerSocketType, typename ClientSocketType>
void JsonServer<ServerSocketType, ClientSocketType>::stop() {
    m_serverSocket->shutdown();

    for (JsonClientType * client : m_clients) {
        delete client;
    }

    m_clients.clear();

    m_serverSocket->close();
    delete m_serverSocket;
}

template<typename ServerSocketType, typename ClientSocketType>
void JsonServer<ServerSocketType, ClientSocketType>::removeClient(JsonClientType * client) {
    m_clients.erase(client);
}

template<typename ServerSocketType, typename ClientSocketType>
JsonResponse * JsonServer<ServerSocketType, ClientSocketType>::handleRequest(const JsonRequest * request) {
    return nullptr;
}
