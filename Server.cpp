#include "Server.hpp"

static void printRequest(Request& req) {
    std::cout << "Type: " << req.getType() << std::endl;
    std::cout << "Object: " << req.getObject() << std::endl;
    std::cout << "Headers: " << std::endl;
    for (std::map<std::string, std::string>::const_iterator it = req.getHeaders().begin(); it != req.getHeaders().end(); ++it) {
        std::cout << it->first << ": " << it->second << std::endl;
    }
    std::cout << "Client FD: " << req.getClient() << std::endl;
    if (req.getType() == "POST") {
        std::cout << "Boundary: " << req.getBoundary() << std::endl;
        std::cout << "Body: " << req.getBody() << std::endl;
    }

}

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

int  Server::checkConnections()
{
  //poll allows to monitor multiple fd's for events
  int newEvents = poll(&_sockets[0], _sockets.size(), -1);
  if (newEvents  < 0) {
      std::perror("poll");
      exit(EXIT_FAILURE);
  }
  // Check if there's a new connection on the server socket
  if (_sockets[0].revents & POLLIN) { 
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

bool Server::isCGIRequest(const std::string& request) {
    // Check if the request method is "POST"
    size_t methodPos = request.find("POST");
    if (methodPos != std::string::npos) {
        // POST request, consider it as CGI
        return true;
    }

    // Check if the request URL starts with "/cgi-bin/"
        std::string url = request; // Skip space and slash
        size_t cgiPos = url.find("cgi-bin/");
        if (cgiPos == 0) {
            // Request URL starts with "/cgi-bin/", consider it as CGI
            return true;
        }
    

    // If not a POST request or does not start with "/cgi-bin/", it's not a CGI request
    return false;
}

void Server::executeCGIScript(const std::string& scriptPath, int clientSocket) {
    // Create pipes for inter-process communication
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    std::string scriptPath2 = "cgi-bin/script.cgi";
    (void)scriptPath;
    pid_t pid = fork();
    if (pid == 0) { // Child process
        // Close read end of the pipe
        close(pipefd[0]);

        // Redirect standard output to the write end of the pipe
        dup2(pipefd[1], STDOUT_FILENO);

        // Close the original write end of the pipe
        close(pipefd[1]);

        // Execute the CGI script

        execl(scriptPath2.c_str(), scriptPath2.c_str(), NULL);
        
        // If execl fails, it will continue here
        perror("execl");
        exit(EXIT_FAILURE);
    } else if (pid < 0) { // Fork failed
        perror("fork");
        exit(EXIT_FAILURE);
    } else { // Parent process
        // Close write end of the pipe
        close(pipefd[1]);

        // Read output from the pipe
        char buffer[1024];
        ssize_t bytesRead;
        std::string responseData;
        while ((bytesRead = read(pipefd[0], buffer, sizeof(buffer))) > 0) {
            responseData.append(buffer, bytesRead);
        }

        // Close read end of the pipe
        close(pipefd[0]);

        // Wait for the child process to finish
        int status;
        waitpid(pid, &status, 0);

        // Send HTTP response with CGI script output
        std::string response = "HTTP/1.1 200 OK\nContent-Type: text/html\n\n" + responseData;
        send(clientSocket, response.c_str(), response.size(), 0);
        close(clientSocket);
    }
}

void    Server::disconnectClient(int bytesRead, int i) {
    if (bytesRead == 0 || (errno != EWOULDBLOCK && errno != EAGAIN)) {
            // Connection closed or error occurred
            std::cout << "Client disconnected" << std::endl;
            close(_sockets[i].fd);
            _sockets.erase(_sockets.begin() + i);
    }
}

int 	Server::handleDelete(int client) {
    (void)client;
    //delete the soecified _object
    return 0;
}

int Server::handleUnknown(int fd) {
    std::string response = "HTTP/1.1 404 Not Found\nContent-Type: text/plain\n\n404 Not Found\n";
    send(fd, response.c_str(), response.size(), 0);
    return 0;
}

int 	Server::handleGet(int fd) {
    serveIndexHTML(fd);
    //send the specified _object
    std::cout << "send response to client" << std::endl;
    return 0;
}

int 	Server::handlePost(int fd) {
    if (_request.find(fd) != _request.end() && !_request[fd].isFullRequest()) {
        return 1; // Return 1 if the request is not a full request
    }
    const char* response = _request[fd].handleUpload();
    send(fd, response, strlen(response), 0);
    return 0;
}

void    Server::detectRequestType(int client) {
    int fd = _sockets[client].fd;
    std::string types[3] ={"GET", "POST", "DELETE"};
    int (Server::*requestFun[3])(int) = {&Server::handleGet, &Server::handlePost, &Server::handleDelete};
    std::string requestType = _request[fd].getType();

    for (int i = 0; i < 3; i++) {
        if (requestType == types[i]) {
            if ((this->*requestFun[i])(fd) == 1)
                return ; //unfinished POST request fx.
            break;
        }
    }
    _request.erase(fd);
    close(fd);
    _sockets.erase(_sockets.begin() + client);
}


void Server::handleRequest(int i) {
    int fd = _sockets[i].fd;
    char buffer[1024]; // Assuming a maximum request size of 1024 bytes
    ssize_t bytesRead = read(fd, buffer, sizeof(buffer));
    std::cout << "NEW REQUEST FROM FD: " << fd << std::endl;
    //std::cout << "BUFFER:\n" << buffer << "\nEND OF BUFFER" << std::endl;
    if (bytesRead <= 0) {
        disconnectClient(bytesRead, i);
    } else {
        if (_request.count(fd) == 0) {
            std::cout << "No request for client " << fd << std::endl;
            _request.insert(std::make_pair(fd, Request(buffer, fd)));
            printRequest(_request[fd]);
        }
        else if (_request[fd].getType() == "POST") {
            std::cout << "Pending POST request for client " << i << std::endl;
            _request[fd].appendToBody(buffer);
        }
        detectRequestType(i);
    }
}

void Server::serveIndexHTML(int clientSocket) {
    // Read contents of index.html file
    std::ifstream indexFile("index.html");
    if (!indexFile) {
        // Error opening index.html file
        std::cerr << "Failed to open index.html file" << std::endl;
        std::string response = "HTTP/1.1 404 Not Found\nContent-Type: text/plain\n\n404 Not Found\n";
        send(clientSocket, response.c_str(), response.size(), 0);
        return;
    }
    std::stringstream buffer;
    buffer << indexFile.rdbuf();
    std::string indexContent = buffer.str();

    // Send HTTP response with index.html content
    std::string response = "HTTP/1.1 200 OK\nContent-Type: text/html\n\n" + indexContent;
    send(clientSocket, response.c_str(), response.size(), 0);
}

void  Server::sendToClient(int i, char buffer[], ssize_t bytesRead) {
 // Parse HTTP request
        std::string request(buffer, bytesRead);
        // Check if the request is a CGI request
        if (isCGIRequest(request)) {
            // Execute CGI script
            executeCGIScript(request, _sockets[i].fd); // Pass client socket descriptor
        } else {
            // Handle non-CGI request (e.g., serve static files)
            // For simplicity, sending a static response
           if (request.find("GET / ") != std::string::npos) {
                serveIndexHTML(_sockets[i].fd);
            } else {
                // Handle non-index.html request
                // For simplicity, sending a 404 Not Found response
                std::string response = "HTTP/1.1 404 Not Found\nContent-Type: text/plain\n\n404 Not Found\n";
                send(_sockets[i].fd, response.c_str(), response.size(), 0);
                close(_sockets[i].fd);
                _sockets.erase(_sockets.begin() + i);
          }
        }
}

Server::Server()
{
  initSocket();
  while (true) {
    if (!checkConnections())
      continue ;
    for (int i = _sockets.size() - 1; i >= 1; --i) {
      if (_sockets[i].revents & POLLIN) // Check if there's data to read on client socket
        handleRequest(i);
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