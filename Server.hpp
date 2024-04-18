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
#include "Request.hpp"

#define PORT 9999
#define QUEUE 10
#define HTML_FILE "web.html"
#define BUF_SIZE 58761
# define MAX_REQ_SIZE 8192

void handleListFiles(int clientSocket);

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

		void 	handleRequest(int i);
		int    detectRequestType(int client);
		int 	handleDelete(int fd);
		int 	handleUnknown(int fd);
		int 	handleGet(int fd);
		int 	handlePost(int fd);

		std::string extractCGIScriptPath(const std::string& request);
		bool isCGIRequest(int fd);
		void executeCGIScript(const std::string& scriptPath, int clientSocket);
		void cleanup();
};

std::ostream &			operator<<( std::ostream & o, Server const & i );
