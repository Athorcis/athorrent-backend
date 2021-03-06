#include "TorrentManager.h"
#include "Utils.h"
#include "ResumeDataManager.h"
#include "AlertManager.h"

#include <boost/filesystem.hpp>
#include <libtorrent/bencode.hpp>
#include <libtorrent/create_torrent.hpp>
#include <libtorrent/torrent_info.hpp>
#include <libtorrent/settings_pack.hpp>
#include <libtorrent/error_code.hpp>

#include <fstream>
#include <iostream>

using namespace std;

namespace fs = boost::filesystem;
namespace lt = libtorrent;

TorrentManager::TorrentManager(string userId) :
    m_userId(userId),
    m_torrentsPath("cache/" + m_userId + "/torrents"),
    m_resumeDataPath("cache/" + m_userId + "/fastresume"),
    m_filesPath("files/" + m_userId)
{
    if (!fs::exists(m_torrentsPath)) {
        fs::create_directories(m_torrentsPath);
    }

    if (!fs::exists(m_resumeDataPath)) {
        fs::create_directories(m_resumeDataPath);
    }

    if (!fs::exists(m_filesPath)) {
        fs::create_directories(m_filesPath);
    }

    lt::settings_pack settings;
    settings.set_int(lt::settings_pack::alert_mask, lt::alert::status_notification  | lt::alert::storage_notification | lt::alert::error_notification);
    settings.set_int(lt::settings_pack::active_downloads, 12);
    settings.set_int(lt::settings_pack::active_seeds, 20);
    
    m_session.apply_settings(settings);

    m_resumeDataManager = new ResumeDataManager(*this);
    
    m_alertManager = new AlertManager(*this);
    m_alertManager->start();
    
    addTorrentsFromDirectory(m_torrentsPath);
}

lt::session & TorrentManager::getSession()
{
    return m_session;
}

AlertManager & TorrentManager::getAlertManager()
{
    return *m_alertManager;
}

ResumeDataManager & TorrentManager::getResumeDataManager()
{
    return *m_resumeDataManager;
}

const string & TorrentManager::getResumeDataPath() const
{
    return m_resumeDataPath;
}

void TorrentManager::createTorrent(lt::torrent_handle & handle)
{
    if (handle.is_valid()) {
        std::shared_ptr<const lt::torrent_info> info = handle.torrent_file();
        
        lt::create_torrent createTorrent(*info);
        lt::entry entry = createTorrent.generate();

        string path = m_torrentsPath + "/" + bin2hex(info->info_hash().to_string()) + ".torrent";
        writeBencodedTree(path, entry);
    }
}

void TorrentManager::writeBencodedTree(const string & path, const lt::entry & entry)
{
    ofstream file (path.c_str(), ofstream::out | ofstream::binary | ofstream::trunc);

    if (file.is_open()) {
        ostream_iterator<char> iterator(file);
        lt::bencode(iterator, entry);
    }
}

bool TorrentManager::hasTorrent(lt::sha1_hash hash) {
    return m_session.find_torrent(hash).is_valid();
}

bool TorrentManager::hasTorrent(string hash) {
    return hasTorrent(lt::sha1_hash(hex2bin(hash)));
}

lt::torrent_handle TorrentManager::getTorrent(lt::sha1_hash hash) {
    return m_session.find_torrent(hash);
}

lt::torrent_handle TorrentManager::getTorrent(string hash) {
    return getTorrent(lt::sha1_hash(hex2bin(hash)));
}

vector<lt::torrent_handle> TorrentManager::getTorrents() {
    return m_session.get_torrents();
}

bool TorrentManager::pauseTorrent(string hash) {
    lt::torrent_handle torrent = getTorrent(hash);

    if (torrent.is_valid()) {
        torrent.auto_managed(false);
        torrent.pause(lt::torrent_handle::graceful_pause);
    } else {
        return false;
    }

    return true;
}

bool TorrentManager::resumeTorrent(string hash) {
    lt::torrent_handle torrent = getTorrent(hash);

    if (torrent.is_valid() && (!torrent.is_seed() || torrent.get_torrent_info().priv())) {
        torrent.resume();
        torrent.auto_managed(true);
    } else {
        return false;
    }

    return true;
}

bool TorrentManager::removeTorrent(string hash) {
    lt::torrent_handle torrent = getTorrent(hash);

    if (torrent.is_valid()) {
        m_session.remove_torrent(torrent);
        fs::remove(m_torrentsPath + "/" + hash + ".torrent");
        fs::remove(m_resumeDataPath + "/" + hash + ".fastresume");
    } else {
        return false;
    }

    return true;
}

string TorrentManager::addTorrentFromFile(string path, bool resumeData) {
    cout << "loadTorrentFromFile " << path << endl;
    lt::error_code errorCode;
    lt::torrent_info * torrentInfo = new lt::torrent_info(path, errorCode);
    lt::sha1_hash hash = torrentInfo->info_hash();

    if (!hasTorrent(hash)) {
        string hex = bin2hex(hash.to_string());

        if (!fs::exists(m_torrentsPath + "/" + hex + ".torrent")) {
            fs::copy_file(path, m_torrentsPath + "/" + hex + ".torrent");
        }

        lt::add_torrent_params parameters;

        parameters.ti = std::shared_ptr<lt::torrent_info>(torrentInfo);
        parameters.save_path = m_filesPath;

        if (resumeData) {
            m_resumeDataManager->loadResumeData(hex, parameters.resume_data);
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

    lt::error_code errorCode;
    lt::add_torrent_params parameters;

    parameters.url = uri;
    parameters.save_path = m_filesPath;
    parameters.flags = lt::torrent_flags::default_flags | lt::add_torrent_params::flag_duplicate_is_error;
    
    try {
        lt::torrent_handle torrentHandle = m_session.add_torrent(parameters, errorCode);
        
        return bin2hex(torrentHandle.info_hash().to_string());
    } catch (std::exception except) {
        
    }
    
    return string();
}

void TorrentManager::addTorrentsFromDirectory(string path) {
    for (fs::directory_iterator iterator(path), end; iterator != end; ++iterator) {
        addTorrentFromFile(iterator->path().string(), true);
    }
}
