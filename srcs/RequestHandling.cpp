#include "Request.hpp"

int    Request::detectRequestType() {
    std::string methods[3] ={"GET", "POST", "DELETE"};
    int (Request::*requestFun[3])() = {&Request::_handleGet, &Request::_handlePost, &Request::_handleDelete};

    for (int i = 0; i < 3; i++) {
        if (_method == methods[i]) {
            if ((this->*requestFun[i])() != 0)
                return 1; //unfinished POST request fx.
            return 0;
        }
    }
   _sendStatusPage(405, "405 Method Not Allowed"); //UNKNOWN METHOD
    return 0;
}


int 	Request::_handleDelete() {
    std::string response;
    if (std::remove(_path.c_str()) != 0) {
        std::cerr << "Error deleting file: " << strerror(errno) << std::endl;
       return _sendStatusPage(400, "404 Not Found");
    }
    else
        return _sendStatusPage(200, "File successfully deleted");
}


void freeEnvironmentVariables(char** env) {
    // Iterate through the environment array and free each string
    for (int i = 0; env[i] != NULL; ++i) {
        delete[] env[i];
    }

    // Free the array itself
    delete[] env;
}

char** fillEnvironmentVariables(const std::string& formData) {
    std::string queryString = "QUERY_STRING=" + formData;
    std::cout << queryString << std::endl;

    char **env = new char*[2];

    // Allocate memory for the QUERY_STRING string and copy the content
    env[0] = new char[queryString.length() + 1]; // +1 for null terminator
    strcpy(env[0], queryString.c_str());

    // Set the null terminator
    env[1] = NULL;

    return env;
}


int 	Request::_handleGet() {
    char **env = fillEnvironmentVariables(_body);
    if (isCGIRequest())
    {
       executeCGIScript(env);
       freeEnvironmentVariables(env);
       return 0;
    }
    if (is_directory(_path.c_str())) {
        std::string index = _location.getIndex();
        if (!index.empty())
            _path += index;
        else if (_location.getAutoindex())
            return _handleListFiles(_path);
        else
            return _sendStatusPage(403, "403 Forbidden: i dont know if its the right status code??");
    }

    std::ifstream file(_path.c_str(), std::ios::binary); // Open the file
    if (!file) { 
        // Error opening index.html file
        std::cerr << "Failed to open " << _path << " file" << std::endl;
        return _sendStatusPage(404, "404 Not Found");
    }
    // Get the size of the file
    file.seekg(0, std::ios::end); // Move file pointer to the end
    _fileSize = file.tellg();
    file.close();
    
    // Send HTTP response headers
    std::ostringstream responseHeader;
    responseHeader << "HTTP/1.1 " << intToStr(_statusCode) << " \nContent-Type: " << getMimeType(_path) << "\nContent-Length: " << _fileSize << "\n\n";
    sendResponse(responseHeader.str().c_str(), responseHeader.str().size(), MSG_MORE);
    _pendingResponse = 1;
    return _pendingResponse;
}

int    Request::createResponse() {    
    // Read file in chunks and send each chunk if file size exceeds buffer size
    std::ifstream file(_path.c_str(), std::ios::binary);
    if (!file) { 
        // Error opening index.html file
        std::cerr << "Failed to open " << _path << " file" << std::endl;
        _sendStatusPage(500, "500 Internal Error: failed to send requested content");
        return 0;
    }

    char buffer[BUF_SIZE];
    file.seekg(_bytesSent); // Move file pointer to the correct position
    size_t bytesRead = file.read(buffer, (_fileSize - _bytesSent > BUF_SIZE) ? BUF_SIZE : _fileSize - _bytesSent).gcount();
    
    if (bytesRead == 0) {
        std::cout << "sendResponse: error: Requested file is empty" << std::endl;
        return _sendStatusPage(500, "500 Internal Error: failed to send requested content");
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
        return _sendStatusPage(500, "500 Internal Error: failed to send requested content");
    }
    
    return ((_bytesSent < _fileSize) ? 1 : 0);
}

int Request::sendResponse(const char* response, size_t size, int flag) {
    int status = send(client, response, size, flag);
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

int 	Request::_handlePost() {
    char **env = fillEnvironmentVariables(_body);
    if (isCGIRequest())
    {
       executeCGIScript(env);
       freeEnvironmentVariables(env);
       return 0;
    }
    if (!_isFullRequest()) {
        return KEEP_ALIVE; // Return 1 if the request is not a full request
    }
    const char* uploadFile = _createFileName();
    
    // Write file data to disk
    int	fd;
	if ( (fd = open(uploadFile, O_RDWR|O_CREAT, S_IRWXU|S_IRWXO|S_IRWXG)) == -1 ) {
        _sendStatusPage(500, "500 Internal Error: failed to save requested content");
    }

    std::ofstream file(uploadFile, std::ios::out | std::ios::trunc | std::ios::binary);
    if ( !file.is_open() ) {
		_sendStatusPage(500, "500 Internal Error: failed to save requested content");
	}
    if (file.write(reinterpret_cast<const char*>(_body.c_str()), _body.size() - _boundary.size() - 7)) {
		_sendStatusPage(200, "Upload successful");
	}
    close(fd);
    file.close();
    return 0;
}