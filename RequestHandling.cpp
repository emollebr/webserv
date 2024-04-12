#include "Server.hpp"
#include <dirent.h>
#include <sstream>

void Server::handleRequest(int i) {
    int fd = _sockets[i].fd;
    char buffer[98761];
    ssize_t bytesRead = read(fd, buffer, sizeof(buffer));
    std::cout << "BUFFER:" << buffer << std::endl;
    if (bytesRead <= 0) {
        disconnectClient(bytesRead, i);
    } else {
        if (_request.count(fd) == 0) {
            std::cout << "No request for client " << fd << std::endl;
            _request.insert(std::make_pair(fd, new Request(buffer, fd)));
            std::cout << *_request[fd] << std::endl;
        }
        else if (_request[fd]->getMethod() == "POST") {
            std::cout << "Pending POST request for client " << i << std::endl;
            _request[fd]->appendToBody(buffer);
        }
        detectRequestType(i);
    }
}

void    Server::detectRequestType(int client) {
    int fd = _sockets[client].fd;
    std::string methods[3] ={"GET", "POST", "DELETE"};
    int (Server::*requestFun[3])(int) = {&Server::handleGet, &Server::handlePost, &Server::handleDelete};
    std::string requestMethod = _request[fd]->getMethod();

    for (int i = 0; i < 3; i++) {
        if (requestMethod == methods[i]) {
            if ((this->*requestFun[i])(fd) == 1)
                return ; //unfinished POST request fx.
            break;
        }
    }
    delete _request[fd];
    _request.erase(fd);
    close(fd);
    _sockets.erase(_sockets.begin() + client);
}

int 	Server::handleDelete(int fd) {
    std::string response;
    const char* path = _request[fd]->getObject().c_str();
    std::cout << path << std::endl;
    if (std::remove(path) != 0) {
        std::cerr << "Error deleting file: " << strerror(errno) << std::endl;
        response = "HTTP/1.1 404 Not Found\nContent-Type: text/plain\n\n404 Not Found\n";
    }
    else
        response = "HTTP/1.1 200 OK\nContent-Type: text/plain\n\nFile successfully deleted\n";
    send(fd, response.c_str(), response.size(), 0);
    return 0;

}

int Server::handleUnknown(int fd) {
    std::string response = "HTTP/1.1 400 Bad Request\nContent-Type: text/plain\n\n400 Bad Request\n";
    send(fd, response.c_str(), response.size(), 0);
    return 0;
}

int 	Server::handleGet(int fd) {
    std::string object = _request[fd]->getObject();
    if (object == "/")
        object += "home.html";
    if (object == "/list_files") {
        handleListFiles(fd);
        return 0;
    }
    //complete the path
    object = "database" + object;
    std::ifstream file(object.c_str());
    std::cout << "In GET handling: serving: " << object << std::endl;
    if (!file) {
        // Error opening index.html file
        std::cerr << "Failed to open " << object << " file" << std::endl;
        std::string response = "HTTP/1.1 404 Not Found\nContent-Type: text/plain\n\n404 Not Found\n";
        send(fd, response.c_str(), response.size(), 0);
        return 0;
    }
    //read contents
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();

    // Send HTTP response with content
    std::string response = "HTTP/1.1 200 OK\nContent-Type: text/html\n\n" + content;
    send(fd, response.c_str(), response.size(), 0);
    return 0;
}

int 	Server::handlePost(int fd) {
    if (isCGIRequest(fd))
        executeCGIScript(_request[fd]->getObject(), fd);
    if (!_request[fd]->isFullRequest()) {
        return 1; // Return 1 if the request is not a full request
    }
    const char* response = _request[fd]->handleUpload();
    send(fd, response, strlen(response), 0);
    return 0;
}


std::vector<std::string> listFiles(const std::string& directoryPath) {
    std::vector<std::string> fileList;
    DIR* dir;
    struct dirent* entry;

    // Open the directory
    if ((dir = opendir(directoryPath.c_str())) != NULL) {
        // Iterate through each entry in the directory
        while ((entry = readdir(dir)) != NULL) {
            if (std::strcmp(entry->d_name, ".") != 0 && std::strcmp(entry->d_name, "..") != 0)
                fileList.push_back(entry->d_name); // Add the file name to the list
        }
        closedir(dir); // Close the directory
    } else {
        std::cerr << "Error opening directory: " << directoryPath << std::endl;
    }

    return fileList;
}

void handleListFiles(int clientSocket) {
    // Get the list of files in the database/uploads directory
    std::vector<std::string> files = listFiles("database/uploads");

    // Prepare the response
    std::ostringstream response;
    response << "HTTP/1.1 200 OK\r\n"
             << "Content-Type: text/plain\r\n"
             << "Connection: close\r\n\r\n";

    // Append the list of files to the response
    for (std::vector<std::string>::const_iterator it = files.begin(); it != files.end(); ++it) {
        response << *it << std::endl;
    }
    std::cout << "Get File List: response: " << response.str() << std::endl;

    // Send the response to the client
    send(clientSocket, response.str().c_str(), response.str().size(), 0);
}
