#pragma once
# define REQUEST_HPP

#include <cstdio>
#include <sys/socket.h>
#include <unistd.h> // For dup2, execl, close
#include <iostream>
#include <fstream>      // For std::ifstream
#include <sstream>
#include <cstring> // for strlen
#include <unistd.h> // for send
#include <climits>
#include <map>
#include <poll.h>

class Request {
private:
    std::string                           _method;
    std::string                           _object;
    std::string                         _protocol;
    std::map<std::string, std::string>   _headers;
    std::string                         _boundary;
    int                                   _client;
    bool                              _isFullRequest;
    std::string                             _body;
    long long int                   _bytesCounter;
    long long int                  _contentLength;

        class formObject
    {
        public:
                formObject() {bodyLength = 0;};
                ~formObject() {};
                std::string name;
                std::string fileName;
                std::string contentType;
                std::string body;
                ssize_t bodyLength;
    };
    
    formObject _form;

    int 	handlePost( void );
    int 	handleGet( void );
    int     handleUnknown( void );
    int 	handleDelete( void );

    bool    isFullRequest() const {
        return _isFullRequest;
    };

    ssize_t&        _getFormBodyLength(ssize_t &bodyLength);
    const char*                         _handleUpload( void );
    const char*                             _createFilePath();
    std::string  _parseBoundary(std::string contentType);

public:

    Request(char *buffer, int client, int bytesRead);
    ~Request() {
        std::cout << "Request deleted" << std::endl;
    };

    int                                 detectRequestType();
    void     _pendingPostRequest(char* buffer, int bytesRead);

    const std::map<std::string, std::string>& getHeaders() const {
        return _headers;
    };

    const std::string& getBody() const {
        return _body;
    };

    const std::string& getMethod() const {
        return _method;
    };

    const std::string& getObject() const {
        return _object;
    };

    const std::string& getProtocol() const {
        return _protocol;
    };

    const std::string& getBoundary() const {
        return _boundary;
    };

    const int& getClient() const {
        return _client;
    };

};

std::ostream &operator<<(std::ostream &str, Request &rp);