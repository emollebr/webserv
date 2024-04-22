#include "Request.hpp"
#include "Server.hpp"

Request::Request(char *buffer, int client, int bytesRead) : _pendingResponse(0), _bytesSent(0), client(client){
    
    std::istringstream iss(buffer);
    std::string line;
    int bytesProcessed = 0;

    iss >> _method >> _object >> _protocol;
    _filePath = finishPath(_object);
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

        //validate content headers headers/handle errors
        std::map<std::string, std::string>::const_iterator it = _headers.find("Content-Disposition");
        if (it == _headers.end()) {
            std::string msg = "400 Bad Request: Missing 'Content-Disposition' header for POST request\n";
            std::ostringstream response;
            response << "HTTP/1.1 400 Bad Request\nContent-Type: text/plain\nContent-Length: " << msg.size() << "\n" << msg;
            send(client, response.str().c_str(), response.str().size(), 0);
            throw MissingRequestHeaderException();
        }  
        it = _headers.find("Content-Length");
        if (it == _headers.end()) {
            std::string msg = "400 Bad Request: Missing 'Content-Length' header for POST request\n";
            std::ostringstream response;
            response << "HTTP/1.1 400 Bad Request\nContent-Type: text/plain\nContent-Length: " << msg.size() << "\n" << msg;
            send(client, response.str().c_str(), response.str().size(), 0);
            throw MissingRequestHeaderException();
        }
        std::istringstream ss(it->second);
        ss >> _contentLength;
        if (_contentLength > MAX_BODY_SIZE) {
            std::stringstream bs;
            bs << MAX_BODY_SIZE;
            std::string msg = "413 Payload Too Large: Content-Length exceeds limit of " + bs.str() + " bytes\n";
            std::ostringstream response;
            response << "HTTP/1.1 413 Payload Too Large\nContent-Type: text/plain\nContent-Length: " << msg.size() << "\n" << msg;
            send(client, response.str().c_str(), response.str().size(), 0);
            throw MaxBodySizeExceededException();
        }

        //save the request body
        char *bodyStart = std::strstr(buffer, "\r\n\r\n");
        if (bodyStart != NULL) {
            // Skip an additional 3 occurrences of "\r\n"
            for (int i = 0; i < 5; ++i) {
                bodyStart = std::strstr(bodyStart + 2, "\r\n");
                if (bodyStart == NULL) {
                    break; // Stop if we reach the end of the buffer
                }
            }

            if (bodyStart != NULL) {
                bodyStart += 2; // Move past the last "\r\n"
                size_t bodySize = bytesRead - (bodyStart - buffer); // Calculate the size of the binary data
                _body.append(bodyStart, bodySize); // Append the binary data
            }
        }
        
        _bytesReceived = bytesRead - bytesProcessed;
        _fullRequest = (_bytesReceived < _contentLength) ? false : true;
    }
    return ;
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
    std::ostringstream oss;
    oss << "(" << counter << ")";
    // Keep incrementing counter and appending it to the filename until a unique filename is found
    while (_fileExists(newFilename.c_str() + oss.str() + extension)) {
        counter++;
        oss.str("");
        oss << "(" << counter << ")";
    }

    newFilename = newFilename + oss.str() + extension;
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