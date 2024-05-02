#include "Server.hpp"

volatile sig_atomic_t g_signal_received = 0;

void  Server::initSocket( void )
{
    pollfd serversock; //this will be the first element in our poll vector and the servers socket
    serversock.fd = socket(AF_INET, SOCK_STREAM, 0);
    if (serversock.fd == -1) {
      std::cout << "Failed to create socket. errno: " << errno << std::endl;
      exit(EXIT_FAILURE);
    }
    serversock.events = POLLIN; // Looking for events: IN sig
    
    //populate the sockaddr struct
    _sockaddr.sin_family = AF_INET;
    _sockaddr.sin_addr.s_addr = INADDR_ANY;
    _sockaddr.sin_port = htons(PORT); // htons is necessary to convert a number to network byte order
    
    if (bind(serversock.fd, (struct sockaddr*)&_sockaddr, sizeof(sockaddr)) < 0) {
      std::cout << "Failed to bind to port 9999. errno: " << errno << std::endl;
      close(serversock.fd);
      exit(EXIT_FAILURE);
    }
    
    if (listen(serversock.fd, QUEUE) < 0) { // Start listening. Hold at most x connections in the queue
      std::cout << "Failed to listen on socket. errno: " << errno << std::endl;
      close(serversock.fd);
      exit(EXIT_FAILURE);
    }

    fcntl(serversock.fd, F_SETFL, O_NONBLOCK); // Set server socket to non-blocking mode
    _sockets.push_back(serversock); //adding the server socket to the vector
}

int  Server::checkConnections()
{
    const int POLL_TIMEOUT = 1;

  //poll allows to monitor multiple fd's for events
  int newEvents = poll(&_sockets[0], _sockets.size(), POLL_TIMEOUT);
  if (newEvents  < 0) {
      std::perror("poll");
      exit(EXIT_FAILURE);
  }
  if (newEvents == 0)
    return 0;
  // Check if there's a new connection on the server socket
  if (_sockets[0].revents & POLLIN) { 
  	unsigned int addrlen = sizeof(sockaddr);
    int new_socket = accept(_sockets[0].fd, (struct sockaddr *)&_sockaddr, (socklen_t*)&addrlen);
    if (new_socket < 0) {
        std::perror("accept");
        exit(EXIT_FAILURE);
    }
    //set timeout for connection
    struct timeval timeout;
    timeout.tv_sec = 10;  // 10 seconds timeout
    timeout.tv_usec = 0;
    if (setsockopt(new_socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout)) < 0) {
        perror("Error setting timeout");
        close(new_socket);
        return 1;
    }
    
    fcntl(new_socket, F_SETFL, O_NONBLOCK); // Set new socket to non-blocking mode
    
    // Add new connection/socket to the vector
    pollfd newfd;
    newfd.fd = new_socket;
    std::cout << "New connection: " << new_socket << std::endl;
    newfd.events = POLLIN;
    _sockets.push_back(newfd);
  }

  return newEvents;
}


std::string Server::extractCGIScriptPath(const std::string& request) {
    // Parse the request URL to extract the script path
    size_t urlPos = request.find(" /");
    if (urlPos != std::string::npos) {
        std::string url = request.substr(urlPos + 2); // Skip space and slash
        size_t cgiPos = url.find("cgi-bin/");
        if (cgiPos == 0) {
            // Extract the script path from the URL
            size_t scriptStart = cgiPos + 9; // Skip "/cgi-bin/"
            size_t scriptEnd = url.find_first_of(" ?", scriptStart);
            if (scriptEnd != std::string::npos) {
                std::string scriptPath = url.substr(scriptStart, scriptEnd - scriptStart);
                // Append the script path to the CGI directory
                return "cgi-bin/" + scriptPath;
            }
        }
    }

    // If the request URL does not contain "/cgi-bin/", return an empty string
    return "";
}

unsigned int Server::_ipStringToInt(const std::string& ipAddress) {
    std::vector<unsigned int>   parts;
    std::istringstream          ipStream(ipAddress);
    std::string                 part;
    unsigned int                ip;
    int                         count = 0;

    if (ipAddress.empty())
        return(0);
    while (std::getline(ipStream, part, '.')) 
    {
        count++;
        unsigned int num = static_cast<unsigned int>(atoi(part.c_str()));
        parts.push_back(num);
    }
    if (count != 4)
        throw(std::runtime_error("Error, IP address has invalid format"));
    ip = (parts[0] << 24) | (parts[1] << 16) | (parts[2] << 8) | parts[3];
    return (ip);
}

int  Server::_initSocket(std::string address, size_t port)
{
    pollfd serversock; //this will be the first element in our poll vector and the servers socket
    serversock.fd = socket(AF_INET, SOCK_STREAM, 0);
    if (serversock.fd == -1)
        throw SocketInitException(address, " failed to create socket for port ", port, errno);
    serversock.events = POLLIN; // Looking for events: IN sig
    
    //populate the sockaddr struct
    _sockaddr.sin_family = AF_INET;
    unsigned int ip = _ipStringToInt(address);
    _sockaddr.sin_addr.s_addr = htonl(ip);
    _sockaddr.sin_port = htons(port); // htons is necessary to convert a number to network byte order
    
    if (bind(serversock.fd, (struct sockaddr*)&_sockaddr, sizeof(sockaddr)) < 0) {
      close(serversock.fd);
      throw SocketInitException(address, " failed to bind to port ", port, errno);
    }
    
    if (listen(serversock.fd, QUEUE) < 0) { // Start listening. Hold at most x connections in the queue
      close(serversock.fd);
      throw SocketInitException(address, " failed to listen on socket for port ", port, errno);
    }

    fcntl(serversock.fd, F_SETFL, O_NONBLOCK); // Set server socket to non-blocking mode
    _sockets.push_back(serversock); //adding the server socket to the vector
    _nServerSockets += 1;
    return (serversock.fd);
}

// Accepts new connections on server sockets
int  Server::_checkConnections()
{
    // Check if there's a new connection on the server sockets
    for (int i = 0; i < _nServerSockets; ++i) {
        if (_sockets[i].revents & POLLIN) { 
            unsigned int addrlen = sizeof(sockaddr);
            int new_socket = accept(_sockets[i].fd, (struct sockaddr *)&_sockaddr, (socklen_t*)&addrlen);
            if (new_socket < 0) {
                std::perror("accept");
                return 1;
            }
            //set timeout for connection
            struct timeval timeout;
            timeout.tv_sec = 20;  // 10 seconds timeout
            timeout.tv_usec = 0;
            if (setsockopt(new_socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout)) < 0) {
                perror("Error setting timeout");
                close(new_socket);
                return 1;
            }
            
            fcntl(new_socket, F_SETFL, O_NONBLOCK); // Set new socket to non-blocking mode
            
            // Add new connection/socket to the vector
            pollfd newfd;
            newfd.fd = new_socket;
            newfd.events = POLLIN;
            _sockets.push_back(newfd);
        }
    }
}




void    Server::disconnectClient(int i) {
    // Connection closed or error occurred
    if (_request.find(_sockets[i].fd) != _request.end()) {
        delete _request[_sockets[i].fd];
        _request.erase(_sockets[i].fd);
    }
    std::cout << "Client " << _sockets[i].fd << " disconnected" << std::endl;
    close(_sockets[i].fd);
    _sockets.erase(_sockets.begin() + i);
}

int Server::handleRequest(int i) {
    int fd = _sockets[i].fd;
    ssize_t	bytesRead;
    char	buffer[BUF_SIZE] = {0};
    ssize_t	bytesRead;
    try { //reading from client socket
        bytesRead = recv(fd, &buffer, BUF_SIZE, O_NONBLOCK);
        if (bytesRead == 0)
            throw ConnectionClosedException("Client closed the connection\n");
        else if (bytesRead == -1)
            throw SocketReceiveErrorException("Failed to read from client's socket");
    } catch (const std::exception& e) {
        std::cerr << "Caught exception: " << e.what() << std::endl;
        return -1;
    }
	fflush( stdout );

    if (_request.count(fd) == 0) { //make new request
        _request.insert(std::make_pair(fd, new Request(buffer, fd, bytesRead)));
    }
    else if (_request[fd]->getMethod() == "POST") { //pending chunked request
        _request[fd]->pendingPostRequest(buffer, bytesRead);
    }
    /*} else if (_request[fd]->hasPendingResponse()) { //pending chunked response
        std::cout << "sclient shouldnt send request right now" << std::endl;
        return ;
    }*/
    return (_request[fd]->detectRequestType());
}

void Server::handleSigpipe() {
for (std::map<int, Request*>::iterator it = _request.begin(); it != _request.end();  ++it) {
        if (it->second->hasPendingResponse()) {
            int fd = it->first;
            close(fd);
            delete it->second;
            _request.erase(it);
        }
    }
    g_signal_received = 0;  // Reset the flag
}

void Server::handleSigint() {
    for (int i = _sockets.size() - 1; i > 0; --i) {
        if (_request.find(_sockets[i].fd) != _request.end()) {
            disconnectClient(i);
        }
    }
    close(_sockets[0].fd);
    exit(EXIT_SUCCESS);
}

Server::Server()
{
    initSocket();
    signal(SIGPIPE, signal_handler);
    signal(SIGINT, signal_handler);
    while (true) {
        checkConnections();
        for (int i = _sockets.size() - 1; i >= 1; --i) {
            if (_sockets[i].revents & POLLIN) {// Check if there's data to read on client socket
                if (handleRequest(i) == 0) {
                    disconnectClient(i);
                }
            } 
            else if (_request.find(_sockets[i].fd) != _request.end()) {
                if (_request[_sockets[i].fd]->hasPendingResponse()) {
                    if (_request[_sockets[i].fd]->sendResponse() == 0) {
                        disconnectClient(i);
                    }
                }
            }
            if (g_signal_received == SIGPIPE) {
                handleSigpipe();
            }
            if (g_signal_received == SIGINT)
                handleSigint();
        }
    }
}

Server::Server( const Server & src )
{
	(void)src;
}

/*
** -------------------------------- DESTRUCTOR --------------------------------
*/

Server::~Server()
{
     for (int i = _sockets.size(); i >= 0; --i) {
        close(_sockets[i].fd);
    }
}


/*
** --------------------------------- OVERLOAD ---------------------------------
*/

Server &				Server::operator=( Server const & rhs )
{
	//if ( this != &rhs )
	//{
		//this->_value = rhs.getValue();
	//}
	(void)rhs;
	return *this;
}

std::ostream &			operator<<( std::ostream & o, Server const & i )
{
	//o << "Value = " << i.getValue();
	(void)i;
	return o;
}


/*
** --------------------------------- METHODS ----------------------------------
*/


/*
** --------------------------------- ACCESSOR ---------------------------------
*/


/* ************************************************************************** */