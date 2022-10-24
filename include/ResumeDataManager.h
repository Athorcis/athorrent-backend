#ifndef RESUME_DATA_MANAGER_H
#define RESUME_DATA_MANAGER_H

#include "TorrentManager.h"

#include <libtorrent/torrent_handle.hpp>
#include <libtorrent/session.hpp>
#include <libtorrent/entry.hpp>

#include <boost/asio/deadline_timer.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/mutex.hpp>

#include <string>
#include <vector>

class ResumeDataManager
{
    public:
        ResumeDataManager(TorrentManager & torrentManager);

        void start(boost::asio::io_service & ioService);
        void reset();
        void stop();

        void requestGlobalSaveResumeData();
        void requestSaveResumeData(const libtorrent::torrent_handle & handle);

        void loadResumeData(const std::string & hash, std::vector<char> & data);
        void saveResumeData(const libtorrent::torrent_handle & handle, std::shared_ptr<libtorrent::entry> resumeData);

        bool hasGlobalSaveResumeDataPending() const;
        bool hasSaveResumeDataPending() const;

        void waitForSaveResumeDataEnd();

    protected:
        TorrentManager & m_torrentManager;
        libtorrent::session & m_session;

        bool m_globalSaveResumeDataPending = false;
        int m_saveResumeDataPending = 0;

        boost::mutex m_resumeDataMutex;
        boost::condition_variable m_resumeDataCondition;

        boost::asio::deadline_timer * m_timer;

        void tryReleaseLock();
};

#endif // RESUME_DATA_MANAGER_H
