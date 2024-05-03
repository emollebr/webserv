#include "common.hpp"

void signal_handler(int signum) {
    g_signal_received = signum;
}

std::string intToStr(int num) {
    std::ostringstream oss;
    oss << num;
    return oss.str();
}

// Function to get MIME type based on file extension
std::string getMimeType(const std::string& filename) {
    static std::map<std::string, std::string> mimeMap;
    //parse file with known mime types
    if (mimeMap.empty()) {
            std::ifstream file("./database/mime_types.txt");
            if (!file.is_open()) {
                std::cerr << "Error opening MIME types file." << std::endl;
                return "application/octet-stream"; // Default MIME type if file cannot be opened
            }

            std::string line;
            while (std::getline(file, line)) {
                std::istringstream iss(line);
                std::string extension, mimeType;
                if (iss >> extension >> mimeType) {
                    mimeMap[extension] = mimeType;
                }
            }
        }

    // Find extension
    size_t dotPos = filename.rfind('.');
    if (dotPos != std::string::npos) {
        std::string extension = filename.substr(dotPos);
        // Look up MIME type in the map
        std::map<std::string, std::string>::iterator it = mimeMap.find(extension);
        if (it != mimeMap.end()) {
            return it->second; // Return MIME type if found
        }
    }
    return "application/octet-stream"; // Default MIME type if extension not found
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
    std::vector<std::string> files = listFiles("database/uploads");

    std::ostringstream response;
    response << "HTTP/1.1 200 OK\r\n"
             << "Content-Type: text/plain\r\n"
             << "Connection: close\r\n\r\n";

    for (std::vector<std::string>::const_iterator it = files.begin(); it != files.end(); ++it) {
        response << *it << std::endl;
    }

    send(clientSocket, response.str().c_str(), response.str().size(), 0);
}