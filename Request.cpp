#include "Request.hpp"
#include "Server.hpp"

static std::string  parseBoundary(std::string contentType) {

    std::string boundary;
    size_t boundaryPos = contentType.find("boundary=");
    if (boundaryPos != std::string::npos) {
        size_t endPos = contentType.find_last_not_of(" \t\r\n") + 1;
        size_t startPos = boundaryPos + 9;

        boundary = "--" + (contentType.substr(startPos, endPos - startPos)) + "--"; // 9 is the length of "boundary="
        std::cout << "Boundary: \n" << boundary << std::endl;
        return boundary;
    }
    else {
        std::runtime_error("No boundary found in request header\n");
        return "";
    }
}

void    Request::parseBody( std::istringstream& iss) {
    std::string line;
    while (std::getline(iss, line)) {
        std:: cout << "Line: " << line << std::endl;
        size_t pos = line.find(_boundary);
        if (pos == std::string::npos)
            appendToBody(line.c_str());
        else
            _isFullBody = true;
    }
}

Request::Request(char *buffer, int client) : _clientfd(client) {
    
    std::istringstream iss(buffer);
    std::string line;
    std::getline(iss, line);

    //parse type and object
    size_t firstSpacePos = line.find(' ');
    size_t secondSpacePos = line.find(' ', firstSpacePos + 1);

    _type = line.substr(0, firstSpacePos);
    _object = line.substr(firstSpacePos + 1, secondSpacePos - firstSpacePos - 1);

    // Parse headers
    while (std::getline(iss, line) && line != "\r") {
        size_t pos = line.find(": ");
        if (pos != std::string::npos) {
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 2);
            _headers[key] = value;
        }
    }

    _body = NULL;
  
    if (_type.find("POST") != std::string::npos) {
        _boundary = parseBoundary(_headers["Content-Type"]);
        //get content headers
        while (std::getline(iss, line) && line != "\r") {
            size_t pos = line.find(": ");
            if (pos != std::string::npos) {
                std::string key = line.substr(0, pos);
                std::string value = line.substr(pos + 2);
                _headers[key] = value;
            }
        }
        //get body
        parseBody(iss);
    }

    std::cout << "Created new request" << std::endl;
    return ;
 }

void Request::handleUpload() {
    const char* filename = createFileName();
    //std:: cout << "handleUpload(): Filename: " << filename << std::endl;
    // Write file data to disk
    std::ofstream file(filename, std::ios::binary);
    file << _body;
    file.close();

    // Send HTTP response indicating success
    const char* response = "HTTP/1.1 200 OK\r\nContent-Length: 18\r\n\r\nUpload successful";
    send(_clientfd, response, strlen(response), 0);
}

 void Request::appendToBody(const char* buffer) {
    // Calculate the total size including existing _body and new buffer
    size_t totalSize = strlen(buffer);
    if (_body != NULL)
        totalSize += strlen(_body);

    // Allocate memory for the updated _body
    char* newBody = new char[totalSize + 1];

     if (_body != NULL) {
        strcpy(newBody, _body);
        strcat(newBody, buffer);
    }
    else
        strcpy(newBody, buffer);

    // Delete old _body and update _body pointer
    delete[] _body;
    _body = newBody;
}

/* static bool fileExists(const char* filename) {
    std::ifstream file(filename);
    return file.good();
}
 */
/* // Function to generate a new filename if the original filename already exists
std::string generateNewFilename(const std::string& originalFilename) {
    std::string newFilename = originalFilename;
    std::string extension;
    size_t dotPos = originalFilename.find_last_of('.');
    if (dotPos != std::string::npos) {
        extension = originalFilename.substr(dotPos);
        newFilename = originalFilename.substr(0, dotPos);
    }

    int counter = 1;

    // Keep incrementing counter and appending it to the filename until a unique filename is found
    while (fileExists(newFilename + "(" + std::to_string(counter) + ")" + extension)) {
        counter++;
    }

    // Construct the new filename with the counter and extension
    newFilename = newFilename + "(" + std::to_string(counter) + ")" + extension;
    return newFilename;
} */


const char* Request::createFileName() {

    std::string content = _headers["Content-Disposition"];
    size_t filename_start = content.find("filename=");
    
    size_t startPos = filename_start + 10;
    size_t endPos = content.find_last_not_of(" \"\t\r\n") + 1;
    std::string filename = content.substr(startPos, endPos - startPos);
    std::cout << "WE READING WHAT??::\n" << filename << std::endl;
    

/*    if (fileExists(ret)) {
      char* newFilename = generateNewFilename(filename);
      delete[] filename;
      return newFilename;
  } */

  char* result = strdup(filename.data()); // Allocate memory and copy the data
return result;
}