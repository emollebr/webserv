#pragma once
# define SERVER_HPP

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
#include <unistd.h> // For dup2, execl, close
#include <sys/wait.h>
#include <sys/stat.h>
#include <fstream>      // For std::ifstream
#include <sstream> 
#include <climits>
#include <list>
#include <string>
#include <dirent.h>
#include <sstream>
#include "Request.hpp"

#define PORT 9999
#define QUEUE 10
#define BUF_SIZE 8000
#define DATABASE_DIR "database"
#define UPLOADS_DIR "database/uploads"
#define HTML_INDEX "/index.html"
#define HTML_404 "404.html"
#define HTML_FILE "web.html"
#define HTML_FILE_MANAGER "file_manager.html"

extern volatile sig_atomic_t g_signal_received;

void 									signal_handler(int signum);
void 								handleListFiles(int clientSocket);
std::vector<std::string> 	listFiles(const std::string& directoryPath);
std::string 					getMimeType(const std::string& filename);
std::string 							finishPath(std::string object);

class Server
{

	public:

		Server();
		Server( Server const & src );
		~Server();

		Server &		operator=( Server const & rhs );


	private:
		sockaddr_in 					_sockaddr;
		std::vector<pollfd> 			_sockets;
		std::map<int, Request*>			_request;

		void	initSocket( void );
		int		checkConnections( void );
		void 	readFromClient(int i);
		void    disconnectClient(int i);
		int 	handleRequest(int i);
		void 			handleSigpipe( void );
		void	handleSigint( void );


		std::string extractCGIScriptPath(const std::string& request);
		bool isCGIRequest(int fd);
		void executeCGIScript(const std::string& scriptPath, int clientSocket);
		void cleanup( void );
};

std::ostream &			operator<<( std::ostream & o, Server const & i );
