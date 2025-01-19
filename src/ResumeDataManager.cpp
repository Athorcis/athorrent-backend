#include "ResumeDataManager.h"

#include <libtorrent/torrent_status.hpp>
#include <libtorrent/read_resume_data.hpp>
#include <libtorrent/write_resume_data.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/hex.hpp>
#include <fstream>
#include <iostream>

using namespace std;

namespace fs = boost::filesystem;
namespace lt = libtorrent;

ResumeDataManager::ResumeDataManager(TorrentManager & torrentManager) : m_torrentManager(torrentManager), m_session(torrentManager.getSession()) {}

void ResumeDataManager::start(boost::asio::io_context & ioService)
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

lt::add_torrent_params ResumeDataManager::loadResumeData(const string & hash, bool & resumeDataLoaded)
{
    lt::add_torrent_params atp;

    string path = m_torrentManager.getResumeDataPath() + '/' + hash + ".fastresume";

    try {
        if (fs::exists(path) && fs::file_size(path) > 0) {
            ifstream file(path.c_str(), ifstream::in | ifstream::binary);
            file.unsetf(ios::skipws);
            istream_iterator<char> start(file), end;

            vector<char> buf(start, end);

            atp = lt::read_resume_data(buf);
            resumeDataLoaded = true;
        }
    } catch (const std::exception & except) {
        cerr << "failed to load fastresume data for " << path << " " << except.what() << endl;
    }

    return atp;
}

void ResumeDataManager::saveResumeData(const lt::save_resume_data_alert * alert)
{
    lt::torrent_handle handle = alert->handle;

    string hash = boost::algorithm::hex(handle.info_hash().to_string());
    string path = m_torrentManager.getResumeDataPath() + '/' + hash + ".fastresume";

    std::ofstream of(path, std::ios_base::binary);
    of.unsetf(std::ios_base::skipws);
    auto const b = write_resume_data_buf(alert->params);
    of.write(b.data(), int(b.size()));

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
