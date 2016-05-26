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


#include <fstream>

using namespace std;

TorrentManager::TorrentManager(string userId) :
    m_userId(userId),
    m_torrentsPath("torrents/" + m_userId),
    m_filesPath("files/" + m_userId) {
    if (!boost::filesystem::exists(m_torrentsPath)) {
        boost::filesystem::create_directory(m_torrentsPath);
    }

    if (!boost::filesystem::exists(m_filesPath)) {
        boost::filesystem::create_directory(m_filesPath);
    }

    addTorrentsFromDirectory(m_torrentsPath);

    libtorrent::settings_pack settings;
    settings.set_str(libtorrent::settings_pack::listen_interfaces, "0.0.0.0:6881");
    settings.set_int(libtorrent::settings_pack::alert_mask, libtorrent::alert::status_notification);

    m_session.apply_settings(settings);

    boost::thread thread(boost::bind(&TorrentManager::eventLoop, this));
}

void TorrentManager::eventLoop() {
    libtorrent::time_duration timeout = libtorrent::seconds(60);

    while (m_session.wait_for_alert(timeout)) {
        std::vector<libtorrent::alert *> alerts;
        
        m_session.pop_alerts(&alerts);
        
        for (auto alert : alerts) {
            switch (alert->type()) {
                case libtorrent::metadata_received_alert::alert_type:
                    libtorrent::torrent_handle torrent = libtorrent::alert_cast<libtorrent::metadata_received_alert>(alert)->handle;

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
    } else {
        return false;
    }

    return true;
}

void TorrentManager::loadFastResumeData(string hash, vector<char> & data) {
    string path = "cache/states/" + hash + ".fastresume";
    boost::system::error_code errorCode;

    cout << "try loadFastResumeData " << path << endl;

    if (boost::filesystem::file_size(path, errorCode) > 0) {
        cout << "loadFastResumeData " << path << endl;

        ifstream stream(path.c_str(), ios::binary);
        stream.unsetf(ios::skipws);
        istream_iterator<char> start(stream), end;

        data.assign(start, end);
    }
}

void TorrentManager::saveFastResumeData(string hash, libtorrent::entry & entry) {
    string path = "cache/states/" + hash + ".fastresume";

    cout << "saveFastResumeData " << path << endl;

    ofstream stream(path.c_str(), ios::binary);
    stream.unsetf(ios::skipws);
    ostream_iterator<char> start(stream);

    libtorrent::bencode(start, entry);
}

void TorrentManager::addTorrentFromFile(string path) {
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

        // loadFastResumeData(hex, parameters.resume_data);

        m_session.add_torrent(parameters, errorCode);
    } else {
        delete torrentInfo;
    }
}

void TorrentManager::addTorrentFromMagnet(string uri) {
    cout << "loadTorrentFromMagnet " << uri << endl;

    libtorrent::error_code errorCode;
    libtorrent::add_torrent_params parameters;

    parameters.url = uri;
    parameters.save_path = m_filesPath;

    libtorrent::torrent_handle torrentHandle = m_session.add_torrent(parameters, errorCode);
}

void TorrentManager::addTorrentsFromDirectory(string path) {
    for (boost::filesystem::directory_iterator iterator(path), end; iterator != end; ++iterator) {
        addTorrentFromFile(iterator->path().string());
    }
}
