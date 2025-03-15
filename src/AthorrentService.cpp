#include "AthorrentService.h"

#include <iostream>
#include <fstream>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/hex.hpp>
#include <libtorrent/announce_entry.hpp>
#include <libtorrent/torrent_info.hpp>
#include <libtorrent/torrent_status.hpp>
#include <rapidjson/document.h>

using namespace rapidjson;

AthorrentService::AthorrentService(const std::string & address, TorrentManager * torrentManager) : JsonServer(address), m_torrentManager(torrentManager) {
}

JsonResponse * AthorrentService::handleRequest(const JsonRequest * request) {
    auto * response = new JsonResponse();
    Document::AllocatorType & allocator = response->getAllocator();
    
    const std::string & action = request->getAction();

    //std::cout << "REQUEST: " << action << std::endl;

    if (action == "ping") {
        response->setSuccess("pong");
    } else if (action == "addTorrentFromFile") {
        if (request->hasParameter("file")) {
            std::string file = request->getParameter("file");
            std::string cwd(boost::filesystem::current_path().string());

            boost::replace_first(file, "<workdir>", cwd);

            if (!boost::filesystem::exists(file)) {
                throw JsonRequestFailedException("TORRENT_FILE_NOT_FOUND");
            }

            bool added;
            std::string hash = m_torrentManager->addTorrentFromFile(file, false, &added);

            Value & data = response->getData();
            data.SetObject();

            data.AddMember("hash", Value(hash, allocator).Move(), allocator);
            data.AddMember("added", added, allocator);
            response->setStatus("success");
        }
    }
    else if (action == "addTorrentFromMagnet") {
        if (request->hasParameter("magnet")) {
            std::string magnet = request->getParameter("magnet");
            bool added;
            std::string hash = m_torrentManager->addTorrentFromMagnet(magnet, &added);

            Value & data = response->getData();
            data.SetObject();

            data.AddMember("hash", Value(hash, allocator).Move(), allocator);
            data.AddMember("added", added, allocator);

            response->setStatus("success");
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

        for (const libtorrent::torrent_handle& torrent : torrents) {
            libtorrent::torrent_status status = torrent.status(libtorrent::torrent_handle::query_accurate_download_counters | libtorrent::torrent_handle::query_name);

            Value torrentVal;
            torrentVal.SetObject();

            torrentVal.AddMember("name", Value(status.name, allocator).Move(), allocator);

            if (torrent.flags() & lt::torrent_flags::paused) {
                if (status.state == libtorrent::torrent_status::seeding && !torrent.torrent_file()->priv()) {
                    torrentVal.AddMember("state", "disabled", allocator);
                } else {
                    torrentVal.AddMember("state", "paused", allocator);
                }
            } else if (status.state == libtorrent::torrent_status::checking_files) {
                torrentVal.AddMember("state", "checking_files", allocator);
            } else if (status.state == libtorrent::torrent_status::downloading_metadata) {
                torrentVal.AddMember("state", "downloading_metadata", allocator);
            } else if (status.state == libtorrent::torrent_status::downloading) {
                torrentVal.AddMember("state", "downloading", allocator);
            } else if (status.state == libtorrent::torrent_status::finished) {
                torrentVal.AddMember("state", "finished", allocator);
            } else if (status.state == libtorrent::torrent_status::seeding) {
                torrentVal.AddMember("state", "seeding", allocator);
            } else if (status.state == libtorrent::torrent_status::checking_resume_data) {
                torrentVal.AddMember("state", "checking_resume_data", allocator);
            }
            else {
                torrentVal.AddMember("state", "unknown_state", allocator);
            }

            torrentVal.AddMember("paused", !!(torrent.flags() & lt::torrent_flags::paused), allocator);
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
            torrentVal.AddMember("hash", Value(boost::algorithm::hex(status.info_hashes.get_best().to_string()), allocator).Move(), allocator);

            data.PushBack(torrentVal, allocator);
        }
        
        response->setStatus("success");
    } else if (action == "getPaths") {
        Value & data = response->getData();
        data.SetArray();

        std::string cwd(boost::filesystem::current_path().string());
        std::vector<libtorrent::torrent_handle> torrents = m_torrentManager->getTorrents();

        for (const libtorrent::torrent_handle & torrent : torrents) {
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
                std::vector<libtorrent::announce_entry> trackers = torrent_handle.trackers();

                libtorrent::info_hash_t hashes = torrent_handle.info_hashes();

                bool hasV1 = hashes.has_v1();
                bool hasV2 = hashes.has_v2();

                for (const libtorrent::announce_entry& tracker : trackers) {
                    std::vector<libtorrent::announce_endpoint> endpoints = tracker.endpoints;
                    libtorrent::announce_endpoint bestEndpoint;

                    bool validEndpoint = false;
                    bool useV2 = false;

                    for (const libtorrent::announce_endpoint& endpoint : endpoints) {
                        if (hasV1) {
                            const libtorrent::announce_infohash & announceInfohash = endpoint.info_hashes.at(0);

                            if (!announceInfohash.last_error.failed()) {
                                bestEndpoint = endpoint;
                                validEndpoint = true;
                                break;
                            }
                        }

                        if (hasV2) {
                            const libtorrent::announce_infohash & announceInfohash = endpoint.info_hashes.at(1);

                            if (!announceInfohash.last_error.failed()) {
                                bestEndpoint = endpoint;
                                validEndpoint = true;
                                useV2 = true;
                               break;
                            }
                        }
                    }

                    bool hasEndpoints = true;

                    if (!validEndpoint) {
                        if (endpoints.empty()) {
                            hasEndpoints = false;
                            std::cout << "no endpoint found for tracker " << tracker.url << std::endl;
                        }
                        else {
                            bestEndpoint = endpoints.back();
                        }
                    }

                    Value trackerVal;
                    trackerVal.SetObject();

                    trackerVal.AddMember("id", Value(tracker.trackerid, allocator).Move(), allocator);
                    trackerVal.AddMember("url", Value(tracker.url, allocator).Move(), allocator);

                    if (hasEndpoints) {
                        const libtorrent::announce_infohash & announceInfohash = bestEndpoint.info_hashes.at(useV2 ? 1 : 0);

                        trackerVal.AddMember("peers", announceInfohash.scrape_incomplete, allocator);
                        trackerVal.AddMember("seeds", announceInfohash.scrape_complete, allocator);
                        trackerVal.AddMember("message", Value(announceInfohash.message, allocator).Move(), allocator);

                        std::string state;

                        if (announceInfohash.updating) {
                            state = "Updating";
                        }
                        else if (announceInfohash.fails > 0) {
                            boost::system::error_code lastError = announceInfohash.last_error;

                            if (lastError.value() == libtorrent::errors::error_code_enum::tracker_failure) {
                                state = "TrackerError";
                            }
                            else if (lastError.value() == libtorrent::errors::error_code_enum::announce_skipped) {
                                state = "Unreachable";
                            }
                            else {
                                state = "NotWorking";
                            }

                            trackerVal.AddMember("error", Value(lastError.message(), allocator).Move(), allocator);
                        }
                        else if (tracker.verified) {
                            state = "Working";
                        }
                        else {
                            state = "NotContacted";
                        }

                        trackerVal.AddMember("state", Value(state, allocator), allocator);
                    }
                    else {
                        trackerVal.AddMember("peers", -1, allocator);
                        trackerVal.AddMember("seeds", -1, allocator);
                        trackerVal.AddMember("message", "", allocator);
                        trackerVal.AddMember("state", Value(tracker.verified ? "Working" : "NotContacted", allocator), allocator);
                    }

                    data.PushBack(trackerVal, allocator);
                }
            }

            response->setStatus("success");
        }
    } else {
        std::cerr << "something went wrong [" << action << "]" << std::endl;
    }

    return response;
}
