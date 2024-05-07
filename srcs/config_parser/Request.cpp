#include "common.hpp"

Request::Request(char *buffer, int client, int bytesRead, ServerConfig config) : _pendingResponse(0), _bytesSent(0),  _errorPages(config.getErrorPages()), _redirStatus(0), client(client) {
    
    std::istringstream iss(buffer);
    std::string line;
    int bytesProcessed = 0;

    //Parse method and PATH
    iss >> _method >> _object >> _protocol;
    _path = _object;
    bytesProcessed += _method.size() + _path.size() + _protocol.size() + 3;
    std::getline(iss, line);

    //Parse headers
    while (std::getline(iss, line)  && line != "\r") {
        bytesProcessed += line.size() + 1;
        size_t pos = line.find(": ");
        if (pos != std::string::npos) {
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 2);
            _headers[key] = value;
        }
    }

    std::cout << "ReqCon: URL is " << _path << std::endl;

    //Get CGI extension OR locations
    if (_getCGIPath(config.getCGIExtention()) == 0) {
        std::cout << "ReqCon looking for location" << std::endl;
        //Find appropriate location
        std::vector<std::string> tokens = tokenizePath(_path);
        if (tokens.size() == 0 || !_findLocation(tokens, config.getLocations(), 0)) {
            //No match found, set default location
            _getDefaultLocation(config.getLocations());
        }
    }
    std::cout << "_cgipath is " << _cgi_path << std::endl;

    //TO DO: Adapt to work with CGI
    if (_method == "POST") {
        _boundary = _headers["Content-Type"];
        _boundary.erase(0, 30); // 30 characters to boundary
        _parseContentHeaders(buffer, iss.tellg());
        _parseRequestBody(buffer, bytesRead);
        _bytesReceived = bytesRead - bytesProcessed;
        _fullRequest = (_bytesReceived < _contentLength) ? false : true;
    }
    else if (_method == "GET" && _path.find('&') != std::string::npos) {
    // Find the position of '?' in the request object
        std::cout << "ONJECT CONSTRUCTOR: " << _path << std::endl;
        size_t pos = _path.find('?');
        if (pos != std::string::npos) {
            // Extract the query string after '?'
            std::string queryString = _path.substr(pos + 1);
            _path = _path.substr(0, pos);
            //_filePath = finishPath(_path);

            // Split the query string by '&' to separate key-value pairs
            std::istringstream queryStream(queryString);
            std::string keyValue;
            while (std::getline(queryStream, keyValue, '&')) {
                // Split each key-value pair by '='
                size_t equalPos = keyValue.find('=');
                if (equalPos != std::string::npos) {
                    std::string key = keyValue.substr(0, equalPos);
                    std::string value = keyValue.substr(equalPos + 1);

                    // URL decode the key and value
                    key = urlDecode(key);
                    value = urlDecode(value);

                    // Store the key-value pair in _body
                    _body += key + "=" + value + "\n";
                }
            }
        }
    }
    return ;
}

std::string Request::urlDecode(const std::string& str) {
    std::ostringstream decodedStream;
    for (size_t i = 0; i < str.size(); ++i) {
        if (str[i] == '%') {
            if (i + 2 < str.size() && isxdigit(str[i + 1]) && isxdigit(str[i + 2])) {
                int hexValue;
                std::istringstream hexStream(str.substr(i + 1, 2));
                hexStream >> std::hex >> hexValue;
                decodedStream << static_cast<char>(hexValue);
                i += 2;
            } else {
                // Invalid percent encoding, ignore '%'
                decodedStream << '%';
            }
        } else if (str[i] == '+') {
            // Convert '+' to space
            decodedStream << ' ';
        } else {
            // Copy other characters as is
            decodedStream << str[i];
        }
    }
    return decodedStream.str();
}

int  Request::_getCGIPath(std::map<std::string, std::string> cgi_map) {
        std::map<std::string, std::string>::iterator it;
    for (it = cgi_map.begin(); it != cgi_map.end(); ++it) {
        std::cout << it->first << " => " << it->second << std::endl;
    }

    size_t dotPos = _path.find_last_of('.');
    if (dotPos != std::string::npos) {
        std::string ext = _path.substr(dotPos); // Extract the extension
	    std::map<std::string, std::string>::iterator cgi_it = cgi_map.find(ext);
        std::cout << "getCGIPath extension: " << ext << std::endl;
        if (cgi_it != cgi_map.end()) {
            _cgi_path = cgi_it->second;
            return 1;
        }
    }
    return 0;
}

void    Request::_parseContentHeaders(char *buffer, std::streampos pos) {
    std::string line;
    std::istringstream iss(buffer);
    iss.seekg(pos);
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
}

void Request::_parseRequestBody(char *buffer, int bytesRead) {
  
    //save the request body
    char *bodyStart = std::strstr(buffer, "\r\n\r\n");
    if (bodyStart != NULL) {
        if (_path.find("cgi-bin") == std::string::npos) {
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
    

}

//validate content headers /handle errors
void Request::_validateContentHeaders(size_t maxBodySize) {
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


// Check if the request URL starts with "/cgi-bin/"
bool Request::isCGIRequest() {
    size_t cgiPos = _path.find("cgi-bin/");
    if (cgiPos != std::string::npos) {
        // Request URL starts with "/cgi-bin/", consider it as CGI
        std::cout << "IS CGI\n";
        if (!_cgi_path.empty())
            return true;
    }
    // If does not start with "/cgi-bin/", it's not a CGI request
    return false;
}

#include <fcntl.h> // Include for non-blocking I/O
#include <fcntl.h> // Include for non-blocking I/O

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
    time_t startTime = time(0);

        // Wait for the child process to finish or timeout
    bool timeout = false;
    pid_t pidWait = 0;
    pid_t pid = fork();
    if (pid == 0) { // Child process
        // Close read end of the pipe
        close(pipefd[0]);

        // Redirect standard output to the write end of the pipe
        dup2(pipefd[1], STDOUT_FILENO);

        // Close the original write end of the pipe
        close(pipefd[1]);

        char* argv[] = {(char*)path.c_str(), 0};

        // Execute the CGI script
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
        

        // Set a timeout duration (e.g., 3 seconds)
        const time_t timeoutDuration = 5; // 3 seconds

        while (pidWait == 0 && (time(0) - startTime) <= timeoutDuration)
            pidWait = waitpid(pid, NULL, WNOHANG);
        if (pidWait == 0) {
            timeout = true;
            // Kill the child process
            kill(pid, SIGKILL);
        } else if (pidWait == -1) {
            perror("waitpid");
            exit(EXIT_FAILURE);
        }

    }
    std::string responseData;

        if (timeout == false){
        char buffer[1024];
        ssize_t bytesRead;

        bytesRead = read(pipefd[0], buffer, sizeof(buffer));
        if (bytesRead > 0) {
            responseData.append(buffer, bytesRead);
        }
        close(pipefd[0]);
    }
        if (timeout == true) {
        const std::string timeoutResponse = "HTTP/1.1 200 OK\nContent-Type: text/html\n\nThe server took too long to process the request.";
        send(client, timeoutResponse.c_str(), timeoutResponse.size(), 0);
        //close(clientSocket);
    } else {
        // Send HTTP response with CGI script output
        std::string response = "HTTP/1.1 200 OK\nContent-Type: text/html\n\n" + responseData;
        send(client, response.c_str(), response.size(), 0);
        //close(clientSocket);
    }
}

//appends the new request body to the pending request
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
        _path = it->second;
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

std::ostream &operator<<(std::ostream &str, Request &rp)
{
    str << "Client FD: " << rp.client << std::endl;
    str << "Method: " << rp.getMethod() << std::endl;
    str << "Path: " << rp.getPath() << std::endl;
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