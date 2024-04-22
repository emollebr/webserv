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
#include <arpa/inet.h>
#include "Request.hpp"
#include "ServerConfig.hpp"

#define IP "127.0.0.1"
#define PORT 9999
#define QUEUE 10
#define BUF_SIZE 8000
#define MAX_BODY_SIZE 100000
#define DATABASE_DIR "database"
#define UPLOADS_DIR "database/uploads"
#define HTML_INDEX "/index.html"
#define HTML_404 "404.html"
#define HTML_FILE "web.html"
#define HTML_FILE_MANAGER "file_manager.html"


enum ErrorCode {
    SUCCESS = 0,
    FILE_NOT_FOUND = 1,
    INVALID_ARGUMENT = 2,
    OUT_OF_MEMORY = 3,
    NETWORK_ERROR = 4,
	BAD_REQUEST = 5,
    // Add more error codes as needed
};

extern volatile sig_atomic_t g_signal_received;
class ServerConfig;

void 									signal_handler(int signum);
void 								handleListFiles(int clientSocket);
std::vector<std::string> 	listFiles(const std::string& directoryPath);
std::string 					getMimeType(const std::string& filename);
std::string 							finishPath(std::string object);

struct serverSocket {
	std::string address;
	size_t      port;
	int			fd;
};


class Server
{

	public:

		Server(ServerConfig config);
		Server( Server const & src );
		~Server();

		Server &		operator=( Server const & rhs );

	private:
		sockaddr_in 					_sockaddr;
		std::vector<pollfd> 			_sockets;
		std::map<int, Request*>			_request;
		ServerConfig					_config;
		std::vector<serverSocket>		_servSocks;

		int 	setupServerSockets( void );
		int		initSocket(std::string address, size_t port);
		int		checkConnections( void );
		void    disconnectClient(int i);
		int 	handleRequest(int i);
		void 	handleSigpipe( void );
		void	handleSigint( void );


		std::string extractCGIScriptPath(const std::string& request);
		bool isCGIRequest(int fd);
		void executeCGIScript(const std::string& scriptPath, int clientSocket);
		void cleanup( void );

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
				SocketReceiveErrorException(int errorCode) : m_errorCode(errorCode) {}
				virtual const char* what() const throw() {
					return strerror(m_errorCode);
				}
				int errorCode() const {
					return m_errorCode;
				}
			private:
				int m_errorCode;
		};

};

std::ostream &			operator<<( std::ostream & o, Server const & i );
std::ostream& 			operator<<(std::ostream& os, const serverSocket& ss);
