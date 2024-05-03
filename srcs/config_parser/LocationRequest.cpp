#include "common.hpp"

//check if request method is allowed in location
int Request::_validateMethod() {
    for (std::vector<std::string>::const_iterator it = _location.getMethods().begin(); it != _location.getMethods().end(); ++it) {
        std::cout << "checking methods: " << *it << std::endl;
        if (*it == _method )
           return true;
    }
    return false;
}

//check if requested target is a server configured location
int Request::_checkLocations(std::map<std::string, LocationConfig> locations) {

    std::istringstream iss(_object);
    std::string token;
    
    iss += 1;
    std::getline(iss, token, '/');
    token = '/' + token;

    std::cout << "Checking for location: " << token << std::endl;
    for (std::map<std::string, LocationConfig>::const_iterator it = locations.begin(); it != locations.end(); ++it) {
        std::cout << "location: " << it->first << std::endl;
        if (it->first.find(token) != std::string::npos) {
            _location = it->second;
            _replaceRoot(it->first); 

            if (_validateMethod() == false) {
                _sendStatusPage(405, "405 Method Not Allowed in this location");
                throw MethodNotAllowedException();
            }

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