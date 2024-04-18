#include "Server.hpp"

int    Request::detectRequestType() {
    std::string methods[3] ={"GET", "POST", "DELETE"};
    int (Request::*requestFun[3])() = {&Request::handleGet, &Request::handlePost, &Request::handleDelete};
    std::string requestMethod = _method;

    for (int i = 0; i < 3; i++) {
        if (requestMethod == methods[i]) {
            if ((this->*requestFun[i])() == 1)
                return 1; //unfinished POST request fx.
            break;
        }
    }
    return 0;
}

int 	Request::handleDelete() {
    std::string response;
    std::string path = finishPath(_object);
    std::cout << path << std::endl;
    if (std::remove(path.c_str()) != 0) {
        std::cerr << "Error deleting file: " << strerror(errno) << std::endl;
        response = "HTTP/1.1 404 Not Found\nContent-Type: text/plain\n\n404 Not Found\n";
    }
    else
        response = "HTTP/1.1 200 OK\nContent-Type: text/plain\n\nFile successfully deleted\n";
    send(_client, response.c_str(), response.size(), 0);
    return 0;
}

int Request::handleUnknown() {
    std::string response = "HTTP/1.1 405 Method Not Allowed\nContent-Type: text/plain\n\n405 Method Not Allowed\n";
    send(_client, response.c_str(), response.size(), 0);
    return 0;
}

int 	Request::handleGet() {
    if (_object == "/list_files") {
        handleListFiles(_client);
        return 0;
    }

    std::string path = finishPath(_object);
    std::ifstream file(path.c_str(), std::ios::binary);
    std::cout << "In GET handling: serving: " << path << std::endl;
    if (!file) {
        // Error opening index.html file
        std::cerr << "Failed to open " << path << " file" << std::endl;
        std::string response = "HTTP/1.1 404 Not Found\nContent-Type: text/plain\n\n404 Not Found\n";
        send(_client, response.c_str(), response.size(), 0);
        return 0;
    }
    // Get the size of the file
    file.seekg(0, std::ios::end); // Move file pointer to the end
    off_t fileSize = file.tellg();
    file.seekg(0, std::ios::beg); // Reset file pointer to the beginning
    std::cout << "filesize to send: " << fileSize << std::endl;
    
    // Send HTTP response headers
    int flag = (fileSize < BUF_SIZE) ? 0 : MSG_MORE;
    std::ostringstream responseHeader;
    responseHeader << "HTTP/1.1 200 OK\nContent-Type: " << getMimeType(path) << "\nContent-Length: " << fileSize << "\n\n";
    send(_client, responseHeader.str().c_str(), responseHeader.str().size(), flag);

    // Read file in chunks and send each chunk if file size exceeds buffer size
    char buffer[BUF_SIZE];
    while (fileSize > 0) {
        size_t bytesRead = file.read(buffer, (fileSize > BUF_SIZE) ? BUF_SIZE : fileSize).gcount();
        if (bytesRead > 0) {
            fileSize -= bytesRead;
            flag = (fileSize > 0) ? 0 : MSG_MORE;
            ssize_t bytesSent = send(_client, buffer, bytesRead, flag);
            if (bytesSent != static_cast<ssize_t>(bytesRead)) {
                std::cerr << "Failed to send file chunk" << std::endl;
                break;
            }
        } else {
            std::cerr << "Failed to read from file" << std::endl;
            break;
        }
    }

    return 0;
}


int 	Request::handlePost() {
   // if (isCGIRequest(fd))
   //     executeCGIScript(_request[fd]->getObject(), fd);
    if (!isFullRequest()) {
        return 1; // Return 1 if the request is not a full request
    }
    const char* filepath = _createFilePath();
    const char* response = "HTTP/1.1 500 Internal Server Error\r\nContent-Type: text/plain\r\nContent-Length: 18\r\n\r\nError saving file";
    std:: cout << "handleUpload(): Filename: " << filepath << std::endl;
    // Write file data to disk
    int	fd;
	if ( (fd = open(filepath, O_RDWR|O_CREAT, S_IRWXU|S_IRWXO|S_IRWXG)) == -1 )
		std::cout << "Error: could not open file \"" << filepath << "\" with exeution rights " << std::endl;

    std::ofstream file(filepath, std::ios::out | std::ios::trunc | std::ios::binary);
    if ( !file.is_open() ) {
		std::cout << "Error: open file \"" << filepath << "\" failed" << std::endl;
		return -1;
	}
    if (!file.write(reinterpret_cast<const char*>(_body.c_str()), _body.size() - _boundary.size() - 7)) {
		file.close();
		return -1;
	}
    close(fd);
    file.close();

    // Send HTTP response indicating success
    response = "HTTP/1.1 201 Created\r\nContent-Type: text/plain\r\nContent-Length: 18\r\n\r\nUpload successful";
    send(_client, response, strlen(response), 0);
    return 0;
}
