#include "AthorrentService.h"
#include "Utils.h"

#include <iostream>
#include <fstream>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <libtorrent/announce_entry.hpp>
#include <libtorrent/torrent_info.hpp>
#include <libtorrent/torrent_status.hpp>
#include <rapidjson/document.h>

using namespace rapidjson;

AthorrentService::AthorrentService(const std::string & address, TorrentManager * torrentManager) : JsonServer(address), m_torrentManager(torrentManager) {
}

void AthorrentService::run() {
    JsonServer::run();
}

void AthorrentService::stop() {
    JsonServer::stop();
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
            std::string cwd(boost::filesystem::current_path().string());

            boost::replace_first(file, "<workdir>", cwd);

            if (boost::filesystem::exists(file)) {
                std::string hash = m_torrentManager->addTorrentFromFile(file);
                
                if (!hash.empty()) {
                    Value & data = response->getData();
                    data.SetObject();
                    
                    data.AddMember("hash", Value(hash, allocator).Move(), allocator);
                    response->setStatus("success");
                }
            }
        }
    } else if (action == "addTorrentFromMagnet") {
        if (request->hasParameter("magnet")) {
            std::string magnet = request->getParameter("magnet");
            std::string hash = m_torrentManager->addTorrentFromMagnet(magnet);
            
            if (!hash.empty()) {
                Value & data = response->getData();
                data.SetObject();

                data.AddMember("hash", Value(hash, allocator).Move(), allocator);
                response->setStatus("success");
            }
        }
    } else if (action == "pauseTorrent") {
        if (request->hasParameter("hash")) {
            if (m_torrentManager->pauseTorrent(request->getParameter("hash"))) {
                response->setSuccess("torrent paused");
            }
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

            torrentVal.AddMember("name", Value(status.name, allocator).Move(), allocator);

            if (status.paused) {
                if (torrent.is_seed() && !torrent.get_torrent_info().priv()) {
                    torrentVal.AddMember("state", "disabled", allocator);
                } else {
                    torrentVal.AddMember("state", "paused", allocator);
                }
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
            torrentVal.AddMember("hash", Value(bin2hex(status.info_hash.to_string()), allocator).Move(), allocator);

            data.PushBack(torrentVal, allocator);
        }
        
        response->setStatus("success");
    } else if (action == "getPaths") {
        Value & data = response->getData();
        data.SetArray();

        std::string cwd(boost::filesystem::current_path().string());
        std::vector<libtorrent::torrent_handle> torrents = m_torrentManager->getTorrents();

        for (libtorrent::torrent_handle torrent : torrents) {
            libtorrent::torrent_status status = torrent.status(libtorrent::torrent_handle::query_save_path | libtorrent::torrent_handle::query_name);

            std::string torrentPath = status.save_path + '/' + status.name;

            boost::replace_first(torrentPath, cwd, "<workdir>");
            data.PushBack(Value(torrentPath, allocator).Move(), allocator);
        }
        
        response->setStatus("success");
    } else if (action == "listTrackers") {
        if (request->hasParameter("hash")) {
            Value & data = response->getData();
            data.SetArray();

            libtorrent::torrent_handle torrent_handle = m_torrentManager->getTorrent(request->getParameter("hash"));

            if (torrent_handle.is_valid()) {
                std::shared_ptr<const libtorrent::torrent_info> torrent_info = torrent_handle.torrent_file();
                std::vector<libtorrent::announce_entry> trackers = torrent_info->trackers();

                for (libtorrent::announce_entry tracker : trackers) {
                    
                    for (libtorrent::announce_endpoint endpoint : tracker.endpoints) {
                        Value trackerVal;
                        trackerVal.SetObject();
                        
                        trackerVal.AddMember("id", Value(tracker.trackerid, allocator).Move(), allocator);
                        trackerVal.AddMember("url", Value(tracker.url, allocator).Move(), allocator);
                        trackerVal.AddMember("peers", endpoint.scrape_complete + endpoint.scrape_incomplete, allocator);
                        trackerVal.AddMember("message", Value(endpoint.message, allocator).Move(), allocator);

                        data.PushBack(trackerVal, allocator);
                    }
                }
            }

            response->setStatus("success");
        }
    } else {
        std::cerr << "something went wrong [" << action << "]" << std::endl;
    }

    return response;
}
