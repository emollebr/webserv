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
#include <fstream>      // For std::ifstream
#include <sstream> 
#include <climits>
#include <list>
#include "Request.hpp"

#define PORT 9999
#define QUEUE 10
#define HTML_FILE "web.html"
#define BUF_SIZE 31450000000000000

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
		std::map<int, Request>			_request;

		void	initSocket( void );
		int		checkConnections( void );
		void 	readFromClient(int i);
		void    disconnectClient(int bytesRead, int i);


		void    detectRequestType(int client);
		int 	handleGet(int client);
		int 	handlePost(int client);
		int 	handleDelete(int client);
		int 	handleUnknown(int client);

		void  	sendToClient(int i, char buffer[], ssize_t bytesRead);
		std::string extractCGIScriptPath(const std::string& request);
		bool isCGIRequest(const std::string& request);
		void executeCGIScript(const std::string& scriptPath, int clientSocket);
		void handleRequest(int i);
		void serveIndexHTML(int clientSocket);
		void cleanup();



};

std::ostream &			operator<<( std::ostream & o, Server const & i );
