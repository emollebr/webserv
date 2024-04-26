#pragma once
# define REQUEST_HPP

#include "common.hpp"


class Request {
private:
    std::string                               _method;
    std::string                               _object;
    std::string                               _protocol;
    std::map<std::string, std::string>        _headers;
    std::string                               _boundary;
    std::string                               _body; 
    bool                                      _fullRequest;
    size_t                                    _bytesReceived;
    size_t                                    _contentLength;
    std::string                               _filePath; //for pending response
    bool                                      _pendingResponse;
    off_t                                     _fileSize;
    ssize_t                                   _bytesSent;
    std::map<unsigned int, std::string>       _errorPages;


    int 	_handlePost( void );
    int 	_handleGet( void );
    int 	_handleDelete( void );

    bool    _isFullRequest() const {
        return _fullRequest;
    };

    std::string     _parseBoundary(std::string contentType);
    const char*     _createFileName( void );
    bool            _fileExists(std::string filename);
    int		        _sendStatusPage(int errorCode, std::string msg);
    std::string     _generateNewFilename(const std::string& originalFilename);
    void            _validateContentHeaders(size_t maxBodySize);

public:

    Request(char *buffer, int client, int bytesRead, size_t maxBodySize, std::map<unsigned int, std::string>	error_pages);
    ~Request() {
    };

    int                                   client; //socket fd

    int             detectRequestType( void );
    int             createResponse( void );
    int             sendResponse(const char* response, size_t size, int flag);
    void            pendingPostRequest(char* buffer, int bytesRead);
    bool            isCGIRequest();
    void            executeCGIScript(const std::string& scriptPath, char** env);


    bool            hasPendingResponse( void ) {
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

    class MaxBodySizeExceededException : public std::exception {
        public:
            virtual const char* what() const throw();
    };

    class MissingRequestHeaderException : public std::exception {
        public:
            virtual const char* what() const throw();
    };

    class EmptyRequestedFileException : public std::exception {
        public:
            virtual const char* what() const throw();
    };

    class FileReadException : public std::exception {
        public:
            virtual const char* what() const throw();
    };

};

std::ostream &operator<<(std::ostream &str, Request &rp);