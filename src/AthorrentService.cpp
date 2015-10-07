#include "AthorrentService.h"

#include <iostream>
#include <fstream>
#include <boost/filesystem.hpp>
#include "Utils.h"

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
    JsonResponse * response = NULL;
    const std::string & action = request->getAction();
    
    std::cout << "REQUEST: " << action << std::endl;
    
    if (action == "ping") {
        response = new JsonResponse("pong", true);
    } else if (action == "addTorrentFromFile") {
        if (request->hasParameter("file")) {
            std::string file = request->getParameter("file");
            
            if (boost::filesystem::exists(file)) {
                m_torrentManager->addTorrentFromFile(file);
                response = new JsonResponse("torrent added", true);
            }
        }
    } else if (action == "addTorrentFromMagnet") {
        if (request->hasParameter("magnet")) {
            std::string magnet = request->getParameter("magnet");
            
            m_torrentManager->addTorrentFromMagnet(magnet);
            response = new JsonResponse("magnet added", true);
        }
    } else if (action == "pauseTorrent") {
        if (request->hasParameter("hash")) {
            if (m_torrentManager->pauseTorrent(request->getParameter("hash"))) {
                response = new JsonResponse("torrent paused", true);
            }
        }
    } else if (action == "resumeTorrent") {
        if (request->hasParameter("hash")) {
            m_torrentManager->resumeTorrent(request->getParameter("hash"));
            response = new JsonResponse("torrent resumed", true);
        }
    } else if (action == "removeTorrent") {
        if (request->hasParameter("hash")) {
            m_torrentManager->removeTorrent(request->getParameter("hash"));
            response = new JsonResponse("torrent removed", true);
        }
    } else if (action == "getTorrents") {
        JSONNODE * resultNode = json_new(JSON_ARRAY);
        
        std::vector<libtorrent::torrent_handle> torrents = m_torrentManager->getTorrents();
        
        for (libtorrent::torrent_handle torrent : torrents) {
            libtorrent::torrent_status status = torrent.status(libtorrent::torrent_handle::query_accurate_download_counters | libtorrent::torrent_handle::query_name);
            
            JSONNODE * torrentNode = json_new(JSON_NODE);
            
            json_push_back(torrentNode, json_new_a("name", Utils::fromUtf8(status.name).c_str()));
            
            if (status.paused) {
                json_push_back(torrentNode, json_new_a("state", "paused"));
            } else if (status.state == libtorrent::torrent_status::queued_for_checking) {
                json_push_back(torrentNode, json_new_a("state", "queued_for_checking"));
            } else if (status.state == libtorrent::torrent_status::checking_files) {
                json_push_back(torrentNode, json_new_a("state", "checking_files"));
            } else if (status.state == libtorrent::torrent_status::downloading_metadata) {
                json_push_back(torrentNode, json_new_a("state", "downloading_metadata"));
            } else if (status.state == libtorrent::torrent_status::downloading) {
                json_push_back(torrentNode, json_new_a("state", "downloading"));
            } else if (status.state == libtorrent::torrent_status::finished) {
                json_push_back(torrentNode, json_new_a("state", "finished"));
            } else if (status.state == libtorrent::torrent_status::seeding) {
                json_push_back(torrentNode, json_new_a("state", "seeding"));
            } else if (status.state == libtorrent::torrent_status::allocating) {
                json_push_back(torrentNode, json_new_a("state", "allocating"));
            } else if (status.state == libtorrent::torrent_status::checking_resume_data) {
                json_push_back(torrentNode, json_new_a("state", "checking_resume_data"));
            }
            
            json_push_back(torrentNode, json_new_b("paused", status.paused));
            json_push_back(torrentNode, json_new_a("total_payload_download", std::to_string(status.total_payload_download).c_str()));
            json_push_back(torrentNode, json_new_a("total_payload_upload", std::to_string(status.total_payload_upload).c_str()));
            json_push_back(torrentNode, json_new_a("size", std::to_string(status.total_wanted).c_str()));
            json_push_back(torrentNode, json_new_f("progress", status.progress));
            json_push_back(torrentNode, json_new_i("download_rate", status.download_rate));
            json_push_back(torrentNode, json_new_i("download_payload_rate", status.download_payload_rate));
            json_push_back(torrentNode, json_new_i("upload_rate", status.upload_rate));
            json_push_back(torrentNode, json_new_i("upload_payload_rate", status.upload_payload_rate));
            json_push_back(torrentNode, json_new_i("num_seeds", status.num_seeds));
            json_push_back(torrentNode, json_new_i("num_peers", status.num_peers));
            json_push_back(torrentNode, json_new_i("num_complete", status.num_complete));
            json_push_back(torrentNode, json_new_i("num_incomplete", status.num_incomplete));
            json_push_back(torrentNode, json_new_i("list_seeds", status.list_seeds));
            json_push_back(torrentNode, json_new_i("list_peers", status.list_peers));
            json_push_back(torrentNode, json_new_a("hash", libtorrent::to_hex(status.info_hash.to_string()).c_str()));
            
            json_push_back(resultNode, torrentNode);
        }
        
        response = new JsonResponse(resultNode);
    } else if (action == "getPaths") {
        JSONNODE * resultNode = json_new(JSON_ARRAY);
        
        std::vector<libtorrent::torrent_handle> torrents = m_torrentManager->getTorrents();
        
        for (libtorrent::torrent_handle torrent : torrents) {
            libtorrent::torrent_status status = torrent.status(libtorrent::torrent_handle::query_save_path | libtorrent::torrent_handle::query_name);
            json_push_back(resultNode, json_new_a("", (status.save_path + '/' + Utils::fromUtf8(status.name)).c_str()));
        }
        
        response = new JsonResponse(resultNode);
    } else if (action == "listTrackers") {
        if (request->hasParameter("hash")) {
            JSONNODE * resultNode = json_new(JSON_ARRAY);
            
            libtorrent::torrent_handle torrent_handle = m_torrentManager->getTorrent(request->getParameter("hash"));
            
            if (torrent_handle.is_valid()) {
                libtorrent::torrent_info torrent_info = torrent_handle.get_torrent_info();
                std::vector<libtorrent::announce_entry> trackers = torrent_info.trackers();
                
                for (libtorrent::announce_entry tracker : trackers) {
                    JSONNODE * trackerNode = json_new(JSON_NODE);
                
                    json_push_back(trackerNode, json_new_a("id", tracker.trackerid.c_str()));
                    json_push_back(trackerNode, json_new_a("url", tracker.url.c_str()));
                    json_push_back(trackerNode, json_new_i("peers", tracker.scrape_complete + tracker.scrape_incomplete));
                    json_push_back(trackerNode, json_new_a("message", tracker.message.c_str()));
                    
                    json_push_back(resultNode, trackerNode);
                }
                
                response = new JsonResponse(resultNode);
            }
        }
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