#include "Server.hpp"

volatile sig_atomic_t g_signal_received = 0;


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

    return 0;
}

//Properly disconnects a client [i] in the socket array and deletes any request
void    Server::_disconnectClient(int i) {
    // Connection closed or error occurred
    if (_request.find(_sockets[i].fd) != _request.end()) {
        delete _request[_sockets[i].fd];
        _request.erase(_sockets[i].fd);
    }
    close(_sockets[i].fd);
    _sockets.erase(_sockets.begin() + i);
}

int Server::_handleRequest(int i) {
    int fd = _sockets[i].fd;

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
        try {
            _request.insert(std::make_pair(fd, new Request(buffer, fd, bytesRead, MAX_BODY_SIZE, _config.getErrorPages())));
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

void Server::handleSigint() {
    for (int i = _sockets.size() - 1; i > 0; --i) {
        if (_request.find(_sockets[i].fd) != _request.end()) {
            _disconnectClient(i);
        }
    }
    if (_sockets.size() > 0)
        close(_sockets[0].fd);
}

int Server::serverRun( void ) {
    const int POLL_TIMEOUT = 1;
    //poll allows to monitor multiple fd's for events
    int newEvents = poll(&_sockets[0], _sockets.size(), POLL_TIMEOUT);
    if (newEvents  < 0) {
        std::perror("poll");
        handleSigint();
    }
    if (_checkConnections() != 0)
        return -1; //handle error correctly later!

    //Check all client sockets
    for (size_t i = _sockets.size() - 1; static_cast<int>(i) >= _nServerSockets; --i) {
        if (_sockets[i].revents & POLLIN) {// Check if there's data to read on client socket
            try {
                if (_handleRequest(i) == KEEP_ALIVE)
                    break; ;
            } catch (const std::exception& e) {
                std::cout << "Caught exception: " << e.what() << std::endl;
            }
            _disconnectClient(i);
        }
        else if (_request.find(_sockets[i].fd) != _request.end()) { // Client is waiting for chunked response
            if (_request[_sockets[i].fd]->hasPendingResponse()) {
                if (_request[_sockets[i].fd]->createResponse() < KEEP_ALIVE)
                    _disconnectClient(i);
            }
        }
    }
    return (0);
}


Server::Server(ServerConfig& config) : _config(config), _nServerSockets(0)
{
    sethostname(_config.getHost().c_str(), _config.getHost().size());
    try {
        const std::vector<size_t>& ports = _config.getListenPorts();

        for (std::vector<size_t>::const_iterator it = ports.begin(); it != ports.end(); ++it) {
            _initSocket(_config.getHost(), *it);
        }
    } catch (const std::exception& e) {
        handleSigint();
        std::cout << "Caught exception: " << e.what() << std::endl;
        throw e;
    }
}

Server::Server(const Server &src)
    : _sockaddr(src._sockaddr),
      _sockets(src._sockets),
      _config(src._config),
      _nServerSockets(src._nServerSockets) {
    // Deep copy _request map
    for (std::map<int, Request*>::const_iterator it = src._request.begin(); it != src._request.end(); ++it) {
        _request[it->first] = new Request(*(it->second));
    }
}

/*
** -------------------------------- DESTRUCTOR --------------------------------
*/

Server::~Server()
{
     //for (int i = _sockets.size(); i >= 0; --i) {
     //   close(_sockets[i].fd);
    //}
}


/*
** --------------------------------- OVERLOAD ---------------------------------
*/

Server &Server::operator=(Server const &rhs) {
    if (this != &rhs) {
        // Assign members
        _sockaddr = rhs._sockaddr;
        _sockets = rhs._sockets;
        _config = rhs._config;
        _nServerSockets = rhs._nServerSockets;

        // Deep copy _request map
        for (std::map<int, Request*>::const_iterator it = rhs._request.begin(); it != rhs._request.end(); ++it) {
            _request[it->first] = new Request(*(it->second));
        }
    }
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