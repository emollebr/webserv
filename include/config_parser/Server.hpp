#pragma once
# define SERVER_HPP

#include "common.hpp"
#include "ServerConfig.hpp"
#include "Request.hpp"

class Request;
std::string 				intToStr(int num);

class Server
{

	public:

		Server(ServerConfig& config);
		Server( Server const & src );
		~Server();

		Server &		operator=( Server const & rhs );
		int			 	serverRun();
		void 			handleSigpipe( void );
		void			handleSigint( void );

	private:
		sockaddr_in 					_sockaddr;
		std::vector<pollfd> 			_sockets;
		std::map<int, Request*>			_request;
		ServerConfig					_config;
		int							_nServerSockets;

		int		_initSocket(std::string address, size_t port);
		unsigned int _ipStringToInt(const std::string ipAddress);
		int 	_setupServerSockets( void );
		int		_checkConnections( void );
		void    _disconnectClient(int i);
		int 	_handleRequest(int i);

		std::string extractCGIScriptPath(const std::string& request);
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