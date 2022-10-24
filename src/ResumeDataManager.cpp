#include "ResumeDataManager.h"
#include "Utils.h"

#include <libtorrent/torrent_status.hpp>
#include <boost/filesystem.hpp>
#include <fstream>
#include <iostream>

using namespace std;

namespace fs = boost::filesystem;
namespace lt = libtorrent;

ResumeDataManager::ResumeDataManager(TorrentManager & torrentManager) : m_torrentManager(torrentManager), m_session(torrentManager.getSession()) {}

void ResumeDataManager::start(boost::asio::io_service & ioService)
{
    m_timer = new boost::asio::deadline_timer(ioService, boost::posix_time::seconds(1));
    reset();
}

void ResumeDataManager::reset()
{
    m_timer->expires_at(m_timer->expires_at() + boost::posix_time::minutes(5));
    m_timer->async_wait([this](const boost::system::error_code & error) {
        if (!hasGlobalSaveResumeDataPending()) {
            requestGlobalSaveResumeData();
        }
    });
}

void ResumeDataManager::stop()
{
    m_session.pause();
    
    if (!hasGlobalSaveResumeDataPending()) {
        requestGlobalSaveResumeData();
    }

    waitForSaveResumeDataEnd();
}

void ResumeDataManager::requestGlobalSaveResumeData()
{
    waitForSaveResumeDataEnd();
    m_globalSaveResumeDataPending = true;

    std::vector<lt::torrent_handle> handles = m_session.get_torrents();

    for (const auto & handle : handles) {
        if (!handle.is_valid()) {
            continue;
        }

        lt::torrent_status status = handle.status();

        if (!status.has_metadata) {
            continue;
        }

        if (handle.need_save_resume_data()) {
            ++m_saveResumeDataPending;
            handle.save_resume_data(lt::torrent_handle::flush_disk_cache | lt::torrent_handle::save_info_dict);
        }
    }

    tryReleaseLock();
}

void ResumeDataManager::requestSaveResumeData(const lt::torrent_handle & handle)
{
    waitForSaveResumeDataEnd();

    if (handle.need_save_resume_data()) {
        ++m_saveResumeDataPending;
        handle.save_resume_data(lt::torrent_handle::flush_disk_cache | lt::torrent_handle::save_info_dict);
    } else {
        tryReleaseLock();
    }
}

void ResumeDataManager::loadResumeData(const string & hash, vector<char> & data)
{
    string path = m_torrentManager.getResumeDataPath() + '/' + hash + ".fastresume";

    try {
        if (fs::file_size(path) > 0) {
            ifstream file(path.c_str(), ifstream::in | ifstream::binary);
            file.unsetf(ios::skipws);

            istream_iterator<char> start(file), end;
            data.assign(start, end);
        }
    } catch (const std::exception & except) {
        cerr << "failed to load fastresume data" << endl;
    }
}

void ResumeDataManager::saveResumeData(const lt::torrent_handle & handle, std::shared_ptr<lt::entry> resumeData)
{
    string hash = bin2hex(handle.info_hash().to_string());
    string path = m_torrentManager.getResumeDataPath() + '/' + hash + ".fastresume";

    m_torrentManager.writeBencodedTree(path, *resumeData);

    if (hasSaveResumeDataPending()) {
        --m_saveResumeDataPending;
        tryReleaseLock();
    }
}

bool ResumeDataManager::hasGlobalSaveResumeDataPending() const
{
    return m_globalSaveResumeDataPending;
}

bool ResumeDataManager::hasSaveResumeDataPending() const
{
    return m_saveResumeDataPending > 0 || hasGlobalSaveResumeDataPending();
}

void ResumeDataManager::waitForSaveResumeDataEnd()
{
    if (hasSaveResumeDataPending()) {
        boost::mutex::scoped_lock lock(m_resumeDataMutex);
        m_resumeDataCondition.wait(lock);
    }
}

void ResumeDataManager::tryReleaseLock()
{
    if (m_saveResumeDataPending == 0) {
        if (hasGlobalSaveResumeDataPending()) {
            m_globalSaveResumeDataPending = false;
        }

        m_resumeDataCondition.notify_one();
    }
}
