#pragma once
# define REQUEST_HPP

#include <cstdio>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>
#include <fstream> 
#include <sstream>
#include <cstring>
#include <unistd.h>
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
    std::string                             _body; 
    bool                           _fullRequest;
    long long int                   _bytesReceived;
    long long int                  _contentLength;
    std::string                         _filePath; //for pending response
    bool                          _pendingResponse;
    off_t                              _fileSize;
    ssize_t                             _bytesSent;


    int 	_handlePost( void );
    int 	_handleGet( void );
    int     _handleUnknown( void );
    int 	_handleDelete( void );

    bool    _isFullRequest() const {
        return _fullRequest;
    };

    std::string                   _parseBoundary(std::string contentType);
    const char*                                   _createFileName( void );
    static bool                         _fileExists(std::string filename);
    std::string _generateNewFilename(const std::string& originalFilename);


public:

    Request(char *buffer, int client, int bytesRead);
    ~Request() {
        std::cout << "Request deleted" << std::endl;
    };

    int                                   client; //socket fd

    int                           detectRequestType( void );
    int                                sendResponse( void );
    void    pendingPostRequest(char* buffer, int bytesRead);
    bool        hasPendingResponse( void ) {
        return _pendingResponse;
    };

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

};

std::ostream &operator<<(std::ostream &str, Request &rp);