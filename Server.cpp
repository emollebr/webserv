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


std::string Server::extractCGIScriptPath(const std::string& request) {
    // Parse the request URL to extract the script path
    size_t urlPos = request.find(" /");
    if (urlPos != std::string::npos) {
        std::string url = request.substr(urlPos + 2); // Skip space and slash
        size_t cgiPos = url.find("cgi-bin/");
        if (cgiPos == 0) {
            // Extract the script path from the URL
            size_t scriptStart = cgiPos + 8; // Skip "cgi-bin/"
            size_t scriptEnd = url.find_first_of(" ?", scriptStart);
            if (scriptEnd != std::string::npos) {
                std::string scriptPath = url.substr(scriptStart, scriptEnd - scriptStart);
                // Append the script filename to the CGI directory
                return "cgi-bin/" + scriptPath;
            }
        }
    }

    // If the request URL does not contain "/cgi-bin/", return an empty string
    return "";
}


bool Server::isCGIRequest(const std::string& request) {
    // Check if the request method is "POST"
    size_t postPos = request.find("POST");
    if (postPos == std::string::npos)
        postPos = request.find("GET");
    if (postPos != std::string::npos) {
        // Request method is "POST", now check if the URL starts with "/cgi-bin/"
        size_t urlStartPos = request.find("/", postPos); // Find the position of the first '/' after "POST"
        if (urlStartPos != std::string::npos) {
            size_t cgiPos = request.find("/cgi-bin/", urlStartPos);
            if (cgiPos == urlStartPos) {
                // Request URL starts with "/cgi-bin/", consider it as CGI
                std::cout << "skibidi" << std::endl;
                return true;
            }
        }
    }

    // If not a POST request or does not start with "/cgi-bin/", it's not a CGI request
    return false;
}

std::string extractMultipartBoundary(const std::string& request) {
    size_t boundaryPos = request.find("boundary=");
    if (boundaryPos != std::string::npos) {
        size_t boundaryEndPos = request.find("\r\n", boundaryPos);
        if (boundaryEndPos != std::string::npos) {
            std::string boundary = request.substr(boundaryPos + 9, boundaryEndPos - boundaryPos - 9);
            return boundary;
        }
    }
    return "";
}

std::vector<std::string> splitMultipartRequest(const std::string& request, const std::string& boundary) {
    std::vector<std::string> parts;
    size_t boundaryPos = request.find("--" + boundary);
    while (boundaryPos != std::string::npos) {
        size_t partStartPos = boundaryPos + boundary.length() + 2; // Skip boundary and CRLF
        size_t partEndPos = request.find("--" + boundary, partStartPos);
        if (partEndPos != std::string::npos) {
            parts.push_back(request.substr(partStartPos, partEndPos - partStartPos - 2)); // Exclude final CRLF
            boundaryPos = request.find("--" + boundary, partEndPos + boundary.length() + 2);
        } else {
            break; // Boundary not found, stop parsing
        }
    }
    return parts;
}

bool isFormField(const std::string& part) {
    return part.find("Content-Disposition: form-data") != std::string::npos &&
           part.find("filename") == std::string::npos;
}

std::string extractFormFieldData(const std::string& part) {
    size_t dataStartPos = part.find("\r\n\r\n") + 4; // Skip headers and CRLFs
    return part.substr(dataStartPos);
}

bool isFileField(const std::string& part) {
    return part.find("Content-Disposition: form-data") != std::string::npos &&
           part.find("filename") != std::string::npos;
}

void saveUploadedFile(const std::string& part) {
    // Extract filename from Content-Disposition header
    size_t filenamePos = part.find("filename=");
    if (filenamePos != std::string::npos) {
        size_t filenameStartPos = part.find("\"", filenamePos) + 1;
        size_t filenameEndPos = part.find("\"", filenameStartPos);
        std::string filename = part.substr(filenameStartPos, filenameEndPos - filenameStartPos);

        // Extract file data
        size_t dataStartPos = part.find("\r\n\r\n") + 4; // Skip headers and CRLFs
        std::string fileData = part.substr(dataStartPos);

        // Save file to disk
        std::ofstream file(filename.c_str(), std::ios::binary); // Convert std::string to const char*
        file << fileData;
        file.close();
    }
}

#include <unistd.h>
#include <sys/wait.h>

void Server::executeCGIScript(const std::string& scriptPath, int clientSocket, char** env) {
    // Create pipes for inter-process communication
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();
    if (pid == 0) { // Child process
        // Close read end of the pipe
        close(pipefd[0]);

        // Redirect standard output to the write end of the pipe
        dup2(pipefd[1], STDOUT_FILENO);

        // Close the original write end of the pipe
        close(pipefd[1]);

        char* argv[] = {(char*)scriptPath.c_str(), 0};

        execve(scriptPath.c_str(), argv, env);

        // If execve fails, it will continue here
        perror("execve");
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
    }
}

char** fillEnvironmentVariables(const std::string& formData, const std::string& boundary) {
    std::string queryString = "QUERY_STRING=" + formData;
    (void)boundary;

    char **env = new char*[2]; // Three elements: QUERY_STRING, CONTENT_TYPE, and null terminator

    // Copy the environment variable strings to the allocated memory
    env[0] = strdup(queryString.c_str());
    env[2] = 0; // Null terminator to indicate the end of the array
    return env;
}

void Server::handleRequest(int i) {
    char buffer[1024]; // Assuming a maximum request size of 1024 bytes
    ssize_t bytesRead = read(_sockets[i].fd, buffer, sizeof(buffer));
    std::cout << buffer << std::endl;
    if (bytesRead <= 0) {
        if (bytesRead == 0 || (errno != EWOULDBLOCK && errno != EAGAIN)) {
            // Connection closed or error occurred
            std::cout << "Client disconnected" << std::endl;
            close(_sockets[i].fd);
            _sockets.erase(_sockets.begin() + i);
        }
    } else {
        // Parse HTTP request
        std::string request(buffer, bytesRead);
        // Check if the request is a CGI request
        if (isCGIRequest(request)) {
            // Extract form data and files
            std::string formData;
            std::string boundary = extractMultipartBoundary(request);
            if (request.find("POST") != std::string::npos) {
                // For POST requests, extract form data from the request body
                size_t contentLengthPos = request.find("Content-Length:");
                if (contentLengthPos != std::string::npos) {
                    size_t bodyStartPos = request.find("\r\n\r\n", contentLengthPos);
                    if (bodyStartPos != std::string::npos) {
                        formData = request.substr(bodyStartPos + 4);
                    }
                }
            } else if (request.find("GET") != std::string::npos) {
                // For GET requests, extract form data from the query string
                size_t queryStartPos = request.find("?");
                if (queryStartPos != std::string::npos) {
                    formData = request.substr(queryStartPos + 1);
                }
            }
            // Execute CGI script
            std::string cgiScriptPath = extractCGIScriptPath(request);
            
            char **env = fillEnvironmentVariables(formData, boundary);
            executeCGIScript(cgiScriptPath, _sockets[i].fd, env);
            close(_sockets[i].fd); // Close client socket descriptor
        } else {
            // Handle non-CGI request
            // For simplicity, sending a static response
            if (request.find("form.html") != std::string::npos) {
                std::ifstream indexFile("form.html");
                std::stringstream buffer;
                buffer << indexFile.rdbuf();
                std::string indexContent = buffer.str();
                std::string response = "HTTP/1.1 200 OK\nContent-Type: text/html\n\n" + indexContent;
                send(_sockets[i].fd, response.c_str(), response.size(), 0);
                close(_sockets[i].fd);
            } else if (request.find("GET / ") != std::string::npos) {
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
}


void Server::serveIndexHTML(int clientSocket) {
    // Read contents of index.html file
    std::ifstream indexFile("index.html");
    if (!indexFile) {
        // Error opening index.html file
        std::cerr << "Failed to open index.html file" << std::endl;
        std::string response = "HTTP/1.1 404 Not Found\nContent-Type: text/plain\n\n404 Not Found\n";
        send(clientSocket, response.c_str(), response.size(), 0);
        close(clientSocket);
        return;
        return;
    }
    std::stringstream buffer;
    buffer << indexFile.rdbuf();
    std::string indexContent = buffer.str();

    // Send HTTP response with index.html content
    std::string response = "HTTP/1.1 200 OK\nContent-Type: text/html\n\n" + indexContent;
    send(clientSocket, response.c_str(), response.size(), 0);
    close(clientSocket);
}

void  Server::sendToClient(int i) {
  //char buffer[100];
  //std::cout << "Received: " << buffer << "\n" << std::endl;
  (void)i;
}

void Server::cleanup() {
    close(_sockets[0].fd); // Close the server socket
}

Server::Server()
{
  initSocket();
  while (true) {
    checkConnections();
    for (int i = _sockets.size() - 1; i >= 1; --i) {
      if (_sockets[i].revents & POLLIN) { // Check if there's data to read on client socket
        handleRequest(i);
        break;
      } else {
        sendToClient(i);
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
    cleanup();
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