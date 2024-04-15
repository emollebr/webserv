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
    std::string                             _body;
    int                                   _client;
    bool                              _isFullBody;

public:
    Request(char *buffer, int client);
    ~Request() {
        std::cout << "Request deleted" << std::endl;
    };

    const char*                handleUpload( void );
    void        parseBody( std::istringstream& iss);
    void        appendToBody(const char* buffer);
    const char*                 createFilePath();


    bool    isFullRequest() const {
       return _isFullBody;
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

    const int& getClient() const {
        return _client;
    };

};

std::ostream &operator<<(std::ostream &str, Request &rp);