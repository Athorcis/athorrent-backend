#ifndef JSON_SERVER_H
#define JSON_SERVER_H

#include <string>

#include "JsonClient.h"

template<typename ServerSocketType, typename ClientSocketType>
class JsonServer {
    typedef JsonClient<ServerSocketType, ClientSocketType> JsonClientType;
    typedef JsonServer<ServerSocketType, ClientSocketType> JsonServerType;

    public:
        explicit JsonServer(const std::string & address);
        virtual ~JsonServer();

        void start();
        virtual void run();
        virtual void stop();

        void removeClient(JsonClientType * client);

        virtual JsonResponse * handleRequest(const JsonRequest * request);

    private:
        ServerSocketType * m_serverSocket;
        std::set<JsonClientType *> m_clients;
};

#include "../src/JsonServer.cpp"

#endif /* JSON_SERVER_H */
