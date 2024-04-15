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
        size_t pos = line.find(_boundary);
        if (pos == std::string::npos) {
            _isFullBody = false;
            _body.append(line);
        }
        else
            _isFullBody = true;
    }
}

Request::Request(char *buffer, int client) : _client(client) {
    
    std::istringstream iss(buffer);
    std::string line;

    //parse
    iss >> _method >> _object >> _protocol;
    std::getline(iss, line);
    // Parse headers
    while (std::getline(iss, line) && line != "\r") {
        size_t pos = line.find(": ");
        if (pos != std::string::npos) {
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 2);
            _headers[key] = value;
        }
    }
  
    if (_method == "POST") {
        _boundary = parseBoundary(_headers["Content-Type"]);
        //get content headers
        while (std::getline(iss, line) && line == "\r")
            continue;
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

const char* Request::handleUpload() {
    const char* filepath = createFilePath();
    const char* response = "HTTP/1.1 500 Internal Server Error\r\nContent-Type: text/plain\r\nContent-Length: 18\r\n\r\nError saving file";
    std:: cout << "handleUpload(): Filename: " << filepath << std::endl;
    // Write file data to disk
    int	fd;
	if ( (fd = open(filepath, O_RDWR|O_CREAT, S_IRWXU|S_IRWXO|S_IRWXG)) == -1 )
		std::cout << "Error: could not open file \"" << filepath << "\" with exeution rights " << std::endl;

    std::ofstream file(filepath, std::ios::out | std::ios::trunc | std::ios::binary);
    if ( !file.is_open() ) {
		std::cout << "Error: open file \"" << filepath << "\" failed" << std::endl;
		return NULL;
	}
    if (!file.write(reinterpret_cast<const char*>(_body.c_str()), _body.size())) {
		file.close();
		return NULL;
	}
    close(fd);
    file.close();

    // Send HTTP response indicating success
   response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 18\r\n\r\nUpload successful";
    return response;
}

static bool fileExists(std::string filename) {
    std::ifstream file(filename.c_str());
    return file.good();
}

// Function to generate a new filename if the original filename already exists
std::string generateNewFilename(const std::string& originalFilename) {
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
    while (fileExists(newFilename.c_str() + oss.str() + extension)) {
        counter++;
        oss << "(" << counter << ")";
    }

    newFilename = newFilename + oss.str() + extension;
    return newFilename;
}


const char* Request::createFilePath() {

    std::string content = _headers["Content-Disposition"];
    size_t filename_start = content.find("filename=");
    
    size_t startPos = filename_start + 10;
    size_t endPos = content.find_last_not_of(" \"\t\r\n") + 1;
    std::string filename = "database/uploads/" + content.substr(startPos, endPos - startPos);    

   if (fileExists(filename.c_str())) {
      std::string tmp = generateNewFilename(filename);
      filename = tmp;
    }

    char* result = strdup(filename.data()); // Allocate memory and copy the data
    return result;
}

std::ostream &operator<<(std::ostream &str, Request &rp)
{
    str << "Client FD: " << rp.getClient() << std::endl;
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