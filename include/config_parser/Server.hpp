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
		ServerConfig					_config;
		int							_nServerSockets;

		int				_initSocket(std::string address, size_t port);
		unsigned int	_ipStringToInt(const std::string& ipAddress);
		int 			_setupServerSockets( void );
		int				_checkConnections( void );
		void   			 _disconnectClient(int i);
		int 			_handleRequest(int i);
		std::string 	extractCGIScriptPath(const std::string& request);
		
		class ConnectionClosedException : public std::exception {
			public:
				ConnectionClosedException(const char* message) : m_message(message) {}
				virtual const char* what() const throw() {
					return m_message;
				}
			private:
				const char* m_message;
		};

		// Exception for socket receive error
		class SocketReceiveErrorException : public std::exception {
			public:
				SocketReceiveErrorException(const char* message) : m_message(message) {}
				virtual const char* what() const throw() {
					return m_message;
				}
			private:
				const char* m_message;
		};

		class SocketInitException : public std::runtime_error {
			public:
				SocketInitException(const std::string& host, const std::string& message, int port, int errNum)
					: std::runtime_error("Host " + host + message + intToStr(port) + ". errno: " + intToStr(errNum)), m_errNum(errNum) {}

				int errorNumber() const { return m_errNum; }

			private:
				int m_errNum;

		};

};

std::ostream &			operator<<( std::ostream & o, Server const & i );
