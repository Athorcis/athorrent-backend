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

        void setFlag(const std::string & flag);
        void resetFlag(const std::string & flag);

        static std::string getPath(const std::string & port);

    private:
        TorrentManager * m_torrentManager;
        std::string m_flagDir;
};

#endif /* JSON_SERVICE_H */
