#ifndef ALERT_MANAGER_H
#define ALERT_MANAGER_H

#include "TorrentManager.h"

#include <libtorrent/alert_types.hpp>
#include <libtorrent/session.hpp>
#include <string>

class AlertManager
{
    public:
        explicit AlertManager(TorrentManager & torrentManager);

        void start();
        void run();

        void handleAlert(libtorrent::alert * alert);

        void handleSaveResumeDataAlert(libtorrent::save_resume_data_alert * alert);
        void handleSaveResumeDataFailedAlert(libtorrent::save_resume_data_failed_alert * alert);

        void handleFastresumeRejectedAlert(libtorrent::fastresume_rejected_alert * alert);

        void handleMetadataReceivedAlert(libtorrent::metadata_received_alert * alert);

        void handleTorrentFinishedAlert(const libtorrent::torrent_finished_alert * alert);

        void setFrontendBinPath(const std::string & frontendBinPath);
    
    protected:
        TorrentManager & m_torrentManager;
        libtorrent::session & m_session;
    
        std::string m_frontendBinPath;
};

#endif // ALERT_MANAGER_H
