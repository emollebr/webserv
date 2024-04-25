#pragma once
# define COMMON_HPP

#include <sys/socket.h> // For socket functions
#include <netinet/in.h> // For sockaddr_in
#include <cstdlib> // For exit() and EXIT_FAILURE
#include <iostream> // For cout
#include <unistd.h> // For read
#include <fcntl.h>
#include <poll.h>
#include <vector>
#include <cstring>
#include <errno.h>
#include <cstdio>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fstream>      // For std::ifstream
#include <sstream> 
#include <climits>
#include <list>
#include <string>
#include <dirent.h>
#include <arpa/inet.h>
#include <map>
#include <exception>

#include "ServerConfig.hpp"
#include "Request.hpp"
#include "Server.hpp"

#define IP "127.0.0.1"
#define PORT 9999
#define SERVERSOCKETS_NUM 1
#define QUEUE 10
#define BUF_SIZE 8000
#define MAX_BODY_SIZE 100000
#define DATABASE_DIR "database"
#define UPLOADS_DIR "database/uploads"
#define HTML_INDEX "/index.html"
#define HTML_404 "404.html"
#define HTML_FILE "web.html"
#define HTML_FILE_MANAGER "file_manager.html"

class Server;
class Request;

enum ErrorCode {
	SEND_ERROR = -2,
	CONNECTION_CLOSED = -1,
    SUCCESS = 0,
    KEEP_ALIVE = 1,
    INVALID_ARGUMENT = 2,
    OUT_OF_MEMORY = 3,
    NETWORK_ERROR = 4,
	BAD_REQUEST = 5,
    // Add more error codes as needed
};

extern volatile sig_atomic_t g_signal_received;
class ServerConfig;

void 						signal_handler(int signum);
std::string 				intToStr(int num);
void 						handleListFiles(int clientSocket);
std::vector<std::string> 	listFiles(const std::string& directoryPath);
std::string 				getMimeType(const std::string& filename);
std::string 				finishPath(std::string object);
