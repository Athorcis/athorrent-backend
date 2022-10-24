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
        TorrentManager(const std::string & port);
    
        void createTorrent(const libtorrent::torrent_handle & handle);

        libtorrent::session & getSession();
        AlertManager & getAlertManager();
        ResumeDataManager & getResumeDataManager();
        
        const std::string & getResumeDataPath() const;
    
        void writeBencodedTree(const std::string & path, const libtorrent::entry & entry);
    
        bool hasTorrent(libtorrent::sha1_hash hash) const;
        bool hasTorrent(const std::string & hash) const;

        libtorrent::torrent_handle getTorrent(libtorrent::sha1_hash hash) const;
        libtorrent::torrent_handle getTorrent(const std::string & hash) const;

        std::vector<libtorrent::torrent_handle> getTorrents();

        bool pauseTorrent(const std::string & hash);
        bool resumeTorrent(const std::string & hash);

        bool removeTorrent(const std::string & hash);

        std::string addTorrentFromFile(const std::string & path, bool resumeData = false);
        void addTorrentsFromDirectory(const std::string & path);

        std::string addTorrentFromMagnet(const std::string & uri);

    private:
        std::string m_torrentsPath = "cache/torrents";
        std::string m_resumeDataPath = "cache/fastresume";
        std::string m_filesPath = "files";
    
        libtorrent::session m_session;
    
        ResumeDataManager * m_resumeDataManager;
        AlertManager * m_alertManager;
};

#endif /* TORRENT_MANAGER_H */
