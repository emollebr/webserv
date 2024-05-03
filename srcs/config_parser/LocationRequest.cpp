#include "common.hpp"

//check if request method is allowed in location
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

//check for root, then replace root directory - full path will be finished in _finishPath()
//if no root specified for location, default server root stays root
int Request::_replaceRoot(std::string oldRoot) {
    std::cout << "Old object: " << _object << std::endl;
    std::string newRoot = _location.getRoot();
    if (!newRoot.empty())
        _root = newRoot;
    size_t start_pos = _object.find(oldRoot);
    std::cout << "root: " << _root << std::endl;
    if(start_pos == std::string::npos)
        return false;
    _object.replace(start_pos, oldRoot.length(), "");
    std::cout << "New object: " << _object << std::endl;
    return true;
}

//check if requested target is a server configured location
int Request::_checkLocations(std::map<std::string, LocationConfig> locations) {

    std::istringstream iss(_object);
    std::string token;
    
    iss.ignore(1);
    std::getline(iss, token, '/');
    token = '/' + token;

    std::cout << "Checking for location: " << token << std::endl;
    for (std::map<std::string, LocationConfig>::const_iterator it = locations.begin(); it != locations.end(); ++it) {
        std::cout << "location: " << it->first << std::endl;
        if (it->first == token) {
            std::cout << "FOUND location: " << it->first << std::endl;
            _location = it->second;
            _replaceRoot(it->first); 
/* 
            if (_validateMethod() == false) {
                _sendStatusPage(405, "405 Method Not Allowed in this location");
                throw MethodNotAllowedException();
            } */

            //check if redirection, change target and save status code
            std::pair<int, std::string> redirect = _location.getRedirect();
            if (redirect.first != 0) {
                _redirStatus = redirect.first;
                _filePath = redirect.second;
            }

            //autoindex on or off - call listfiles function

            //default file (??) if directory
            break ;
        }
    }

    return SUCCESS;
}