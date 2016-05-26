    #include "AthorrentService.h"

#include <iostream>
#include <fstream>
#include <boost/filesystem.hpp>
#include "Utils.h"
#include <libtorrent/announce_entry.hpp>
#include <libtorrent/torrent_info.hpp>
#include <libtorrent/torrent_status.hpp>
#include <rapidjson/document.h>

using namespace rapidjson;

AthorrentService::AthorrentService(const std::string & userId, TorrentManager * torrentManager) : JsonServer(AthorrentService::getPath(userId)), m_torrentManager(torrentManager) {
    m_flagDir = "flags/" + userId;

    if (!boost::filesystem::exists(m_flagDir)) {
        boost::filesystem::create_directories(m_flagDir);
    }
}

void AthorrentService::run() {
    setFlag("running");
    JsonServer::run();
}

void AthorrentService::stop() {
    JsonServer::stop();
    resetFlag("running");
}

JsonResponse * AthorrentService::handleRequest(const JsonRequest * request) {
    JsonResponse * response = new JsonResponse();
    Document::AllocatorType & allocator = response->getAllocator();
    
    const std::string & action = request->getAction();

    std::cout << "REQUEST: " << action << std::endl;

    if (action == "ping") {
        response->setSuccess("pong");
    } else if (action == "addTorrentFromFile") {
        if (request->hasParameter("file")) {
            std::string file = request->getParameter("file");

            if (boost::filesystem::exists(file)) {
                m_torrentManager->addTorrentFromFile(file);
                response->setSuccess("torrent added");
            }
        }
    } else if (action == "addTorrentFromMagnet") {
        if (request->hasParameter("magnet")) {
            std::string magnet = request->getParameter("magnet");

            m_torrentManager->addTorrentFromMagnet(magnet);
            response->setSuccess("magnet added");
        }
    } else if (action == "pauseTorrent") {
        std::cout << "BP1" << std::endl;

        if (request->hasParameter("hash")) {
            std::cout << "hash: " << request->getParameter("hash") << std::endl;

            if (m_torrentManager->pauseTorrent(request->getParameter("hash"))) {
                response->setSuccess("torrent paused");
            }
        } else {
            std::cout << "BP2" << std::endl;

        }
    } else if (action == "resumeTorrent") {
        if (request->hasParameter("hash")) {
            m_torrentManager->resumeTorrent(request->getParameter("hash"));
            response->setSuccess("torrent resumed");
        }
    } else if (action == "removeTorrent") {
        if (request->hasParameter("hash")) {
            m_torrentManager->removeTorrent(request->getParameter("hash"));
            response->setSuccess("torrent removed");
        }
    } else if (action == "getTorrents") {
        Value & data = response->getData();
        data.SetArray();
        
        std::vector<libtorrent::torrent_handle> torrents = m_torrentManager->getTorrents();

        for (libtorrent::torrent_handle torrent : torrents) {
            libtorrent::torrent_status status = torrent.status(libtorrent::torrent_handle::query_accurate_download_counters | libtorrent::torrent_handle::query_name);

            Value torrentVal;
            torrentVal.SetObject();

            torrentVal.AddMember("name", Value(Utils::fromUtf8(status.name), allocator).Move(), allocator);

            if (status.paused) {
                torrentVal.AddMember("state", "paused", allocator);
            } else if (status.state == libtorrent::torrent_status::queued_for_checking) {
                torrentVal.AddMember("state", "queued_for_checking", allocator);
            } else if (status.state == libtorrent::torrent_status::checking_files) {
                torrentVal.AddMember("state", "cheking_files", allocator);
            } else if (status.state == libtorrent::torrent_status::downloading_metadata) {
                torrentVal.AddMember("state", "downloading_metadata", allocator);
            } else if (status.state == libtorrent::torrent_status::downloading) {
                torrentVal.AddMember("state", "downloading", allocator);
            } else if (status.state == libtorrent::torrent_status::finished) {
                torrentVal.AddMember("state", "finished", allocator);
            } else if (status.state == libtorrent::torrent_status::seeding) {
                torrentVal.AddMember("state", "seeding", allocator);
            } else if (status.state == libtorrent::torrent_status::allocating) {
                torrentVal.AddMember("state", "allocating", allocator);
            } else if (status.state == libtorrent::torrent_status::checking_resume_data) {
                torrentVal.AddMember("state", "checking_resume_data", allocator);
            }

            torrentVal.AddMember("paused", status.paused, allocator);
            torrentVal.AddMember("total_payload_download", Value(std::to_string(status.total_payload_download), allocator).Move(), allocator);
            torrentVal.AddMember("total_payload_upload", Value(std::to_string(status.total_payload_upload), allocator).Move(), allocator);
            torrentVal.AddMember("size", Value(std::to_string(status.total_wanted), allocator).Move(), allocator);
            torrentVal.AddMember("progress", status.progress, allocator);
            torrentVal.AddMember("download_rate", status.download_rate, allocator);
            torrentVal.AddMember("download_payload_rate", status.download_payload_rate, allocator);
            torrentVal.AddMember("upload_rate", status.upload_rate, allocator);
            torrentVal.AddMember("upload_payload_rate", status.upload_payload_rate, allocator);
            torrentVal.AddMember("num_seeds", status.num_seeds, allocator);
            torrentVal.AddMember("num_peers", status.num_peers, allocator);
            torrentVal.AddMember("num_complete", status.num_peers, allocator);
            torrentVal.AddMember("num_incomplete", status.num_incomplete, allocator);
            torrentVal.AddMember("list_seeds", status.list_seeds, allocator);
            torrentVal.AddMember("list_peers", status.list_peers, allocator);
            torrentVal.AddMember("hash", Value(libtorrent::to_hex(status.info_hash.to_string()), allocator).Move(), allocator);

            data.PushBack(torrentVal, allocator);
        }
        
        response->setStatus("success");
    } else if (action == "getPaths") {
        Value & data = response->getData();
        data.SetArray();

        std::vector<libtorrent::torrent_handle> torrents = m_torrentManager->getTorrents();

        for (libtorrent::torrent_handle torrent : torrents) {
            libtorrent::torrent_status status = torrent.status(libtorrent::torrent_handle::query_save_path | libtorrent::torrent_handle::query_name);

            data.PushBack(Value(status.save_path + '/' + Utils::fromUtf8(status.name), allocator).Move(), allocator);
        }
        
        response->setStatus("success");
    } else if (action == "listTrackers") {
        if (request->hasParameter("hash")) {
            Value & data = response->getData();
            data.SetArray();

            libtorrent::torrent_handle torrent_handle = m_torrentManager->getTorrent(request->getParameter("hash"));

            if (torrent_handle.is_valid()) {
                libtorrent::torrent_info torrent_info = torrent_handle.get_torrent_info();
                std::vector<libtorrent::announce_entry> trackers = torrent_info.trackers();

                for (libtorrent::announce_entry tracker : trackers) {
                    Value trackerVal;
                    trackerVal.SetObject();
                    
                    trackerVal.AddMember("id", Value(tracker.trackerid, allocator).Move(), allocator);
                    trackerVal.AddMember("url", Value(tracker.url, allocator).Move(), allocator);
                    trackerVal.AddMember("peers", tracker.scrape_complete + tracker.scrape_incomplete, allocator);
                    trackerVal.AddMember("message", Value(tracker.message, allocator).Move(), allocator);

                    data.PushBack(trackerVal, allocator);
                }
            }

            response->setStatus("success");
        }
    } else {
        std::cerr << "something went wrong [" << action << "]" << std::endl;
    }

//    } else if (action == "saveTorrents") {
        // vector<libtorrent::torrent_handle> torrents = m_torrentManager->getTorrents();

        // int counter = 0;
        // for (libtorrent::torrent_handle torrent : torrents) {
            // libtorrent::torrent_status status = torrent.status();

            // torrent.save_resume_data();
            // ++counter;
        // }


        // while (counter) {
            // libtorrent::alert const* a = session->wait_for_alert(libtorrent::seconds(10));

            // if we don't get an alert within 10 seconds, abort
            // if (a == 0) break;

            // auto_ptr<libtorrent::alert> holder = session->pop_alert();

            // if (libtorrent::alert_cast<libtorrent::save_resume_data_failed_alert>(a))
            // {
                // cout << "error: save_resume_data_failed_alert" << endl;
                    // libtorrent::process_alert(a);
                    // --counter;
                    // continue;
            // }

            // libtorrent::save_resume_data_alert const* rd = libtorrent::alert_cast<libtorrent::save_resume_data_alert>(a);
            // if (rd == 0)
            // {
                // cout << "error:  not the right type" << endl;
                    // libtorrent::process_alert(a);
                    // continue;
            // }
// cout << "log: saving the resume" << endl;
            // libtorrent::torrent_handle h = rd->handle;
            // libtorrent::torrent_status st = h.status(libtorrent::torrent_handle::query_save_path | libtorrent::torrent_handle::query_name);

            // cout << rd->resume_data->to_string() << endl;

            // saveFastResumeData(libtorrent::to_hex(h.info_hash().to_string()), *rd->resume_data);

            // --counter;
        // }
//    }

    return response;
}

void AthorrentService::setFlag(const std::string & flag) {
    std::ofstream file(m_flagDir + '/' + flag);
    file.close();
}

void AthorrentService::resetFlag(const std::string & flag) {
    boost::filesystem::remove(m_flagDir + '/' + flag);
}

std::string AthorrentService::getPath(const std::string & userId) {
#ifdef _WIN32
    std::string path = "\\\\.\\pipe\\athorrentd\\sockets\\" + userId + ".sck";
#elif defined __linux__
    std::string path = "sockets/" + userId + ".sck";
#endif

    return path;
}
