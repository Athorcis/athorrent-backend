#ifndef JSON_CLIENT_H
#define JSON_CLIENT_H

#include "JsonRequest.h"
#include "JsonResponse.h"

template<typename ServerSocketType, typename ClientSocketType>
class JsonServer;

template<typename ServerSocketType, typename ClientSocketType>
class JsonClient {
    typedef JsonClient<ServerSocketType, ClientSocketType> JsonClientType;
    typedef JsonServer<ServerSocketType, ClientSocketType> JsonServerType;
    
    public:
        JsonClient(JsonServerType * server, ClientSocketType * clientSocket);
        ~JsonClient();
        
        void disconnect();
        
    private:
        JsonServerType * m_server;
        ClientSocketType * m_clientSocket;
        
        void asyncRecv();
        void recv();
        
        void send(const JsonResponse * response);
        
        void handleRequest(const JsonRequest * request);
};

#include "../src/JsonClient.cpp"

#endif /* JSON_CLIENT_H */