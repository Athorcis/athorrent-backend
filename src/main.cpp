#include <iostream>
#include <string>

#include <boost/asio/signal_set.hpp>
#include <boost/filesystem.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>    
#include <boost/program_options/variables_map.hpp>

#include "AthorrentService.h"
#include "TorrentManager.h"

int main(int argc, char * argv[]) {
    boost::program_options::options_description desc("Allowed options");
    desc.add_options()
        ("help", "produce help message")
        ("user", boost::program_options::value<std::string>(), "set compression level");
    
    boost::program_options::variables_map vm;
    boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc), vm);
    boost::program_options::notify(vm);

    if (vm.count("help") || !vm.count("user")) {
        std::cout << desc << std::endl;
        return EXIT_FAILURE;
    }
    
    std::string userId = vm["user"].as<std::string>();
    
    if (!boost::filesystem::exists("torrents")) {
        boost::filesystem::create_directory("torrents");
    }
    
    if (!boost::filesystem::exists("files")) {
        boost::filesystem::create_directory("files");
    }
    
    TorrentManager torrentManager(userId);
    AthorrentService service(userId, &torrentManager);
    
    boost::asio::io_service ioService;
    
    boost::asio::signal_set signals(ioService, SIGINT, SIGTERM);

    signals.async_wait([&service](const boost::system::error_code & error, int signal_number) {
        if (signal_number == SIGINT) {
            std::cout << "caught SIGINT" << std::endl;
        } else if (signal_number == SIGTERM) {
            std::cout << "caught SIGTERM" << std::endl;
        }
        
        service.stop();
        exit(EXIT_SUCCESS);
    });

    service.start();
    ioService.run();

	return EXIT_SUCCESS;
}
