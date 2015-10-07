
INCLUDES:=-I include $(INCLUDES)
WARNINGS=-Wall -Wextra -Wno-deprecated-declarations -Wno-unused-parameter -Werror
CXXFLAGS:=-Wall $(WARNINGS) -pedantic -std=c++11 -DBOOST_ASIO_DYN_LINK -DJSON_ISO_STRICT -DNDEBUG $(CXXFLAGS) $(INCLUDES) -g
LDLIBS:=-lboost_system$(BOOSTLIB_SUFFIX) -lboost_program_options$(BOOSTLIB_SUFFIX) -lboost_filesystem$(BOOSTLIB_SUFFIX) -lboost_thread$(BOOSTLIB_SUFFIX) -lboost_locale$(BOOSTLIB_SUFFIX) -ljson $(LDLIBS)
LDFLAGS=-g
OBJECTS=obj/main.o obj/TorrentManager.o obj/Utils.o obj/LocalSocket.o obj/LocalServerSocket.o obj/LocalClientSocket.o obj/AthorrentService.o obj/JsonRequest.o obj/JsonResponse.o
EXEC=bin/athorrentd

default: $(EXEC)

$(EXEC): obj bin $(OBJECTS)
	@echo CXX $@ && $(CXX) $(LDFLAGS) -o $@ $(OBJECTS) $(LDLIBS)

bin:
	@mkdir $@

obj:
	@mkdir $@

obj/main.o: src/main.cpp include/AthorrentService.h include/TorrentManager.h
	@echo CXX $@ && $(CXX) $(CXXFLAGS) -c -o $@ $<

obj/TorrentManager.o: src/TorrentManager.cpp include/TorrentManager.h include/Utils.h
	@echo CXX $@ && $(CXX) $(CXXFLAGS) -c -o $@ $<

obj/JsonRequest.o: src/JsonRequest.cpp include/JsonRequest.h include/Utils.h
	@echo CXX $@ && $(CXX) $(CXXFLAGS) -c -o $@ $<

obj/JsonResponse.o: src/JsonResponse.cpp include/JsonResponse.h
	@echo CXX $@ && $(CXX) $(CXXFLAGS) -c -o $@ $<

obj/LocalSocket.o: src/LocalSocket.cpp include/LocalSocket.h
	@echo CXX $@ && $(CXX) $(CXXFLAGS) -c -o $@ $<

obj/LocalClientSocket.o: src/LocalClientSocket.cpp include/LocalClientSocket.h
	@echo CXX $@ && $(CXX) $(CXXFLAGS) -c -o $@ $<

obj/LocalServerSocket.o: src/LocalServerSocket.cpp include/LocalServerSocket.h
	@echo CXX $@ && $(CXX) $(CXXFLAGS) -c -o $@ $<

obj/AthorrentService.o: src/AthorrentService.cpp include/AthorrentService.h include/Utils.h
	@echo CXX $@ && $(CXX) $(CXXFLAGS) -c -o $@ $<

obj/Utils.o: src/Utils.cpp include/Utils.h
	@echo CXX $@ && $(CXX) $(CXXFLAGS) -c -o $@ $<

include/AthorrentService.h: include/TorrentManager.h include/JsonServer.h include/LocalServerSocket.h include/LocalClientSocket.h

include/JsonServer.h: include/JsonClient.h src/JsonServer.cpp

include/JsonClient.h: include/JsonRequest.h include/JsonResponse.h src/JsonClient.cpp

build: $(EXEC)

clean:
	@rm obj/*.o

rebuild: clean build

install: build
	cp $(EXEC) $(INSTALLDIR)
