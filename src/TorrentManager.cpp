#include "TorrentManager.h"
#include "Utils.h"

#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/filesystem.hpp>
#include <libtorrent/bencode.hpp>
#include <libtorrent/create_torrent.hpp>
#include <libtorrent/alert_types.hpp>
#include <libtorrent/torrent_info.hpp>
#include <libtorrent/settings_pack.hpp>
#include <libtorrent/error_code.hpp>


#include <fstream>

using namespace std;

TorrentManager::TorrentManager(string userId) :
    m_userId(userId),
    m_torrentsPath("cache/" + m_userId + "/torrents"),
    m_resumeDataPath("cache/" + m_userId + "/fastresume"),
    m_filesPath("files/" + m_userId),
    m_requestedSaveResumeData(0) {
    if (!boost::filesystem::exists(m_torrentsPath)) {
        boost::filesystem::create_directory(m_torrentsPath);
    }

    if (!boost::filesystem::exists(m_resumeDataPath)) {
        boost::filesystem::create_directory(m_resumeDataPath);
    }

    if (!boost::filesystem::exists(m_filesPath)) {
        boost::filesystem::create_directory(m_filesPath);
    }

    libtorrent::settings_pack settings;
    settings.set_int(libtorrent::settings_pack::alert_mask, libtorrent::alert::status_notification  | libtorrent::alert::storage_notification | libtorrent::alert::error_notification);

    m_session.apply_settings(settings);

    boost::thread thread(boost::bind(&TorrentManager::eventLoop, this));
    
    addTorrentsFromDirectory(m_torrentsPath);
}

void TorrentManager::eventLoop() {
    libtorrent::time_duration timeout = libtorrent::seconds(15);
    libtorrent::torrent_handle torrent;
    boost::thread thread;
    
    while (true) {
        m_session.wait_for_alert(timeout);
        
        std::vector<libtorrent::alert *> alerts;
        
        m_session.pop_alerts(&alerts);
        
        for (auto alert : alerts) {
            switch (alert->type()) {
                case libtorrent::save_resume_data_alert::alert_type:
                    handleSaveResumeDataAlert(libtorrent::alert_cast<libtorrent::save_resume_data_alert>(alert));
                    break;

                case libtorrent::torrent_finished_alert::alert_type:
                    torrent = libtorrent::alert_cast<libtorrent::torrent_finished_alert>(alert)->handle;
                    std::cout << "torrent finished" << std::endl;
                   thread = boost::thread(boost::bind(&TorrentManager::requestSaveResumeData, this), torrent);
                    break;
                    
                case libtorrent::save_resume_data_failed_alert::alert_type:
                    break;
                
                case libtorrent::fastresume_rejected_alert::alert_type:
                    std::cout << "fast resume data rejected" << std::endl;
                    break;

                case libtorrent::metadata_received_alert::alert_type:
                    torrent = libtorrent::alert_cast<libtorrent::metadata_received_alert>(alert)->handle;

                    if (torrent.is_valid()) {
                        boost::shared_ptr<const libtorrent::torrent_info> ti = torrent.torrent_file();
                        libtorrent::create_torrent ct(*ti);
                        libtorrent::entry te = ct.generate();
                        std::vector<char> buffer;
                        libtorrent::bencode(std::back_inserter(buffer), te);

                        FILE* f = fopen((m_torrentsPath + "/" + libtorrent::to_hex(ti->info_hash().to_string()) + ".torrent").c_str(), "wb+");
                        if (f) {
                            fwrite(&buffer[0], 1, buffer.size(), f);
                            fclose(f);
                        }
                    }

                    break;
            }
        }
    }
}

void TorrentManager::requestSaveResumeData() {
    waitForSaveResumeData();
    
    std::cout << "start global save resume data" << endl;
    bool atLeastOneRequest = false;
    m_globalSaveResumeDataPending = true;
    
    std::vector<libtorrent::torrent_handle> handles = m_session.get_torrents();
    m_session.pause();

    for (std::vector<libtorrent::torrent_handle>::iterator i = handles.begin(); i != handles.end(); ++i) {
        libtorrent::torrent_handle& h = *i;

        if (!h.is_valid()) {
            continue;
        }

        libtorrent::torrent_status s = h.status();

        if (!s.has_metadata) {
            continue;
        }

        if (requestSaveResumeData(h, true)) {
            atLeastOneRequest = true;
        }
    }
    
    m_globalSaveResumeDataPending = false;
    
    if (!atLeastOneRequest) {
        m_resumeDataCondition.notify_one();
    }
}

bool TorrentManager::isGlobalSaveResumeDataPending() const
{
    return m_globalSaveResumeDataPending;
}

void TorrentManager::waitForSaveResumeData()
{
    if (m_requestedSaveResumeData > 0) {
        std::cout << "wait for save resume data" << endl;
        boost::mutex::scoped_lock lock(m_resumeDataMutex);
        m_resumeDataCondition.wait(lock);
        std::cout << "wait is over" << endl;
    }
}

bool TorrentManager::requestSaveResumeData(libtorrent::torrent_handle handle, bool global) {
    if (!global) {
        waitForSaveResumeData();
        std::cout << "start save resume data" << endl;
    }
    
    if (handle.need_save_resume_data()) {
        std::cout << "torrent need save resume data" << std::endl;
        handle.save_resume_data(libtorrent::torrent_handle::flush_disk_cache | libtorrent::torrent_handle::save_info_dict);
        ++m_requestedSaveResumeData;
        
        return true;
    } else if (!global) {
        std::cout << "notifying one" << std::endl;
        m_resumeDataCondition.notify_one();
    }
    
    return false;
}

void TorrentManager::handleSaveResumeDataAlert(libtorrent::save_resume_data_alert * alert) {
    std::cout << "save resume data" << std::endl;
    
    libtorrent::torrent_handle handle = alert->handle;
    string hash = libtorrent::to_hex(handle.info_hash().to_string());
    boost::shared_ptr<libtorrent::entry> resume_data = alert->resume_data;

    string path = "cache/" + m_userId + "/fastresume/"+ hash + ".fastresume";

    ofstream stream(path.c_str(), ios::binary);
    stream.unsetf(ios::skipws);
    ostream_iterator<char> start(stream);

    libtorrent::bencode(start, *resume_data);

    if (m_requestedSaveResumeData > 0) {
        --m_requestedSaveResumeData;

        if (m_requestedSaveResumeData == 0) {
            std::cout << "notifying one" << std::endl;
            m_resumeDataCondition.notify_one();
        }
    }
}

bool TorrentManager::hasTorrent(libtorrent::sha1_hash hash) {
    return m_session.find_torrent(hash).is_valid();
}

bool TorrentManager::hasTorrent(string hash) {
    return hasTorrent(libtorrent::sha1_hash(Utils::from_hex(hash)));
}

libtorrent::torrent_handle TorrentManager::getTorrent(libtorrent::sha1_hash hash) {
    return m_session.find_torrent(hash);
}

libtorrent::torrent_handle TorrentManager::getTorrent(string hash) {
    return getTorrent(libtorrent::sha1_hash(Utils::from_hex(hash)));
}

vector<libtorrent::torrent_handle> TorrentManager::getTorrents() {
    return m_session.get_torrents();
}

bool TorrentManager::pauseTorrent(string hash) {
    libtorrent::torrent_handle torrent = getTorrent(hash);

    if (torrent.is_valid()) {
        torrent.auto_managed(false);
        torrent.pause(libtorrent::torrent_handle::graceful_pause);
    } else {
        return false;
    }

    return true;
}

bool TorrentManager::resumeTorrent(string hash) {
    libtorrent::torrent_handle torrent = getTorrent(hash);

    if (torrent.is_valid()) {
        torrent.resume();
        torrent.auto_managed(true);
    } else {
        return false;
    }

    return true;
}

bool TorrentManager::removeTorrent(string hash) {
    libtorrent::torrent_handle torrent = getTorrent(hash);

    if (torrent.is_valid()) {
        m_session.remove_torrent(torrent);
        boost::filesystem::remove(m_torrentsPath + "/" + hash + ".torrent");
        boost::filesystem::remove(m_resumeDataPath + "/" + hash + ".fastresume");
    } else {
        return false;
    }

    return true;
}

void TorrentManager::loadFastResumeData(string hash, vector<char> & data) {
    string path = m_resumeDataPath + "/" + hash + ".fastresume";
    boost::system::error_code errorCode;

    cout << "try loadFastResumeData " << path << endl;

    try {
        if (boost::filesystem::file_size(path) > 0) {
            cout << "loadFastResumeData " << path << endl;

            ifstream stream(path.c_str(), ios::binary);
            stream.unsetf(ios::skipws);
            istream_iterator<char> start(stream), end;

            data.assign(start, end);
        }
    } catch (std::exception except) {
        cerr << "failed to load fastresume data" << endl;
    }
}

string TorrentManager::addTorrentFromFile(string path, bool resumeData) {
    cout << "loadTorrentFromFile " << path << endl;
    libtorrent::error_code errorCode;
    libtorrent::torrent_info * torrentInfo = new libtorrent::torrent_info(path, errorCode);
    libtorrent::sha1_hash hash = torrentInfo->info_hash();

    if (!hasTorrent(hash)) {
        string hex = libtorrent::to_hex(hash.to_string());

        if (!boost::filesystem::exists(m_torrentsPath + "/" + hex + ".torrent")) {
            boost::filesystem::copy_file(path, m_torrentsPath + "/" + hex + ".torrent");
        }

        libtorrent::add_torrent_params parameters;

        parameters.ti = boost::shared_ptr<libtorrent::torrent_info>(torrentInfo);
        parameters.save_path = m_filesPath;

        if (resumeData) {
            loadFastResumeData(hex, parameters.resume_data);
        }
        
        m_session.add_torrent(parameters, errorCode);
        
        return hex;
    } else {
        delete torrentInfo;
    }
    
    return string();
}

string TorrentManager::addTorrentFromMagnet(string uri) {
    cout << "loadTorrentFromMagnet " << uri << endl;

    libtorrent::error_code errorCode;
    libtorrent::add_torrent_params parameters;

    parameters.url = uri;
    parameters.save_path = m_filesPath;
    parameters.flags = libtorrent::add_torrent_params::default_flags | libtorrent::add_torrent_params::flag_duplicate_is_error;
    
    try {
        libtorrent::torrent_handle torrentHandle = m_session.add_torrent(parameters, errorCode);
        
        return libtorrent::to_hex(torrentHandle.info_hash().to_string());
    } catch (std::exception except) {
        
    }
    
    return string();
}

void TorrentManager::addTorrentsFromDirectory(string path) {
    for (boost::filesystem::directory_iterator iterator(path), end; iterator != end; ++iterator) {
        addTorrentFromFile(iterator->path().string(), true);
    }
}
