#ifndef TORRENT_MANAGER_H
#define TORRENT_MANAGER_H

#include <libtorrent/session.hpp>
#include <libtorrent/alert_types.hpp>

#include <boost/thread/condition_variable.hpp>
#include <boost/thread/mutex.hpp>

#include <string>
#include <vector>

class TorrentManager
{
    public:
        TorrentManager(std::string userId);

        void eventLoop();

        void requestSaveResumeData();
        bool requestSaveResumeData(libtorrent::torrent_handle handle, bool global = false);
        void handleSaveResumeDataAlert(libtorrent::save_resume_data_alert * alert);
        
        bool isGlobalSaveResumeDataPending() const;
        void waitForSaveResumeData();
        
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

        std::string addTorrentFromFile(std::string path, bool resumeData = false);
        void addTorrentsFromDirectory(std::string path);

        std::string addTorrentFromMagnet(std::string uri);

    private:
        std::string m_userId;

        std::string m_torrentsPath;
        std::string m_resumeDataPath;
        std::string m_filesPath;
        boost::mutex m_resumeDataMutex;
        boost::condition_variable m_resumeDataCondition;
        
        int m_requestedSaveResumeData;
        bool m_globalSaveResumeDataPending;
    
        libtorrent::session m_session;
};

#endif /* TORRENT_MANAGER_H */
