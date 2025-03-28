#include "TorrentManager.h"
#include "ResumeDataManager.h"
#include "AlertManager.h"
#include "JsonRequestFailedException.h"

#include <boost/algorithm/hex.hpp>
#include <boost/filesystem.hpp>
#include <libtorrent/bencode.hpp>
#include <libtorrent/create_torrent.hpp>
#include <libtorrent/torrent_info.hpp>
#include <libtorrent/settings_pack.hpp>
#include <libtorrent/magnet_uri.hpp>
#include <libtorrent/error_code.hpp>

#include <fstream>
#include <iostream>

using namespace std;

namespace fs = boost::filesystem;
namespace lt = libtorrent;

TorrentManager::TorrentManager(const string & port)
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
    settings.set_bool(lt::settings_pack::enable_lsd, false);
    settings.set_bool(lt::settings_pack::enable_upnp, false);
    settings.set_bool(lt::settings_pack::enable_natpmp, false);
    settings.set_str(lt::settings_pack::listen_interfaces, std::string("0.0.0.0:") + port + ",[::]:" + port);

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

void TorrentManager::createTorrent(const lt::torrent_handle & handle)
{
    if (handle.is_valid()) {
        std::shared_ptr<const lt::torrent_info> info = handle.torrent_file();
        
        lt::create_torrent createTorrent(*info);
        lt::entry entry = createTorrent.generate();

        string path = m_torrentsPath + "/" + boost::algorithm::hex(info->info_hash().to_string()) + ".torrent";
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

bool TorrentManager::hasTorrent(lt::sha1_hash hash) const {
    return m_session.find_torrent(hash).is_valid();
}

bool TorrentManager::hasTorrent(const string & hash) const {
    return hasTorrent(lt::sha1_hash(boost::algorithm::unhex(hash).c_str()));
}

lt::torrent_handle TorrentManager::getTorrent(lt::sha1_hash hash) const {
    return m_session.find_torrent(hash);
}

lt::torrent_handle TorrentManager::getTorrent(const string & hash) const {
    return getTorrent(lt::sha1_hash(boost::algorithm::unhex(hash).c_str()));
}

vector<lt::torrent_handle> TorrentManager::getTorrents() {
    return m_session.get_torrents();
}

bool TorrentManager::pauseTorrent(const string & hash) {
    lt::torrent_handle torrent = getTorrent(hash);

    if (torrent.is_valid()) {
        torrent.unset_flags(lt::torrent_flags::auto_managed);
        torrent.pause(lt::torrent_handle::graceful_pause);
    } else {
        return false;
    }

    return true;
}

bool TorrentManager::resumeTorrent(const string & hash) {
    lt::torrent_handle torrent = getTorrent(hash);

    libtorrent::torrent_status status = torrent.status(libtorrent::torrent_handle::query_name);

    if (torrent.is_valid() && (status.state != libtorrent::torrent_status::seeding || torrent.torrent_file()->priv())) {
        torrent.resume();
        torrent.set_flags(lt::torrent_flags::auto_managed);
    } else {
        return false;
    }

    return true;
}

bool TorrentManager::removeTorrent(const string & hash) {
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

string TorrentManager::addTorrentFromFile(const string & path, bool resumeData, bool * added) {
    cout << "loadTorrentFromFile " << path << endl;
    lt::error_code errorCode;
    auto * torrentInfo = new lt::torrent_info(path, errorCode);

    if (errorCode.failed()) {
        cout << "INVALID_TORRENT_FILE: " << errorCode.message() << endl;
        throw JsonRequestFailedException("INVALID_TORRENT_FILE", errorCode.message());
    }

    lt::sha1_hash hash = torrentInfo->info_hash();
    string hex = boost::algorithm::hex(hash.to_string());

    if (hasTorrent(hash)) {
        delete torrentInfo;

        if (added) {
            *added = false;
        }

        return hex;
    }

    bool resumeDataLoaded = false;

    if (resumeData) {
        lt::add_torrent_params parameters = m_resumeDataManager->loadResumeData(hex, resumeDataLoaded);

        if (resumeDataLoaded) {
            m_session.async_add_torrent(std::move(parameters));
        }
    }

    if (!resumeDataLoaded) {
        if (!fs::exists(m_torrentsPath + "/" + hex + ".torrent")) {
            fs::copy_file(path, m_torrentsPath + "/" + hex + ".torrent");
        }

        lt::add_torrent_params parameters;

        parameters.ti = std::shared_ptr<lt::torrent_info>(torrentInfo);
        parameters.save_path = m_filesPath;

        m_session.async_add_torrent(std::move(parameters));
    }

    if (added) {
        *added = true;
    }

    return hex;
}

string TorrentManager::addTorrentFromMagnet(const string & uri, bool * added) {
    cout << "loadTorrentFromMagnet " << uri << endl;

    lt::error_code errorCode;
    lt::add_torrent_params parameters = lt::parse_magnet_uri(uri, errorCode);

    if (errorCode.failed()) {
        cout << "INVALID_MAGNET_URI: " << errorCode.message() << endl;
        throw JsonRequestFailedException("INVALID_MAGNET_URI", errorCode.message());
    }

    lt::sha1_hash hash = parameters.info_hashes.v1;
    string hex = boost::algorithm::hex(hash.to_string());

    if (hasTorrent(hash)) {

        if (added) {
            *added = false;
        }

        return hex;
    }

    parameters.save_path = m_filesPath;
    parameters.flags |= lt::torrent_flags::duplicate_is_error;
    
    try {
        lt::torrent_handle torrentHandle = m_session.add_torrent(parameters, errorCode);

        if (added) {
            *added = true;
        }

        return hex;
    } catch (const std::exception & except) {
        cout << "FAILED_TO_ADD_MAGNET: " << except.what() << endl;
        throw JsonRequestFailedException("FAILED_TO_ADD_MAGNET", except.what());
    }
}

void TorrentManager::addTorrentsFromDirectory(const string & path) {
    for (fs::directory_iterator iterator(path), end; iterator != end; ++iterator) {
        addTorrentFromFile(iterator->path().string(), true);
    }
}
