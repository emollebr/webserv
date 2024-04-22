#include "Server.hpp"

volatile sig_atomic_t g_signal_received = 0;

int Server::setupServerSockets( void ) {
  /*   std::map<std::string, std::vector<size_t> > listen = _config.getListenMap();
    for (std::map<std::string, std::vector<size_t> >::const_iterator it = listen.begin(); it != listen.end(); ++it) {

        const std::vector<size_t>& ports = it->second;

        for (size_t port : ports) {
            serverSocket sock;
            sock.fd = initSocket(it->first, port);
            sock.address = it->first;
            sock.port = port;
            _servSocks.push_back(sock);
        }
    }
    for (size_t i = 0; i < _servSocks.size(); ++i) {
        std::cout << "Server Socket " << i << ": " << _servSocks[i] << std::endl;
    } */
    return 0;
}

int  Server::initSocket(std::string address, size_t port)
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
    _sockaddr.sin_addr.s_addr = inet_addr(address.c_str());
    _sockaddr.sin_port = htons(port); // htons is necessary to convert a number to network byte order
    
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
    return (serversock.fd);
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

    char	buffer[BUF_SIZE] = {0};
    ssize_t	bytesRead;

    try { //reading from client socket
        bytesRead = recv(fd, &buffer, BUF_SIZE, O_NONBLOCK);
        if (bytesRead == 0)
            throw ConnectionClosedException("Client closed the connection\n");
        else if (bytesRead == -1)
            throw SocketReceiveErrorException(errno);
    } catch (const std::exception& e) {
        std::cerr << "Caught exception: " << e.what() << std::endl;
        return -1;
    }
	fflush( stdout );

    if (_request.count(fd) == 0) { //make new request
        try {
            _request.insert(std::make_pair(fd, new Request(buffer, fd, bytesRead)));
        } catch (const std::exception& e) {
            std::cout << "Caught exception: " << e.what() << std::endl;
            return -1;
        }
    }
    else if (_request[fd]->getMethod() == "POST") { //pending chunked request
        _request[fd]->pendingPostRequest(buffer, bytesRead);
    }
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


Server::Server(ServerConfig config) : _config(config)
{
    signal(SIGPIPE, signal_handler);
    signal(SIGINT, signal_handler);

    sethostname(_config.getHost().c_str(), _config.getHost().size());
    //setupServerSockets();
    initSocket(IP, PORT); //will be replaced by previous line
    
    while (true) {
        checkConnections(); // Here poll() is called, which is a select() equivalent
        for (int i = _sockets.size() - 1; i >= 1; --i) { // 1 will be swapped for '_servSocks.size()'
            if (g_signal_received == SIGPIPE)
                handleSigpipe();
            if (g_signal_received == SIGINT)
                handleSigint();
            if (_sockets[i].revents & POLLIN) {// Check if there's data to read on client socket
                try {
                    if (handleRequest(i) == 1)
                        break; ;
                } catch (const std::exception& e) {
                    std::cout << "Caught exception: " << e.what() << std::endl;
                }
                disconnectClient(i);
            }
            else if (_request.find(_sockets[i].fd) != _request.end()) { // Client is waiting for chunked response
                if (_request[_sockets[i].fd]->hasPendingResponse()) {
                    if (_request[_sockets[i].fd]->sendResponse() == 0)
                        disconnectClient(i);
                }
            }
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

std::ostream& operator<<(std::ostream& os, const serverSocket& ss) {
    os << "Address: " << ss.address << ", Port: " << ss.port << ", File Descriptor: " << ss.fd;
    return os;	
}

/*
** --------------------------------- METHODS ----------------------------------
*/


/*
** --------------------------------- ACCESSOR ---------------------------------
*/


/* ************************************************************************** */