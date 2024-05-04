#include "common.hpp"

//check if request method is allowed in this location
int Request::_validateMethod() {
    std::string methods[3] ={"GET", "POST", "DELETE"};
    bool (LocationConfig::*requestFun[3])() const = {&LocationConfig::getAllowGET, &LocationConfig::getAllowPOST, &LocationConfig::getAllowDELETE};

    for (int i = 0; i < 3; i++) {
        if (_method == methods[i]) {
            return (_location.*requestFun[i])();
        }
    }
    return false;
}

int Request::_replaceRoot(std::string oldRoot) {
    std::cout << "Old object: " << _object << std::endl;
    _root = _location.getRoot();
    size_t start_pos = _object.find(oldRoot);
    std::cout << "root: " << _root << std::endl;
    if(start_pos == std::string::npos)
        return false;
    _object.replace(start_pos, oldRoot.length(), _root);
    std::cout << "New filepath: " << _object << std::endl;
    return true;
}

std::vector<std::string> Request::tokenizePath(const std::string& path) {
    std::vector<std::string> tokens;
    std::istringstream iss(path);
    std::string token;
    while (std::getline(iss, token, '/')) {
        if (!token.empty()) {
            tokens.push_back(token);
        }
    }
    return tokens;
}

void Request::_handleLocation(const std::string& location) {
    _replaceRoot(location); 

    if (_validateMethod() == false) {
        _sendStatusPage(405, "405 Method Not Allowed in this location");
        throw MethodNotAllowedException();
    }

    //check if redirection, change target and save status code
    std::pair<int, std::string> redirect = _location.getRedirect();
    if (redirect.first != 0) {
        _redirStatus = redirect.first;
        _object = redirect.second;
    }
    //HANDLED IN HANDLE GET
    //autoindex on or off - call listfiles function
    //default file (??) if directory
}

void Request::_findLocation(const std::vector<std::string>& tokens, const std::map<std::string, LocationConfig>& locations, size_t index) {
    if (index >= tokens.size()) {
        std::cout << "No matching location found for request." << std::endl;
        return;
    }

    std::string target;
    for (size_t i = 0; i <= index; ++i) {
        target += "/" + tokens[i];
    }

    std::map<std::string, LocationConfig>::const_iterator it = locations.find(target);
    if (it != locations.end()) {
        _location = it->second;
        _handleLocation(it->first);

        // Check if there are nested locations
        std::map<std::string, LocationConfig> nested = it->second.getLocations();
        if (/*nested != NULL && */index + 1 < tokens.size()) {
            _findLocation(tokens, nested, index + 1);
        }
    } else {
        it = locations.find("/");
        if (it != locations.end()) {
            _location = it->second;
            _handleLocation(it->first);
        }
        else
            throw std::runtime_error("No matching location for for request");
    }
}


