#ifndef ALERT_MANAGER_H
#define ALERT_MANAGER_H

#include "TorrentManager.h"

#include <libtorrent/alert_types.hpp>
#include <libtorrent/session.hpp>

class AlertManager
{
    public:
        AlertManager(TorrentManager & torrentManager);

        void start();
        void run();

        void handleAlert(libtorrent::alert * alert);

        void handleSaveResumeDataAlert(libtorrent::save_resume_data_alert * alert);
        void handleSaveResumeDataFailedAlert(libtorrent::save_resume_data_failed_alert * alert);

        void handleFastresumeRejectedAlert(libtorrent::fastresume_rejected_alert * alert);

        void handleMetadataReceivedAlert(libtorrent::metadata_received_alert * alert);

        void handleTorrentFinishedAlert(libtorrent::torrent_finished_alert * alert);

    protected:
        TorrentManager & m_torrentManager;
        libtorrent::session & m_session;
};

#endif // ALERT_MANAGER_H
