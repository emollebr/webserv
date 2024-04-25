#include "Request.hpp"

int    Request::detectRequestType() {
    std::string methods[3] ={"GET", "POST", "DELETE"};
    int (Request::*requestFun[3])() = {&Request::_handleGet, &Request::_handlePost, &Request::_handleDelete};
    std::string requestMethod = _method;

    for (int i = 0; i < 3; i++) {
        if (requestMethod == methods[i]) {
            if ((this->*requestFun[i])() != 0)
                return 1; //unfinished POST request fx.
            return 0;
        }
    }
    _handleUnknown();
    return 0;
}

int 	Request::_handleDelete() {
    std::string response;
    std::cout << _filePath << std::endl;
    if (std::remove(_filePath.c_str()) != 0) {
        std::cerr << "Error deleting file: " << strerror(errno) << std::endl;
        response = _getStatusResponse("404 Not Found", "404 Not Found");
    }
    else
        response = _getStatusResponse("200 OK", "File successfully deleted");
    sendResponse(response.c_str(), response.size(), 0);
    return 0;
}

int Request::_handleUnknown() {
    std::string response = _getStatusResponse("405 Method Not Allowed", "405 Method Not Allowed");
    sendResponse(response.c_str(), response.size(), 0);
    return 0;
}

int 	Request::_handleGet() {
    if (_object == "/list_files") {
        handleListFiles(client);
        return 0;
    }

    std::ifstream file(_filePath.c_str(), std::ios::binary); // Open the file
    if (!file) { 
        // Error opening index.html file
        std::cerr << "Failed to open " << _filePath << " file" << std::endl;
        std::string response = _getStatusResponse("404 Not Found", "404 Not Found");
        sendResponse(response.c_str(), response.size(), 0);
        return 0;
    }
    // Get the size of the file
    file.seekg(0, std::ios::end); // Move file pointer to the end
    _fileSize = file.tellg();
    file.close();
    
    // Send HTTP response headers
    std::ostringstream responseHeader;
    responseHeader << "HTTP/1.1 200 OK\nContent-Type: " << getMimeType(_filePath) << "\nContent-Length: " << _fileSize << "\n\n";
    sendResponse(responseHeader.str().c_str(), responseHeader.str().size(), MSG_MORE);
    _pendingResponse = 1;
    return _pendingResponse;
}

int    Request::createResponse() {
    std::string error_response = _getStatusResponse("500 Internal Server Error", "500 Internal Error: failed to send requested content");
    
    // Read file in chunks and send each chunk if file size exceeds buffer size
    std::ifstream file(_filePath.c_str(), std::ios::binary);
    if (!file) { 
        // Error opening index.html file
        std::cerr << "Failed to open " << _filePath << " file" << std::endl;
        std::string response = _getStatusResponse("404 Not Found", "404 Not Found");
        sendResponse(response.c_str(), response.size(), 0);
        return 0;
    }

    char buffer[BUF_SIZE];
    file.seekg(_bytesSent); // Move file pointer to the correct position
    size_t bytesRead = file.read(buffer, (_fileSize - _bytesSent > BUF_SIZE) ? BUF_SIZE : _fileSize - _bytesSent).gcount();
    
    if (bytesRead == 0) {
        sendResponse(error_response.c_str(), error_response.size(), 0);
        std::cout << "sendResponse: error: Requested file is empty" << std::endl;
        return 0;
    }

    if (file) {
        file.close();
        int flag = (_fileSize - _bytesSent > 0) ? 0 : MSG_MORE;
        //int ret = sendResponse(buffer, bytesRead, flag);
        int status = send(client, buffer, bytesRead, flag);
        if (status == -1) {
            if (g_signal_received == SIGPIPE) {
                perror("SIGPIPE");
                g_signal_received = 0;
                return CONNECTION_CLOSED;
            } else {
                perror("send");
                return SEND_ERROR;
            }
        }
        else if (status == 0)
            return CONNECTION_CLOSED;
        _bytesSent += status;
    } else {
        sendResponse(error_response.c_str(), error_response.size(), 0);
        std::cout << "createResponse: error: Failed to read requested file" << std::endl;
        return 0;
    }
    
    return ((_bytesSent < _fileSize) ? 1 : 0);
}

int Request::sendResponse(const char* response, size_t size, int flag) {
    int status = send(client, response, size, flag);
    std::cout << "Send: status: " << status << ", client fd: " << client << ", response: " << response << std::endl;
    if (status == -1) {
        if (g_signal_received == SIGPIPE) {
            perror("SIGPIPE");
            g_signal_received = 0;
            return CONNECTION_CLOSED;
        } else {
            perror("send");
            return SEND_ERROR;
        }
    }
    else if (status == 0)
        return CONNECTION_CLOSED;
    else if (status < static_cast<int>(size)) {
        _bytesSent += status;
        std::cout << "send: error: failed to send full response to client: " << client << std::endl;  
        return SEND_ERROR; 
    }
    return SUCCESS;
}

char** fillEnvironmentVariables(const std::string& formData) {
    std::string queryString = "QUERY_STRING=" + formData;
    std::cout << queryString << std::endl;
    char **env = new char*[3]; // Three elements: QUERY_STRING, CONTENT_TYPE, and null terminator
    // Copy the environment variable strings to the allocated memory
    env[0] = strdup(queryString.c_str());
    env[1] = 0;
    env[2] = 0;
    return env;
}


int 	Request::_handlePost() {
    std::cout << "OBJECT: " << _object << std::endl;
    char **env = fillEnvironmentVariables(_body);
    std::cout << "OBJECT: " << _object << std::endl;
    if (isCGIRequest())
    {
       executeCGIScript(_object, client, env);
       return 0;
    }
    if (!_isFullRequest()) {
        return KEEP_ALIVE; // Return 1 if the request is not a full request
    }
    const char* filepath = _createFileName();
    std::string response = _getStatusResponse("500 Internal Server Error", "Error saving file to database");
    
    // Write file data to disk
    int	fd;
	if ( (fd = open(filepath, O_RDWR|O_CREAT, S_IRWXU|S_IRWXO|S_IRWXG)) == -1 ) {
		std::cout << "Error: could not open file \"" << filepath << "\" with execution rights " << std::endl;
        return sendResponse(response.c_str(), response.size(), 0);
    }

    std::ofstream file(filepath, std::ios::out | std::ios::trunc | std::ios::binary);
    if ( !file.is_open() ) {
		std::cout << "Error: open file \"" << filepath << "\" failed" << std::endl;
		return sendResponse(response.c_str(), response.size(), 0);
	}
    if (file.write(reinterpret_cast<const char*>(_body.c_str()), _body.size() - _boundary.size() - 7)) {
		response = _getStatusResponse("200 OK", "Upload successful");
	}
    close(fd);
    file.close();

    return sendResponse(response.c_str(), response.size(), 0);
}
