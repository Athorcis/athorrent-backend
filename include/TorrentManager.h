#ifndef TORRENT_MANAGER_H
#define TORRENT_MANAGER_H

#include <libtorrent/session.hpp>

#include <string>
#include <vector>

class AlertManager;
class ResumeDataManager;

class TorrentManager
{
    public:
        TorrentManager(std::string userId, std::string port);
    
        void createTorrent(libtorrent::torrent_handle & handle);

        libtorrent::session & getSession();
        AlertManager & getAlertManager();
        ResumeDataManager & getResumeDataManager();
        
        const std::string & getResumeDataPath() const;
    
        void writeBencodedTree(const std::string & path, const libtorrent::entry & entry);
    
        bool hasTorrent(libtorrent::sha1_hash hash);
        bool hasTorrent(std::string hash);

        libtorrent::torrent_handle getTorrent(libtorrent::sha1_hash hash);
        libtorrent::torrent_handle getTorrent(std::string hash);

        std::vector<libtorrent::torrent_handle> getTorrents();

        bool pauseTorrent(std::string hash);
        bool resumeTorrent(std::string hash);

        bool removeTorrent(std::string hash);

        std::string addTorrentFromFile(std::string path, bool resumeData = false);
        void addTorrentsFromDirectory(std::string path);

        std::string addTorrentFromMagnet(std::string uri);

    private:
        std::string m_userId;

        std::string m_torrentsPath;
        std::string m_resumeDataPath;
        std::string m_filesPath;
    
        libtorrent::session m_session;
    
        ResumeDataManager * m_resumeDataManager;
        AlertManager * m_alertManager;
};

#endif /* TORRENT_MANAGER_H */
