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
    char*                                   _body;
    int                                   _client;
    bool                              _isFullBody;
    //struct timeval                       _timeout;

public:
    Request(char *buffer, int client);
    ~Request() {
        delete[] _body;
        std::cout << "Request deleted" << std::endl;
    };

    const char*                handleUpload( void );
    void        parseBody( std::istringstream& iss);
    void        appendToBody(const char* buffer);
    const char*                 createFileName();


    bool    isFullRequest() const {
       return _isFullBody;
    };

    const std::map<std::string, std::string>& getHeaders() const {
        return _headers;
    };

    const char* getBody() const {
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