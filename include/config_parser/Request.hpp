#pragma once
# define REQUEST_HPP

#include "common.hpp"

class Request {
private:
    LocationConfig                            _location;
    std::string                               _root;
    std::string                               _method;
    std::string                               _object;
    std::string                               _protocol;
    std::map<std::string, std::string>        _headers;
    std::string                               _boundary;
    std::string                               _body;
    bool                                      _fullRequest;
    size_t                                    _bytesReceived;
    size_t                                    _contentLength;
    bool                                      _pendingResponse;
    off_t                                     _fileSize;
    ssize_t                                   _bytesSent;
    std::map<unsigned int, std::string>       _errorPages;
    int                                       _redirStatus;
    std::string                               _cgi_path;

    int 	_handlePost( void );
    int 	_handleGet( void );
    int 	_handleDelete( void );
    int    _handleListFiles(std::string directory);

    bool    _isFullRequest() const {
        return _fullRequest;
    };

    void            _parseContentHeaders(char *buffer, std::streampos pos);
    void            _parseRequestBody(char *buffer, int bytesRead);
    std::string     _parseBoundary(std::string contentType);
    void            _validateContentHeaders(size_t maxBodySize);
    int            _findLocation(const std::vector<std::string>& tokens, const std::map<std::string, LocationConfig>& locations, size_t index);
    void            _getDefaultLocation(std::map<std::string, LocationConfig> locations);
    void            _handleLocation(const std::string& location);
    const char*     _createFileName( void );
    int		        _sendStatusPage(int statusCode, std::string msg);
    void            _finishPath( void );
    bool            _fileExists(std::string filename);
    std::string     _generateNewFilename(const std::string& originalFilename);


    //LocationRequest.cpp
    int             _checkLocations( std::map<std::string, LocationConfig> locations);
    int             _replaceRoot(std::string oldRoot);
    int             _validateMethod( void );

public:

    Request(char *buffer, int client, int bytesRead, ServerConfig config);
    ~Request() {
    };

    int                                   client; //socket fd

    int             detectRequestType( void );
    int             createResponse( void );
    int             sendResponse(const char* response, size_t size, int flag);
    void            pendingPostRequest(char* buffer, int bytesRead);
    bool            isCGIRequest();
    void            executeCGIScript(const std::string& scriptPath, int clientSocket, char** env);
    std::vector<std::string> tokenizePath(const std::string& path);

    bool            hasPendingResponse( void ) {
        return _pendingResponse;
    };

    void        setLocation(LocationConfig location) {
        this->_location = location;
    };

    LocationConfig        getLocation() {
        return _location;
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


    class MaxBodySizeExceededException : public std::exception {
    public:
        virtual const char* what() const throw() {
           return "Max body size exceeded";
        }
    };

    class MissingRequestHeaderException : public std::exception {
    public:
        virtual const char* what() const throw() {
            return "Missing request header";
        }
    };

    class MethodNotAllowedException : public std::exception {
    public:
        virtual const char* what() const throw() {
            return "Method not allowed in this location";
        }
    };

    class NoMatchingLocationException : public std::exception {
    public:
        virtual const char* what() const throw() {
            return "No matching location found for request";
        }
    };

    class EmptyRequestedFileException : public std::exception {
    public:
        virtual const char* what() const throw() {
           return "Requested file is empty";
        }
    };

    class FileReadException : public std::exception {
    public:
        virtual const char* what() const throw() {
            return "Failed to read requested file";
        }
    };

};

std::ostream &operator<<(std::ostream &str, Request &rp);