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
    //replace URL location with root directive
    if (!_location.getRoot().empty()){
        _root = _location.getRoot();
        _path = _root + _path;
       // size_t pos = _path.find(location); 
        //_path.replace(0, pos - 1, _root);
        std::cout << "Handle location: prepended root to location, URL is now: " << _path << std::endl;
    }

    if (_validateMethod() == false) {
        _sendStatusPage(405, "405 Method Not Allowed in this location");
        std::cout << "Method: " << _method << " not allowed in location: " << location << std::endl; 
        throw MethodNotAllowedException();
    }
}

void    Request::_getDefaultLocation(std::map<std::string, LocationConfig> locations) {
    std::map<std::string, LocationConfig>::const_iterator it = locations.begin();
    for (it = locations.begin(); it != locations.end(); ++it) {
        if (it->first == "/") {
            _location = it->second;
            _handleLocation(it->first);
            return ;
        }

    }
    std::cout << "No matching location found for " << _path << " and no server default configured" << std::endl; 
    throw NoMatchingLocationException();
}

void    Request::_handleRedirect() {
    //handle redirect if present
    std::pair<int, std::string> redirect = _location.getRedirect();
    if (redirect.first != 0) {
        std::stringstream response;
        response << "HTTP/1.1 " + intToStr(redirect.first) + "\r\nLocation: " << redirect.second;
        sendResponse(response.str().c_str(), response.str().size(), 0);
        throw std::runtime_error("Redirected client");
    }
}

int Request::_findLocation(const std::vector<std::string>& tokens, const std::map<std::string, LocationConfig>& locations, size_t index) {
    if (index >= tokens.size()) {
        return 0;
    }

    std::string target;
    for (size_t i = 0; i <= index; ++i) {
        target = tokens[i];
    }

    std::map<std::string, LocationConfig>::const_iterator it;
    for (it = locations.begin(); it != locations.end(); ++it) {
        if (it->first == target) {
            _location = it->second;
            // Check if there are nested locations
            std::map<std::string, LocationConfig> nested = it->second.getLocations();
            if (_findLocation(tokens, nested, index + 1) == 0)
                _handleLocation(it->first);
            return 1;
        }
    }
   return 0;
}


