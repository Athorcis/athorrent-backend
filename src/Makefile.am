
WARNINGS = -Wall -Wextra -Wno-deprecated-declarations -Wno-unused-parameter
AM_CPPFLAGS = -I../include $(WARNINGS) -pedantic -DBOOST_NO_CXX11_SCOPED_ENUMS
AM_CXXFLAGS = -std=c++14 $(BOOST_CPPFLAGS) -D BOOST_STACKTRACE_LINK

bin_PROGRAMS = athorrent-backend
athorrent_backend_SOURCES = main.cpp TorrentManager.cpp Utils.cpp LocalSocket.cpp LocalServerSocket.cpp LocalClientSocket.cpp AthorrentService.cpp JsonRequest.cpp JsonResponse.cpp AlertManager.cpp ResumeDataManager.cpp BadJsonRequestException.cpp
athorrent_backend_LDADD = -ltorrent-rasterbar @BOOST_SYSTEM_LIB@ @BOOST_PROGRAM_OPTIONS_LIB@ @BOOST_FILESYSTEM_LIB@ @BOOST_STACKTRACE_LIB@ @BOOST_THREAD_LIB@

if WINDOWS
    AM_CPPFLAGS += -D_WIN32_WINNT=0x0600
    athorrent_backend_LDADD += -lwsock32 -lws2_32
endif

if SETCAP_FOUND
install-exec-hook:
	setcap 'cap_net_bind_service=+ep' $(DESTDIR)$(bindir)/athorrent-backend
endif
