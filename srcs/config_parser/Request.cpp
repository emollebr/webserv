#include "common.hpp"

Request::Request(char *buffer, int client, int bytesRead, ServerConfig config) : _pendingResponse(0), _bytesSent(0),  _errorPages(config.getErrorPages()), _redirStatus(0), client(client) {
    
    std::istringstream iss(buffer);
    std::string line;
    int bytesProcessed = 0;

    iss >> _method >> _object >> _protocol;
    bytesProcessed += _method.size() + _object.size() + _protocol.size() + 3;
    std::getline(iss, line);

    while (std::getline(iss, line)  && line != "\r") {
        bytesProcessed += line.size() + 1;
        size_t pos = line.find(": ");
        if (pos != std::string::npos) {
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 2);
            _headers[key] = value;
        }
    }

    std::vector<std::string> tokens = tokenizePath(_object);
    _findLocation(tokens, config.getLocations(), 0);
  
    if (_method == "POST") {
        _boundary = _headers["Content-Type"];
        _boundary.erase(0, 30); // 30 characters to boundary

        //get content headers
        while (std::getline(iss, line)  && line == "\r") {
            continue;
        }
        while (std::getline(iss, line) && line != "\r") {
            size_t pos = line.find(": ");
            if (pos != std::string::npos) {
                std::string key = line.substr(0, pos);
                std::string value = line.substr(pos + 2);
                _headers[key] = value;
            }
        }

        //save the request body
        char *bodyStart = std::strstr(buffer, "\r\n\r\n");
        if (bodyStart != NULL) {
            if (_object.find("cgi-bin") == std::string::npos) {
                 _validateContentHeaders(_location.getBodySize());
                // Skip an additional 3 occurrences of "\r\n"
                for (int i = 0; i < 5; ++i) {
                    bodyStart = std::strstr(bodyStart + 2, "\r\n");
                    if (bodyStart == NULL) {
                        break; // Stop if we reach the end of the buffer
                    }
                }
            }

            if (bodyStart != NULL) {
                bodyStart += 4; // Move past the last "\r\n"
                size_t bodySize = bytesRead - (bodyStart - buffer); // Calculate the size of the binary data
                _body.append(bodyStart, bodySize); // Append the binary data
            }
        }
        
        _bytesReceived = bytesRead - bytesProcessed;
        _fullRequest = (_bytesReceived < _contentLength) ? false : true;
    }
    return ;
}

void Request::_validateContentHeaders(size_t maxBodySize) {
    //validate content headers headers/handle errors
    std::map<std::string, std::string>::const_iterator it = _headers.find("Content-Disposition");
    if (it == _headers.end()) {
        _sendStatusPage(400, "400 Bad Request: Missing 'Content-Disposition' header for POST request");
        throw MissingRequestHeaderException();
    }  
    it = _headers.find("Content-Length");
    if (it == _headers.end()) {
        _sendStatusPage(400, "400 Bad Request: Missing 'Content-Length' header for POST request");
        throw MissingRequestHeaderException();
    }
    std::istringstream ss(it->second);
    ss >> _contentLength;
    if (_contentLength > maxBodySize) {
        std::string msg = "413 Payload Too Large: Content-Length exceeds limit of " + intToStr(maxBodySize) + " bytes";
        _sendStatusPage(413, msg);
        throw MaxBodySizeExceededException();
    }
}


bool Request::isCGIRequest() {
// Check if the request URL starts with "/cgi-bin/"
    std::cout << "OBJECTcgi:" << _object << std::endl;
    std::string url = _object; // Skip space and slash
    size_t cgiPos = url.find("cgi-bin/");
    std::cout << "IS CGI\n";
    if (cgiPos != std::string::npos) {
        // Request URL starts with "/cgi-bin/", consider it as CGI
        std::cout << "IS CGI\n";
        return true;
    }
    // If does not start with "/cgi-bin/", it's not a CGI request
    return false;
}


void Request::executeCGIScript(const std::string& scriptPath, char** env) {
    // Create pipes for inter-process communication
    

    std::string path = scriptPath;
    if (!path.empty() && path[0] == '/')
        path = path.substr(1);
    std::cout << "path: " << scriptPath << std::endl;

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

        char* argv[] = {(char*)path.c_str(), 0};

        execve(path.c_str(), argv, env);

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
        bytesRead = read(pipefd[0], buffer, sizeof(buffer));
        responseData.append(buffer, bytesRead);
        // Close read end of the pipe
        close(pipefd[0]);

        // Wait for the child process to finish
        int status;
        waitpid(pid, &status, 0);

        // Send HTTP response with CGI script output
        std::string response = "HTTP/1.1 200 OK\nContent-Type: text/html\n\n" + responseData;
        sendResponse(response.c_str(), response.size(), 0);
    }
}


void    Request::pendingPostRequest(char* buffer, int bytesRead) {
    _body.append(std::string(buffer, bytesRead));
    _bytesReceived += bytesRead;
    _fullRequest = (_bytesReceived < _contentLength) ? false : true;
}

bool Request::_fileExists(std::string filename) {
    std::ifstream file(filename.c_str());
    return file.good();
}

int		Request::_sendStatusPage(int statusCode, std::string msg) {
    std::map<unsigned int, std::string>::iterator it = _errorPages.find(statusCode);
    if (it != _errorPages.end()) { //check for default error pages
        _object = it->second;
        return (_handleGet());
    }
    else { //no default || success
        if (_redirStatus != 0)
            statusCode = _redirStatus;
        std::stringstream response;
        response << "HTTP/1.1 " + intToStr(statusCode) + "\r\nContent-Type: text/plain\r\n" << msg.size() + 1 << "\r\n\r\n" + msg + "\r\n";
        return (sendResponse(response.str().c_str(), response.str().size(), 0));
    }
}

// Function to generate a new filename if the original filename already exists
std::string Request::_generateNewFilename(const std::string& originalFilename) {
    std::string newFilename = originalFilename;
    std::string extension;
    size_t dotPos = originalFilename.find_last_of('.');
    if (dotPos != std::string::npos) {
        extension = originalFilename.substr(dotPos);
        newFilename = originalFilename.substr(0, dotPos);
    }

    int counter = 1;
    std::string num = "(" + intToStr(counter) + ")";
    // Keep incrementing counter and appending it to the filename until a unique filename is found
    while (_fileExists(newFilename.c_str() + num + extension)) {
        counter++;
        num = "(" + intToStr(counter) + ")";
    }

    newFilename = newFilename  + num + extension;
    return newFilename;
}


const char* Request::_createFileName() {

    std::string content = _headers["Content-Disposition"];
    size_t filename_start = content.find("filename=");
    
    size_t startPos = filename_start + 10;
    size_t endPos = content.find_last_not_of(" \"\t\r\n") + 1;
    std::string filename = "database/uploads/" + content.substr(startPos, endPos - startPos);    

   if (_fileExists(filename.c_str())) {
      std::string tmp = _generateNewFilename(filename);
      filename = tmp;
    }

    char* result = strdup(filename.data()); // Allocate memory and copy the data
    return result;
}

const char* Request::MaxBodySizeExceededException::what() const throw() {
    return "Max body size exceeded";
}

const char* Request::MissingRequestHeaderException::what() const throw() {
    return "Missing request header";
}

const char* Request::MethodNotAllowedException::what() const throw() {
    return "Method not allowed in this location";
}

const char* Request::EmptyRequestedFileException::what() const throw() {
    return "Requested file is empty";
}

const char* Request::FileReadException::what() const throw() {
    return "Failed to read requested file";
}

std::ostream &operator<<(std::ostream &str, Request &rp)
{
    str << "Client FD: " << rp.client << std::endl;
    str << "Method: " << rp.getMethod() << std::endl;
    str << "Path: " << rp.getObject() << std::endl;
    str << "Protocol: " << rp.getProtocol() << std::endl;
    str << "Headers: " << std::endl;
    std::map<std::string, std::string> headers = rp.getHeaders();
    for (std::map<std::string, std::string>::iterator it = headers.begin(); it != headers.end(); ++it) {
        str << "\t" << it->first << ": " << it->second << std::endl;
    }
    if (rp.getMethod() == "POST")
        str << "Body: " << rp.getBody() <<std::endl;
    return (str);
};