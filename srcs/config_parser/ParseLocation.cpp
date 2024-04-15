/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ParseLocation.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jschott <jschott@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/15 12:09:36 by jschott           #+#    #+#             */
/*   Updated: 2024/04/15 15:05:59 by jschott          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ParseLocation.hpp"

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
}

ParseLocation::~ParseLocation(){
}

std::vector<std::string>	ParseLocation::getIndeces(){
	if (_indeces.empty())
		return std::vector<std::string>();
	std::vector<std::string>	Indeces;
	Indeces = _indeces;
	return (Indeces);
}

/* bool ParseLocation::getIndex(std::string index){
	return (_);
} */

std::vector<std::string> ParseLocation::getMethods(){
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

std::string	ParseLocation::getRedirect(){
	return _redirect;
}

std::string	ParseLocation::getCGI(){
	return _CGI;
}

size_t ParseLocation::getBodySize(){
	return _max_body_size;	
}

std::string ParseLocation::getDefaultFile(){
	return _default_file;
}

std::string ParseLocation::getUploadLocation(){
	return _upload_location;
}

std::string ParseLocation::getCGIExtension(){
	return _cgi_extension;
}

bool ParseLocation::getAllowGET(){
	return _allow_get;
}

bool ParseLocation::getAllowPOST(){
	return _allow_post;
}

const ParseLocation * parseLocation(std::deque<std::string>::iterator begin, std::deque<std::string>::iterator end){
	std::cout << TEXT_BOLD << "	Parsing Location from " << *begin << " to " << *end << std::endl;
	std::deque<std::string>::iterator directiveend = std::find(begin, end, ";") ;
	//IF FOUND ; CREATE TUPLE
	while (begin < end){
		directiveend = std::find(begin, end, ";") ;
		if (directiveend <= end) {
			parseDirective(begin, directiveend - 1);
			begin = directiveend + 1;
		}
		else {
			std::cerr << COLOR_ERROR << "Error: Cannot parse directive" << std::endl << COLOR_STANDARD;
			return ;
		}
	}
	std::cout << TEXT_NOFORMAT;
	ParseLocation *location = new("root");
}
