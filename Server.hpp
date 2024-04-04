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
#include <fstream>
#include <string>

#define PORT 9999
#define QUEUE 10
#define HTML_FILE "web.html"

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
		int		checkConnections( void );
		void 	disconnectClient(int i, ssize_t bytesRead);
		int  	sendToClient(int i, char buffer[]);

};

std::ostream &			operator<<( std::ostream & o, Server const & i );
