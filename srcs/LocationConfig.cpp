/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocationConfig.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jschott <jschott@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/15 12:09:36 by jschott           #+#    #+#             */
/*   Updated: 2024/04/17 17:18:53 by jschott          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "LocationConfig.hpp"

void LocationConfig::init(){
	_max_body_size = 4.2 * 1024;
	_indent_lvl = 0;

	std::string	directives[] = {"root", "index", "methods",
									"return", "client_max_body_size",
									"upload_location", "autoindex"};
	typedef void (LocationConfig::*LocationConfigFunction)(tokeniterator begin, tokeniterator end);
	LocationConfigFunction functions[] = {&LocationConfig::validateRoot, 
											&LocationConfig::validateIndex, 
											&LocationConfig::validateMethods, 
											&LocationConfig::validateRedirect, 
											&LocationConfig::validateBodySize, 
											&LocationConfig::validateUploadLocation, 
											&LocationConfig::validateAutoindex};
	int size = sizeof(directives) / sizeof(directives[0]);
	for (int i = 0; i < size; i++){
		_directives_set[directives[i]] = false;
		_directives_validation_funcs[directives[i]] = functions[i];
	}
}

LocationConfig::LocationConfig(){
	init();
}

LocationConfig::LocationConfig(std::string root){
	_root = root;	
}

LocationConfig::LocationConfig(LocationConfig const & origin){
	*this = origin;		
}

LocationConfig & LocationConfig::operator=(LocationConfig const & origin){
	if (this == &origin)
		return *this;

	_root = origin._root; 
	_index = origin._index;
	_methods_allowed = origin._methods_allowed;
	_redirect = origin._redirect;
	_max_body_size = origin._max_body_size;
	_upload_location = origin._upload_location;
	_autoindex = origin._autoindex;
	_locations = origin._locations;
	_indent_lvl = origin._indent_lvl;
	_directives_set = origin._directives_set; 
	_directives_validation_funcs = origin._directives_validation_funcs;

	return (*this);
}

LocationConfig::~LocationConfig(){
}

std::string	LocationConfig::getIndex() const{
	return (_index);
}

std::set<std::string> LocationConfig::getMethods() const{
	if (_methods_allowed.empty())
		return std::set<std::string>();
	std::set<std::string>	methods;
	methods = _methods_allowed;
	return (methods);
}

bool LocationConfig::hasMethod(std::string method){
	if (_methods_allowed.find(method) != _methods_allowed.end())
		return true;
	return false;
}

std::string		LocationConfig::getRoot() const {
	return _root;
}


std::pair<int, std::string>	LocationConfig::getRedirect() const{
	return _redirect;
}

size_t LocationConfig::getBodySize() const{
	return _max_body_size;	
}

std::string LocationConfig::getUploadLocation() const{
	return _upload_location;
}

std::map<std::string, LocationConfig> const LocationConfig::getLocations() const{
	if (_locations.empty())
		return std::map<std::string, LocationConfig>();
	std::map<std::string, LocationConfig>	locations;
	locations = _locations;
	return (locations);
}

bool LocationConfig::getAllowGET() const{
	if (_methods_allowed.find("GET") != _methods_allowed.end())
		return true;
	return false;
}

bool LocationConfig::getAllowPOST() const{
	if (_methods_allowed.find("POST") != _methods_allowed.end())
		return true;
	return false;
}

bool LocationConfig::getAllowDELETE() const{
	if (_methods_allowed.find("DELETE") != _methods_allowed.end())
		return true;
	return false;
}

bool LocationConfig::getAutoindex() const {
	return _autoindex;
}

size_t LocationConfig::getIndent() const {
	return _indent_lvl;
}

void	LocationConfig::set_indent(size_t new_level){
	_indent_lvl = new_level;
}

void LocationConfig::parseLocationDirective(tokeniterator begin, tokeniterator end) {
	while (*end == "")
		--end;
	if (_directives_set.find(*begin) == _directives_set.end()) {
		throw std::invalid_argument("invalid directive: " + *begin);
	}
	std::map<std::string, void (LocationConfig::*)(tokeniterator, tokeniterator)>::iterator function = _directives_validation_funcs.find(*begin);
	if (function != _directives_validation_funcs.end()) {
		try {
			(this->*(function->second))(begin + 1, end);
			(*_directives_set.find(*begin)).second = true;
		}
		catch(const std::exception& e) {
			throw std::invalid_argument("directive " + *begin + ": " + e.what());
		}
	}
}

void LocationConfig::validateRoot(tokeniterator begin, tokeniterator end){
	
	if (begin == end){
		if (directoryExists(*begin)) {
			if ((*_directives_set.find("root")).second)
				std::cerr << COLOR_WARNING << "Warning: multiple root directives. using last." << COLOR_STANDARD << std::endl;
			_root = (*begin);
			_directives_set["root"] = true;
		}
		else
			throw std::invalid_argument("invalid parameter: " + *begin);
	}
	else
		throw std::invalid_argument("invalid parameter: " + *begin);
}

void LocationConfig::validateIndex(tokeniterator begin, tokeniterator end){

	if (begin != end)
		throw std::invalid_argument("invalid number of parameters.");
	if ((*_directives_set.find("index")).second)
		std::cerr << COLOR_WARNING << "Warning: multiple index directives. using last." << COLOR_STANDARD << std::endl;

	_index = *begin;
}

void LocationConfig::validateMethods(tokeniterator begin, tokeniterator end){

	while (begin <= end){
		if (!hasMethod(*begin) && 
			(*begin == "GET" || *begin == "POST" || *begin == "DELETE"))
			_methods_allowed.insert(*begin);
		else if (!hasMethod(*begin))
			throw std::invalid_argument("invalid parameter: " + *begin);
		begin++;
	}
}

void LocationConfig::validateRedirect(tokeniterator begin, tokeniterator end){

	if (begin + 1 == end){
		int statusCode = std::atoi((*begin).c_str());
		if (statusCode < 100 || statusCode > 599)
			throw std::invalid_argument("invalid parameter: " + *begin);
		if (statusCode < 301 || statusCode > 308)
			std::cerr << COLOR_WARNING << "Warning: unusual status code for redirect: " << statusCode << COLOR_STANDARD << std::endl;
		if (++begin != end)
			throw std::invalid_argument("invalid parameter: " + *begin);
		if ((*_directives_set.find("return")).second)
			std::cerr << COLOR_WARNING << "Warning: Multiple return directives. Will use last." << COLOR_STANDARD << std::endl;
		_redirect.first = statusCode;
		_redirect.second = *begin;
		_directives_set["return"] = true;
	}
	else
		throw std::invalid_argument("Invalid return arguments.");
}

void LocationConfig::validateBodySize(tokeniterator begin, tokeniterator end){

	if (begin == end){
		char* endptrx = NULL;
		long double body = strtod((*begin).c_str(), &endptrx);
		std::string endptr = endptrx;

		if (body < 0)
			throw std::invalid_argument("invalid parameter: " + *begin);
		if (endptr == "" || endptr == "B")
			_max_body_size = body;
		else if (endptr == "M" || endptr == "MB")
			_max_body_size = body * 1024;
		else if (endptr == "M" || endptr == "MB")
			_max_body_size = body * 1024 * 1024;
		else if (endptr == "G" || endptr == "GB")
			_max_body_size = body * 1024 * 1024 * 1024;
		else
			throw std::invalid_argument("invalid parameter: " + *begin);
	}
	else
		throw std::invalid_argument("invalid number of parameters");
	if ((*_directives_set.find("client_max_body_size")).second){
		std::cerr << COLOR_WARNING << "Warning: Multiple client_max_body_size directives. Will use last." << COLOR_STANDARD << std::endl;
		_directives_set["client_max_body_size"] = true;
	}
}

void LocationConfig::validateUploadLocation(tokeniterator begin, tokeniterator end){
	if (begin == end){
		if (directoryExists(*begin)) {
			_upload_location = (*begin);
		}
		else
			throw std::invalid_argument("invalid parameter: " + *begin);
	}
	else
		throw std::invalid_argument("invalid number of parameters.");
}

void LocationConfig::validateAutoindex(tokeniterator begin, tokeniterator end){	
	if (begin == end){
		if (*begin == "1" || *begin == "on")
			_autoindex = true;
		else if (*begin == "0" || *begin == "off")
			_autoindex = false;
		else
			throw std::invalid_argument("invalid parameter: " + *begin);
	}
	else
		throw std::invalid_argument("invalid number of parameters.");
}

void LocationConfig::fillNestedLocation(){
	if (_locations.empty())
		return ;
	for (std::map<std::string, LocationConfig>::iterator it = _locations.begin();
				it != _locations.end(); it++){
		std::string	directives[] = {"root", "index", "methods",
									"return", "client_max_body_size",
									"upload_location", "autoindex"};
		int size = sizeof(directives) / sizeof(directives[0]);
		for (int i = 0; i < size; i++){
			if (!(*(*it).second._directives_set.find(directives[i])).second){
				switch (i) {
				case 0:
					(*it).second._root = _root;
					break;
				case 1:
					if ((*it).second._root != _root)
						(*it).second._index = _root + _index;
					else
						(*it).second._index = _index;
					break;
				case 2:
					(*it).second._methods_allowed = _methods_allowed;
					break;
				case 3:
					(*it).second._redirect = _redirect;
					break;
				case 4:
					(*it).second._max_body_size = _max_body_size;
					break;
				case 5:
					(*it).second._upload_location = _upload_location;
					break;
				case 6:
					(*it).second._autoindex = _autoindex;
					break;
				default:
					continue ;
				}
			if (this->directiveSet(directives[i]))
				(*(*it).second._directives_set.find(directives[i])).second = true;
			}
		}
		(*it).second.fillNestedLocation();
	}
}

LocationConfig::LocationConfig(tokeniterator begin, tokeniterator end){
	tokeniterator statementend;
	init();
	try	{
		while (begin < end){
			while (*begin == "")
				begin++;
			if (*begin == "location"){
				begin++;
				while (*begin == "")
					begin++;
				std::string location_name = *begin++;
				while (*begin == "")
					begin++;
				//CHECK FOR OPENING BRAKET AND FIND CLOSING TO PARSE BLOCK
				if (begin != end && *begin == "{" &&
						((statementend = getClosingBraket(begin, end)) <= end)){
					try	{
						if (_locations.find(location_name) != _locations.end())
							std::cerr << COLOR_WARNING << "Warning: duplicated location block " << location_name << ". Using last." << COLOR_STANDARD << std::endl;
						_locations[location_name] = LocationConfig(begin + 1, statementend - 1);
					}
					catch(const std::exception& e) {
						throw std::invalid_argument("location " + location_name + ": " + e.what());
					}
					begin = statementend + 1;
				}
				else
					throw std::invalid_argument("location block " + *begin + " missing closing '}'");
			}
			else if ((statementend = std::find(begin, end + 1, ";")) <= end) {
				parseLocationDirective(begin, statementend - 1);
				begin = statementend + 1;
			}
			else
				throw std::invalid_argument("directive: " + *begin + " missing closing ';'.");
		}
	}
	catch(const std::exception& e)	{
			throw std::invalid_argument(e.what());
		}
}

bool	LocationConfig::directiveSet(std::string directive) const{
	return ((*_directives_set.find(directive)).second);
}

std::ostream& operator<<(std::ostream& os, const LocationConfig& locationconf) {

	std::string indent = "\t\t";
	for (int lvl = locationconf.getIndent(); lvl >= 0; lvl--)
		indent += "\t";

	std::vector<std::string> printbuff;
	if (locationconf.directiveSet("root"))
		os <<  indent << "root\t\t\t" << locationconf.getRoot() << ";" << std::endl;

	if (locationconf.directiveSet("index"))
		os <<  indent << "index\t\t\t" << locationconf.getIndex() << ";" << std::endl;
	
	if (locationconf.directiveSet("methods")) {
		os <<  indent << "methods\t\t\t";
		std::set<std::string> printset = locationconf.getMethods();
		for (std::set<std::string>::iterator it = printset.begin(); it != printset.end(); it++){
			os << *it;
			std::set<std::string>::iterator next_it = it;
			++next_it;
			if (next_it != printset.end())
				os << " ";
		}
		os << ";" << std::endl;
	}	

	if (locationconf.directiveSet("redirect"))
		os <<  indent << "return\t\t\t" << locationconf.getRedirect().first << " " << locationconf.getRedirect().second << ";" << std::endl;
	
	os <<  indent << "client_max_body_size\t" << locationconf.getBodySize() << ";" << std::endl;
	
	if (locationconf.directiveSet("upload_location"))
		os <<  indent << "upload location\t\t" << locationconf.getUploadLocation() << ";" << std::endl;
	
	os <<  indent << "autoindex\t\t" << locationconf.getAutoindex() << ";" << std::endl;
	
	std::map<std::string, LocationConfig> locations = locationconf.getLocations();
	for (std::map<std::string, LocationConfig>::iterator it = locations.begin(); it != locations.end(); it++){
		(*it).second.set_indent(locationconf.getIndent() + 1);
		os <<  indent << "location\t\t" << (*it).first << " {" << std::endl 
			<< ((*it).second) << "" << std::endl;
		}
	os << indent << "}";
	return os;
}