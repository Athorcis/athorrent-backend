
#include <boost/bind/bind.hpp>
#include <boost/thread.hpp>
#include <iostream>
#include "BadJsonRequestException.h"
#include "JsonClient.h"
#include "JsonRequestFailedException.h"


template<typename ServerSocketType, typename ClientSocketType>
JsonClient<ServerSocketType, ClientSocketType>::JsonClient(JsonServerType * server, ClientSocketType * clientSocket) : m_server(server), m_clientSocket(clientSocket) {
    asyncRecv();
}

template<typename ServerSocketType, typename ClientSocketType>
JsonClient<ServerSocketType, ClientSocketType>::~JsonClient() {
    disconnect();
    delete m_clientSocket;
}

template<typename ServerSocketType, typename ClientSocketType>
void JsonClient<ServerSocketType, ClientSocketType>::disconnect() {
    m_clientSocket->shutdown();
    m_clientSocket->close();
}

template<typename ServerSocketType, typename ClientSocketType>
void JsonClient<ServerSocketType, ClientSocketType>::asyncRecv() {
    boost::thread thread(boost::bind(&JsonClientType::recv, this));
}

template<typename ServerSocketType, typename ClientSocketType>
void JsonClient<ServerSocketType, ClientSocketType>::recv() {
    char * buffer = new char[1024];
    std::string rawRequest;

    do {
        ssize_t bytesRead = m_clientSocket->read(buffer, 1024);

        if (bytesRead > 0) {
            rawRequest.append(buffer, bytesRead);
        } else {
            break;
        }
    } while (rawRequest[rawRequest.size() - 1] != '\n');

    delete[] buffer;

    if (rawRequest.empty()) {
        std::cerr << "empty request received" << std::endl;

        auto *response = new JsonResponse();
        response->setError(std::string("bad request: empty request received"));

        sendResponseAndDisconnect(response);
    }
    else {
        try {
            handleRequest(new JsonRequest(rawRequest));
        }
        catch (const BadJsonRequestException &e) {

            auto *response = new JsonResponse();
            response->setError(std::string("bad request: ") + e.what());

            sendResponseAndDisconnect(response);
        }
    }
}

template<typename ServerSocketType, typename ClientSocketType>
void JsonClient<ServerSocketType, ClientSocketType>::send(const JsonResponse * response) {
    std::string rawResponse = response->toRawResponse();
    size_t length = rawResponse.size();
    size_t offset = 0;

    while (offset < length) {
        ssize_t bytesWritten = m_clientSocket->write(rawResponse.c_str() + offset, std::min<size_t>(1024, length - offset));

        if (bytesWritten > 0) {
            offset += bytesWritten;
        } else {
            break;
        }
    }
}

template<typename ServerSocketType, typename ClientSocketType>
void JsonClient<ServerSocketType, ClientSocketType>::handleRequest(const JsonRequest * request) {
    JsonResponse * response;

    if (request == nullptr) {
        response = new JsonResponse();
        response->setError("bad request");
    } else {
        try {
            response = m_server->handleRequest(request);
        }
        catch (const JsonRequestFailedException & except) {
            response = new JsonResponse();
            response->setError(except.what(), except.getId());
        }
        catch (const std::exception & except) {
            response = new JsonResponse();
            response->setError(except.what());
        }

        delete request;
        
        if (response->getStatus().empty()) {
            response->setError("not found");
        }
    }

    sendResponseAndDisconnect(response);
}

template<typename ServerSocketType, typename ClientSocketType>
void JsonClient<ServerSocketType, ClientSocketType>::sendResponseAndDisconnect(const JsonResponse *response) {
    send(response);
    delete response;

    disconnect();
    m_server->removeClient(this);
}
