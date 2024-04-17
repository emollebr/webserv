/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ParseLocation.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jschott <jschott@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/15 12:09:36 by jschott           #+#    #+#             */
/*   Updated: 2024/04/17 17:18:53 by jschott          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ParseLocation.hpp"

void ParseLocation::init(){
	std::string	directives[] = {"root", "index", "methods",
									"redirect", "CGI", "max_body",
									"default_file", "upload_location", 
									"cgi_extension", "allow_get", "allow_post", 
									"autoindex"};
	typedef void (ParseLocation::*ParseLocationFunction)(std::deque<std::string>::iterator begin, std::deque<std::string>::iterator end);
	ParseLocationFunction functions[] = {&ParseLocation::validateRoot, &ParseLocation::validateIndeces, &ParseLocation::validateMethods, 
											&ParseLocation::validateRedirect, &ParseLocation::validateCGI, &ParseLocation::validateBodySize, 
											&ParseLocation::validateDefaultFile, &ParseLocation::validateUploadLocation, &ParseLocation::validateCGIExtension, 
											&ParseLocation::validateAllowGET, &ParseLocation::validateAllowPOST, &ParseLocation::validateAutoindex};
	int size = sizeof(directives) / sizeof(directives[0]);
	for (int i = 0; i < size; i++){
		_directives_index[directives[i]] = false;
		_validation_index[directives[i]] = functions[i];
	}
}

ParseLocation::ParseLocation(){
	init();
}

ParseLocation::ParseLocation(std::string root){
	_root = root;	
}

ParseLocation::ParseLocation(ParseLocation const & origin){
	*this = origin;		
}

ParseLocation & ParseLocation::operator=(ParseLocation const & origin){
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

ParseLocation::~ParseLocation(){
}

std::vector<std::string>	ParseLocation::getIndeces() const{
	if (_indeces.empty())
		return std::vector<std::string>();
	std::vector<std::string>	Indeces;
	Indeces = _indeces;
	return (Indeces);
}

/* bool ParseLocation::getIndex(std::string index){
	return (_);
} */

std::vector<std::string> ParseLocation::getMethods() const{
	if (_methods_allowed.empty())
		return std::vector<std::string>();
	std::vector<std::string>	methods;
	methods = _methods_allowed;
	return (methods);
}

bool ParseLocation::hasMethod(std::string method){
	if (std::find(_methods_allowed.begin(), _methods_allowed.begin(), method) != _methods_allowed.end())
		return true;
	return false;
}

std::string		ParseLocation::getRoot() const {
	return _root;
}


std::string	ParseLocation::getRedirect() const{
	return _redirect;
}

std::string	ParseLocation::getCGI() const{
	return _CGI;
}

size_t ParseLocation::getBodySize() const{
	return _max_body_size;	
}

std::string ParseLocation::getDefaultFile() const{
	return _default_file;
}

std::string ParseLocation::getUploadLocation() const{
	return _upload_location;
}

std::string ParseLocation::getCGIExtension() const{
	return _cgi_extension;
}

bool ParseLocation::getAllowGET() const{
	return _allow_get;
}

bool ParseLocation::getAllowPOST() const{
	return _allow_post;
}

bool ParseLocation::getAutoindex() const {
	return _autoindex;
}

void ParseLocation::parseLocationDirective(std::deque<std::string>::iterator begin, std::deque<std::string>::iterator end) {
	init();
	if (_directives_index.find(*begin) == _directives_index.end()) {
		return ; //THROW EXCEPTION
	}
	std::map<std::string, void (ParseLocation::*)(std::deque<std::string>::iterator, std::deque<std::string>::iterator)>::iterator function = _validation_index.find(*begin);
	if (function != _validation_index.end()) {
		(this->*(function->second))(begin + 1, end);
	}
}

void ParseLocation::validateRoot(std::deque<std::string>::iterator begin, std::deque<std::string>::iterator end){
	if (begin == end)	
		_root = (*begin);
	// 	std::cout << "ubbi" << std::endl;
	// 	return ;
	// }
}

void ParseLocation::validateIndeces(std::deque<std::string>::iterator begin, std::deque<std::string>::iterator end){
	// if (begin == end)
	// 	return ;
	while (begin <= end)
		_indeces.push_back(*begin++);
}

void ParseLocation::validateMethods(std::deque<std::string>::iterator begin, std::deque<std::string>::iterator end){
	// if (begin == end)
	// 	return ;
	while (begin < end)
		_methods_allowed.push_back(*begin++);
	
}

void ParseLocation::validateRedirect(std::deque<std::string>::iterator begin, std::deque<std::string>::iterator end){
	if (begin == end)
		_redirect = *begin;
}

void ParseLocation::validateCGI(std::deque<std::string>::iterator begin, std::deque<std::string>::iterator end){
	if (begin == end)
		_CGI = *begin;
}

void ParseLocation::validateBodySize(std::deque<std::string>::iterator begin, std::deque<std::string>::iterator end){
	if (begin == end)
		_max_body_size = atoi((*begin).c_str());
}

void ParseLocation::validateDefaultFile(std::deque<std::string>::iterator begin, std::deque<std::string>::iterator end){
	if (begin == end)
		_default_file = *begin;
}

void ParseLocation::validateUploadLocation(std::deque<std::string>::iterator begin, std::deque<std::string>::iterator end){
	if (begin == end)
		_upload_location = *begin;
}

void ParseLocation::validateCGIExtension(std::deque<std::string>::iterator begin, std::deque<std::string>::iterator end){
	if (begin == end)
		_cgi_extension = *begin;
}

void ParseLocation::validateAllowGET(std::deque<std::string>::iterator begin, std::deque<std::string>::iterator end){
	if (begin == end)
		return ;
}

void ParseLocation::validateAllowPOST(std::deque<std::string>::iterator begin, std::deque<std::string>::iterator end){
	if (begin == end)
		return ;
}

void ParseLocation::validateAutoindex(std::deque<std::string>::iterator begin, std::deque<std::string>::iterator end){
	if (begin == end)
		return ;
}

ParseLocation::ParseLocation(std::deque<std::string>::iterator begin, std::deque<std::string>::iterator end){
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

std::ostream& operator<<(std::ostream& os, const ParseLocation& locationconf) {
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
