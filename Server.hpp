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

#define PORT 9999
#define QUEUE 10

class Server
{

	public:

		Server();
		Server( Server const & src );
		~Server();

		Server &		operator=( Server const & rhs );


	private:
		sockaddr_in _sockaddr;
		std::vector<pollfd> _sockets;

		void	initSocket( void );
		void	checkConnections( void );
		void 	readFromClient(int i);
		void  	sendToClient(int i);
		std::string extractCGIScriptPath(const std::string& request);
		bool isCGIRequest(const std::string& request);
		void executeCGIScript(const std::string& scriptPath, int clientSocket, std::vector<char*> env);
		void handleSocketData(int socket);
		void handleRequest(int i);
		void serveIndexHTML(int clientSocket);
		void cleanup();

};

std::ostream &			operator<<( std::ostream & o, Server const & i );
