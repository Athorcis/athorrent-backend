#include <iostream>
#include <string>

#include <boost/asio/signal_set.hpp>
#include <boost/filesystem.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>    
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/errors.hpp>
#include <csignal>
#include <boost/stacktrace.hpp>

#include "AthorrentService.h"
#include "TorrentManager.h"
#include "AlertManager.h"
#include "ResumeDataManager.h"

void signal_handler(int signum) {
    ::signal(signum, SIG_DFL);
    ::signal(SIGABRT, SIG_DFL);

    boost::stacktrace::safe_dump_to("./backtrace.dump");
    ::raise(SIGABRT);
}

int main(int argc, char * argv[])
{
    ::signal(SIGSEGV, &signal_handler);
    ::signal(SIGABRT, &signal_handler);

    boost::program_options::positional_options_description p;
    p.add("user", 1);

    boost::program_options::options_description desc("Allowed options");
    desc.add_options()
        ("help", "produce help message")
        ("port", boost::program_options::value<std::string>(), "listening port to use")
        ("frontend-bin", boost::program_options::value<std::string>(), "path to the frontend binary");

    boost::program_options::variables_map vm;

    try {
        boost::program_options::store(
                boost::program_options::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
        boost::program_options::notify(vm);
    } catch (boost::program_options::error &except) {
        std::cerr << except.what();
        return EXIT_FAILURE;
    }

    if (vm.count("help") || !vm.count("port")) {
        std::cerr << desc << std::endl;
        return EXIT_FAILURE;
    }

    std::string port = vm["port"].as<std::string>();

    if (!boost::filesystem::exists("cache")) {
        boost::filesystem::create_directory("cache");
    }

    if (!boost::filesystem::exists("files")) {
        boost::filesystem::create_directory("files");
    }

    TorrentManager torrentManager(port);

    if (vm.count("frontend-bin")) {
        torrentManager.getAlertManager().setFrontendBinPath(vm["frontend-bin"].as<std::string>());
    }

#ifdef _WIN32
    std::string address = "\\\\.\\pipe\\athorrentd\\sockets\\" + port + ".sck";
#elif defined __linux__
    std::string address = "athorrentd.sck";
#endif

    AthorrentService service(address, &torrentManager);

    boost::asio::io_service ioService;
    torrentManager.getResumeDataManager().start(ioService);

    boost::asio::signal_set signals(ioService, SIGINT, SIGTERM);

    signals.async_wait([&service, &torrentManager](const boost::system::error_code &error, int signal_number) {
        if (signal_number == SIGINT) {
            std::cout << "caught SIGINT" << std::endl;
        } else if (signal_number == SIGTERM) {
            std::cout << "caught SIGTERM" << std::endl;
        }

        torrentManager.getResumeDataManager().stop();

        service.stop();

        exit(EXIT_SUCCESS);
    });

    service.start();
    ioService.run();

	return EXIT_SUCCESS;
}
