#include "Server.hpp"

int    Request::detectRequestType() {
    std::string methods[3] ={"GET", "POST", "DELETE"};
    int (Request::*requestFun[3])() = {&Request::_handleGet, &Request::_handlePost, &Request::_handleDelete};
    std::string requestMethod = _method;

    for (int i = 0; i < 3; i++) {
        if (requestMethod == methods[i]) {
            if ((this->*requestFun[i])() != 0)
                return 1; //unfinished POST request fx.
            break;
        }
    }
    return 0;
}

int 	Request::_handleDelete() {
    std::string response;
    std::cout << _filePath << std::endl;
    if (std::remove(_filePath.c_str()) != 0) {
        std::cerr << "Error deleting file: " << strerror(errno) << std::endl;
        response = "HTTP/1.1 404 Not Found\nContent-Type: text/plain\n\n404 Not Found\n";
    }
    else
        response = "HTTP/1.1 200 OK\nContent-Type: text/plain\n\nFile successfully deleted\n";
    send(client, response.c_str(), response.size(), 0);
    return 0;
}

int Request::_handleUnknown() {
    std::string response = "HTTP/1.1 405 Method Not Allowed\nContent-Type: text/plain\n\n405 Method Not Allowed\n";
    send(client, response.c_str(), response.size(), 0);
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
        std::string response = "HTTP/1.1 404 Not Found\nContent-Type: \n\n404 Not Found\n";
        send(client, response.c_str(), response.size(), 0);
        return 0;
    }
    // Get the size of the file
    file.seekg(0, std::ios::end); // Move file pointer to the end
    _fileSize = file.tellg();
    file.close();
    
    // Send HTTP response headers
    std::ostringstream responseHeader;
    responseHeader << "HTTP/1.1 200 OK\nContent-Type: " << getMimeType(_filePath) << "\nContent-Length: " << _fileSize << "\n\n";
    send(client, responseHeader.str().c_str(), responseHeader.str().size(), MSG_MORE);

    _pendingResponse = sendResponse();
    return _pendingResponse;
}

int    Request::sendResponse() {
    std::string error_response = "HTTP/1.1 500 Internal Server Error\r\nContent-Type: text/plain\r\nContent-Length: 32\r\n\r\nError sending requested content";
    
    // Read file in chunks and send each chunk if file size exceeds buffer size
    std::ifstream file(_filePath.c_str(), std::ios::binary);
    if (!file) { 
        // Error opening index.html file
        std::cerr << "Failed to open " << _filePath << " file" << std::endl;
        std::string response = "HTTP/1.1 404 Not Found\nContent-Type: text/plain\n\n404 Not Found\n";
        send(client, response.c_str(), response.size(), 0);
        return 0;
    }
    char buffer[BUF_SIZE];
    file.seekg(_bytesSent); // Move file pointer to the correct position
    size_t bytesRead = file.read(buffer, (_fileSize - _bytesSent > BUF_SIZE) ? BUF_SIZE : _fileSize - _bytesSent).gcount();
    if (file) {
        file.close();
        int flag = (_fileSize - _bytesSent > 0) ? 0 : MSG_MORE;
        int ret = send(client, buffer, bytesRead, flag);
        if (ret == -1) {
            std::cerr << "Failed to send to client" << std::endl;
            send(client, error_response.c_str(), error_response.size(), 0);
            return 0;
        }
        _bytesSent += ret;
    } else {
        std::cerr << "Failed to read from file" << std::endl;
        send(client, error_response.c_str(), error_response.size(), 0);
        return 0;
    }
    
    return ((_bytesSent < _fileSize) ? 1 : 0);
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
        return 1; // Return 1 if the request is not a full request
    }
    const char* filepath = _createFileName();
    const char* response = "HTTP/1.1 500 Internal Server Error\r\nContent-Type: text/plain\r\nContent-Length: 30\r\n\r\nError saving file to database";
    
    // Write file data to disk
    int	fd;
	if ( (fd = open(filepath, O_RDWR|O_CREAT, S_IRWXU|S_IRWXO|S_IRWXG)) == -1 ) {
		std::cout << "Error: could not open file \"" << filepath << "\" with exeution rights " << std::endl;
        return send(client, response, strlen(response), 0);
    }

    std::ofstream file(filepath, std::ios::out | std::ios::trunc | std::ios::binary);
    if ( !file.is_open() ) {
		std::cout << "Error: open file \"" << filepath << "\" failed" << std::endl;
		return send(client, response, strlen(response), 0);
	}
    if (!file.write(reinterpret_cast<const char*>(_body.c_str()), _body.size() - _boundary.size() - 7)) {
		file.close();
		return send(client, response, strlen(response), 0);
	}
    close(fd);
    file.close();

    // Send HTTP response indicating success
    response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 18\r\n\r\nUpload successful";
    send(client, response, strlen(response), 0);
    return 0;
}
