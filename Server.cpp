#include "Server.hpp"

/*
** ------------------------------- CONSTRUCTOR --------------------------------
*/

Server::Server()
{
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd == -1) {
    std::cout << "Failed to create socket. errno: " << errno << std::endl;
    exit(EXIT_FAILURE);
  }
  
  // Listen to port 9999 on any address
  sockaddr.sin_family = AF_INET;
  sockaddr.sin_addr.s_addr = INADDR_ANY;
  sockaddr.sin_port = htons(PORT); // htons is necessary to convert a number to network byte order
  
  if (bind(sockfd, (struct sockaddr*)&sockaddr, sizeof(sockaddr)) < 0) {
    std::cout << "Failed to bind to port 9999. errno: " << errno << std::endl;
    exit(EXIT_FAILURE);
  }
  
  if (listen(sockfd, 10) < 0) { // Start listening. Hold at most 10 connections in the queue
    std::cout << "Failed to listen on socket. errno: " << errno << std::endl;
    exit(EXIT_FAILURE);
  }

  fcntl(sockfd, F_SETFL, O_NONBLOCK); // Set server socket to non-blocking mode
  
  pollfd serversock;
  serversock.fd = sockfd; // First element is the server socket
  serversock.events = POLLIN; // Looking for events: IN sig
  connects.push_back(serversock);

  while (true) {
    int ret = poll(&connects[0], connects.size(), -1);
    if (ret < 0) {
        std::perror("poll");
        exit(EXIT_FAILURE);
    }
	unsigned int addrlen = sizeof(sockaddr);
    if (connects[0].revents & POLLIN) { // Check if there's a new connection on the server socket
      int new_socket = accept(sockfd, (struct sockaddr *)&sockaddr, (socklen_t*)&addrlen);
      if (new_socket < 0) {
          std::perror("accept");
          exit(EXIT_FAILURE);
      }
      
      fcntl(new_socket, F_SETFL, O_NONBLOCK); // Set new socket to non-blocking mode
      
      // Add new connection/socket to the vector
      pollfd newfd;
      newfd.fd = new_socket;
      newfd.events = POLLIN;
      connects.push_back(newfd);
    }
    
    for (size_t i = 1; i < connects.size() - 1; ++i) {
      if (connects[i].revents & POLLIN) { // Check if there's data to read on client socket
        char buffer[100];
        ssize_t bytesRead = read(connects[i].fd, buffer, sizeof(buffer));
        if (bytesRead <= 0 || (errno != EWOULDBLOCK && errno != EAGAIN)) {
            // Connection closed or error occurred
            close(connects[i].fd);
            connects.erase(connects.begin() + i);
        } else {
          // Echo back to client
          std::cout << "Received: " << buffer << "\n" << std::endl;
          std::string response = "Good talking to you\n";
          send(connects[i].fd, response.c_str(), response.size(), 0);
        }
      }
    }
  }

  close(sockfd);
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