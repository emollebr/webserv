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

/* int Request::_replaceRoot(std::string oldRoot) {
    std::cout << "Old object: " << _object << std::endl;
    _root = _location.getRoot();
    if (_root.empty())
        _root = "database";
    size_t start_pos = _object.find(oldRoot);
    std::cout << "root: " << _root << std::endl;
    if(start_pos == std::string::npos)
        return false;
    _object.replace(start_pos, oldRoot.length(), _root);
    std::cout << "New filepath: " << _object << std::endl;
    return true;
} */

std::vector<std::string> Request::tokenizePath(const std::string& path) {
    std::vector<std::string> tokens;
    std::istringstream iss(path);
    std::string token;
    while (std::getline(iss, token, '/')) {
        if (!token.empty()) {
            tokens.push_back("/" + token);
        }
    }
    return tokens;
}

void Request::_handleLocation(const std::string& location) {
    _object = _location.getRoot() + _object;
    std::cout << "Handle location: added root an URI is now: " << _object << std::endl;

    if (_validateMethod() == false) {
        _sendStatusPage(405, "405 Method Not Allowed in this location");
        std::cout << "Method: " << _method << " not allowed in location: " << location << std::endl; 
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

void    Request::_getDefaultLocation(std::map<std::string, LocationConfig> locations) {
    std::cout << "Looking for default loc " << std::endl;
    std::map<std::string, LocationConfig>::const_iterator it = locations.begin();
    for (it = locations.begin(); it != locations.end(); ++it) {
        if (it->first == "/") {
            _location = it->second;
            _handleLocation(it->first);
            return ;
        }

    }
    std::cout << "No matching location found for " << _object << " and no server default configured" << std::endl; 
    throw NoMatchingLocationException();
}

int Request::_findLocation(const std::vector<std::string>& tokens, const std::map<std::string, LocationConfig>& locations, size_t index) {
    std::cout << "amount of tokens: " << tokens.size() << " for object: " << _object << std::endl;
    if (index >= tokens.size()) {
        return 0;
    }

    std::string target;
    for (size_t i = 0; i <= index; ++i) {
        target += tokens[i];
    }

    bool foundMatch = false;
    std::cout << "Looking for target: " << target << std::endl;
    std::map<std::string, LocationConfig>::const_iterator it;
    for (it = locations.begin(); it != locations.end(); ++it) {
        if (it->first == target) {
            std::cout << "Found mathcing location: " << it->first << std::endl;
            _location = it->second;
            foundMatch = true;
            // Check if there are nested locations
            std::map<std::string, LocationConfig> nested = it->second.getLocations();
            if (/*nested != NULL && */index + 1 < tokens.size()) {
                _findLocation(tokens, nested, index + 1);
            }
            _handleLocation(it->first);
            return 1;
        }
    }
   return 0;
}


