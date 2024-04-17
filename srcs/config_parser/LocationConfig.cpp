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
	std::string	directives[] = {"root", "index", "methods",
									"redirect", "CGI", "max_body",
									"default_file", "upload_location", 
									"cgi_extension", "allow_get", "allow_post", 
									"autoindex"};
	typedef void (LocationConfig::*LocationConfigFunction)(std::deque<std::string>::iterator begin, std::deque<std::string>::iterator end);
	LocationConfigFunction functions[] = {&LocationConfig::validateRoot, &LocationConfig::validateIndeces, &LocationConfig::validateMethods, 
											&LocationConfig::validateRedirect, &LocationConfig::validateCGI, &LocationConfig::validateBodySize, 
											&LocationConfig::validateDefaultFile, &LocationConfig::validateUploadLocation, &LocationConfig::validateCGIExtension, 
											&LocationConfig::validateAllowGET, &LocationConfig::validateAllowPOST, &LocationConfig::validateAutoindex};
	int size = sizeof(directives) / sizeof(directives[0]);
	for (int i = 0; i < size; i++){
		_directives_index[directives[i]] = false;
		_validation_index[directives[i]] = functions[i];
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
	_indeces = origin._indeces;
	_methods_allowed = origin._methods_allowed;
	_redirect = origin._redirect;
	_CGI = origin._CGI;
	_max_body_size = origin._max_body_size;
	_default_file = origin._default_file;
	_upload_location = origin._upload_location;
	_cgi_extension = origin._cgi_extension;
	_allow_get = origin._allow_get;
	_allow_get = origin._allow_get;
	return (*this);
}

LocationConfig::~LocationConfig(){
}

std::vector<std::string>	LocationConfig::getIndeces() const{
	if (_indeces.empty())
		return std::vector<std::string>();
	std::vector<std::string>	Indeces;
	Indeces = _indeces;
	return (Indeces);
}

/* bool LocationConfig::getIndex(std::string index){
	return (_);
} */

std::vector<std::string> LocationConfig::getMethods() const{
	if (_methods_allowed.empty())
		return std::vector<std::string>();
	std::vector<std::string>	methods;
	methods = _methods_allowed;
	return (methods);
}

bool LocationConfig::hasMethod(std::string method){
	if (std::find(_methods_allowed.begin(), _methods_allowed.begin(), method) != _methods_allowed.end())
		return true;
	return false;
}

std::string		LocationConfig::getRoot() const {
	return _root;
}


std::string	LocationConfig::getRedirect() const{
	return _redirect;
}

std::string	LocationConfig::getCGI() const{
	return _CGI;
}

size_t LocationConfig::getBodySize() const{
	return _max_body_size;	
}

std::string LocationConfig::getDefaultFile() const{
	return _default_file;
}

std::string LocationConfig::getUploadLocation() const{
	return _upload_location;
}

std::string LocationConfig::getCGIExtension() const{
	return _cgi_extension;
}

bool LocationConfig::getAllowGET() const{
	return _allow_get;
}

bool LocationConfig::getAllowPOST() const{
	return _allow_post;
}

bool LocationConfig::getAutoindex() const {
	return _autoindex;
}

void LocationConfig::parseLocationDirective(std::deque<std::string>::iterator begin, std::deque<std::string>::iterator end) {
	init();
	if (_directives_index.find(*begin) == _directives_index.end()) {
		return ; //THROW EXCEPTION
	}
	std::map<std::string, void (LocationConfig::*)(std::deque<std::string>::iterator, std::deque<std::string>::iterator)>::iterator function = _validation_index.find(*begin);
	if (function != _validation_index.end()) {
		(this->*(function->second))(begin + 1, end);
	}
}

void LocationConfig::validateRoot(std::deque<std::string>::iterator begin, std::deque<std::string>::iterator end){
	if (begin == end)	
		_root = (*begin);
	// 	std::cout << "ubbi" << std::endl;
	// 	return ;
	// }
}

void LocationConfig::validateIndeces(std::deque<std::string>::iterator begin, std::deque<std::string>::iterator end){
	// if (begin == end)
	// 	return ;
	while (begin <= end)
		_indeces.push_back(*begin++);
}

void LocationConfig::validateMethods(std::deque<std::string>::iterator begin, std::deque<std::string>::iterator end){
	// if (begin == end)
	// 	return ;
	while (begin < end)
		_methods_allowed.push_back(*begin++);
	
}

void LocationConfig::validateRedirect(std::deque<std::string>::iterator begin, std::deque<std::string>::iterator end){
	if (begin == end)
		_redirect = *begin;
}

void LocationConfig::validateCGI(std::deque<std::string>::iterator begin, std::deque<std::string>::iterator end){
	if (begin == end)
		_CGI = *begin;
}

void LocationConfig::validateBodySize(std::deque<std::string>::iterator begin, std::deque<std::string>::iterator end){
	if (begin == end)
		_max_body_size = atoi((*begin).c_str());
}

void LocationConfig::validateDefaultFile(std::deque<std::string>::iterator begin, std::deque<std::string>::iterator end){
	if (begin == end)
		_default_file = *begin;
}

void LocationConfig::validateUploadLocation(std::deque<std::string>::iterator begin, std::deque<std::string>::iterator end){
	if (begin == end)
		_upload_location = *begin;
}

void LocationConfig::validateCGIExtension(std::deque<std::string>::iterator begin, std::deque<std::string>::iterator end){
	if (begin == end)
		_cgi_extension = *begin;
}

void LocationConfig::validateAllowGET(std::deque<std::string>::iterator begin, std::deque<std::string>::iterator end){
	if (begin == end)
		return ;
}

void LocationConfig::validateAllowPOST(std::deque<std::string>::iterator begin, std::deque<std::string>::iterator end){
	if (begin == end)
		return ;
}

void LocationConfig::validateAutoindex(std::deque<std::string>::iterator begin, std::deque<std::string>::iterator end){
	if (begin == end)
		return ;
}

LocationConfig::LocationConfig(std::deque<std::string>::iterator begin, std::deque<std::string>::iterator end){
	// std::cout << TEXT_BOLD << "	Parsing Location from " << *begin << " to " << *end << std::endl;
	std::deque<std::string>::iterator directiveend = std::find(begin, end, ";") ;
	//IF FOUND ; CREATE TUPLE
	while (begin < end){
		directiveend = std::find(begin, end, ";") ;
		if (directiveend <= end) {
			parseLocationDirective(begin, directiveend - 1);
			begin = directiveend + 1;
		}
		else 
			std::cerr << COLOR_ERROR << "Error: Cannot parse directive" << std::endl << COLOR_STANDARD;
	}
	std::cout << TEXT_NOFORMAT;
}

std::ostream& operator<<(std::ostream& os, const LocationConfig& locationconf) {
	// os << "++START LOCATION CONFIGURATION++" << std::endl << std::endl;
	os << "		root		" << locationconf.getRoot() << ";" << std::endl;
	os << "		indices		" << "TBD" << ";" << std::endl;
	os << "		methods		" << "TBD" << ";" << std::endl;
	os << "		redirect	" << locationconf.getRedirect() << ";" << std::endl;
	os << "		CGI		" << locationconf.getCGI() << ";" << std::endl;
	os << "		body_size	" << locationconf.getBodySize() << ";" << std::endl;
	os << "		default file	" << locationconf.getDefaultFile() << ";" << std::endl;
	os << "		upload location	" << locationconf.getUploadLocation() << ";" << std::endl;
	os << "		CGI extension	" << locationconf.getCGIExtension() << ";" << std::endl;
	os << "		autoindex	" << locationconf.getAutoindex() << ";" << std::endl;
	os << "	}";
	return os;
}
