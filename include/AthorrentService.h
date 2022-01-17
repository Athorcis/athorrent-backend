#ifndef JSON_SERVICE_H
#define JSON_SERVICE_H

#include "TorrentManager.h"
#include "JsonServer.h"
#include "LocalServerSocket.h"
#include "LocalClientSocket.h"

class AthorrentService : public JsonServer<LocalServerSocket, LocalClientSocket> {
    public:
        AthorrentService(const std::string & address, TorrentManager * torrentManager);

        void run();
        void stop();

        JsonResponse * handleRequest(const JsonRequest * request);

        static std::string getPath(const std::string & port);

    private:
        TorrentManager * m_torrentManager;
};

#endif /* JSON_SERVICE_H */
