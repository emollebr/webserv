#include "Server.hpp"


void  Server::initSocket( void )
{
    pollfd serversock; //this will be the first element in our poll vector and the servers socket
    serversock.fd = socket(AF_INET, SOCK_STREAM, 0);
    if (serversock.fd == -1) {
      std::cout << "Failed to create socket. errno: " << errno << std::endl;
      exit(EXIT_FAILURE);
    }
    serversock.events = POLLIN; // Looking for events: IN sig
    _sockets.push_back(serversock); //adding the server socket to the vector
    
    //populate the sockaddr struct
    _sockaddr.sin_family = AF_INET;
    _sockaddr.sin_addr.s_addr = INADDR_ANY;
    _sockaddr.sin_port = htons(PORT); // htons is necessary to convert a number to network byte order
    
    if (bind(serversock.fd, (struct sockaddr*)&_sockaddr, sizeof(sockaddr)) < 0) {
      std::cout << "Failed to bind to port 9999. errno: " << errno << std::endl;
      exit(EXIT_FAILURE);
    }
    
    if (listen(serversock.fd, QUEUE) < 0) { // Start listening. Hold at most x connections in the queue
      std::cout << "Failed to listen on socket. errno: " << errno << std::endl;
      exit(EXIT_FAILURE);
    }

    fcntl(serversock.fd, F_SETFL, O_NONBLOCK); // Set server socket to non-blocking mode
}

void  Server::checkConnections()
{
  //poll allows to monitor multiple fd's and prevent blocking
  if (poll(&_sockets[0], _sockets.size(), -1) < 0) {
      std::perror("poll");
      exit(EXIT_FAILURE);
  }

  if (_sockets[0].revents & POLLIN) { // Check if there's a new connection on the server socket
  	unsigned int addrlen = sizeof(sockaddr);
    int new_socket = accept(_sockets[0].fd, (struct sockaddr *)&_sockaddr, (socklen_t*)&addrlen);
    if (new_socket < 0) {
        std::perror("accept");
        exit(EXIT_FAILURE);
    }
    
    fcntl(new_socket, F_SETFL, O_NONBLOCK); // Set new socket to non-blocking mode
    
    // Add new connection/socket to the vector
    pollfd newfd;
    newfd.fd = new_socket;
    newfd.events = POLLIN;
    _sockets.push_back(newfd);
  }
}

void  Server::disconnectClient(int i, ssize_t bytesRead) {

    if (bytesRead == 0 || (errno != EWOULDBLOCK && errno != EAGAIN)) {
        // Connection closed or error occurred
        std::cout << "Client disconnected" << std::endl;
        close(_sockets[i].fd);
        _sockets.erase(_sockets.begin() + i);
    }
}


void  Server::sendToClient(int i, char buffer[]) {
  std::cout << "Received: " << buffer << "\n" << std::endl;
  std::string response = "Good talking to you\n";
  send(_sockets[i].fd, response.c_str(), response.size(), 0);
}

Server::Server()
{
  initSocket();

  while (true) {
    checkConnections();

    for (int i = _sockets.size() - 1; i >= 1; --i) {
      if (_sockets[i].revents & POLLIN) { // Check if there's data to read on client socket
        char buffer[100];
        ssize_t bytesRead = read(_sockets[i].fd, buffer, sizeof(buffer));
        if (bytesRead <= 0) {
            disconnectClient(i, bytesRead);
        } else {
          sendToClient(i, buffer);
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
    close(_sockets[0].fd);
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