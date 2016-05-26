#ifndef TORRENT_MANAGER_H
#define TORRENT_MANAGER_H

#include <libtorrent/session.hpp>

#include <string>
#include <vector>

class TorrentManager {
    public:
        TorrentManager(std::string userId);

        void eventLoop();

        bool hasTorrent(libtorrent::sha1_hash hash);
        bool hasTorrent(std::string hash);

        libtorrent::torrent_handle getTorrent(libtorrent::sha1_hash hash);
        libtorrent::torrent_handle getTorrent(std::string hash);

        std::vector<libtorrent::torrent_handle> getTorrents();

        bool pauseTorrent(std::string hash);
        bool resumeTorrent(std::string hash);

        bool removeTorrent(std::string hash);

        void loadFastResumeData(std::string hash, std::vector<char> & data);
        void saveFastResumeData(std::string hash, libtorrent::entry & entry);

        std::string addTorrentFromFile(std::string path);
        void addTorrentsFromDirectory(std::string path);

        std::string addTorrentFromMagnet(std::string uri);

    private:
        std::string m_userId;

        std::string m_torrentsPath;
        std::string m_filesPath;

        libtorrent::session m_session;
};

#endif /* TORRENT_MANAGER_H */
