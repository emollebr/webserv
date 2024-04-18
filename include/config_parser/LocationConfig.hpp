/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocationConfig.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jschott <jschott@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/04 16:55:56 by jschott           #+#    #+#             */
/*   Updated: 2024/04/17 15:57:53 by jschott          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOCATIONCONFIG_HPP
# define LOCATIONCONFIG_HPP

#include <string>
#include <vector>
#include <deque>
#include "ReadConfig.hpp"

class LocationConfig
{
private:
	std::string					_root; // /var/ww/html
	
// OPTIONAL VALUES
	std::vector<std::string>	_indeces; // index.html index.php
	std::vector<std::string>	_methods_allowed; // GET POST DELETE
	std::string					_redirect; // /new-location
	std::string					_CGI; // /cgi_bin/script
	size_t						_max_body_size;
	std::string					_default_file; // index.html
	std::string					_upload_location; // /uploads
	std::string					_cgi_extension; // .php
	bool						_allow_get;
	bool						_allow_post;
	bool						_autoindex;
	

	std::map<std::string, bool>	_directives_set;
	std::map<std::string, void (LocationConfig::*)(tokeniterator begin, tokeniterator end)> 
								_directives_validation_funcs;

	void						init();

public:
	LocationConfig();
	LocationConfig(std::string root);
	LocationConfig(tokeniterator begin, tokeniterator end);
	LocationConfig(LocationConfig const & origin);
	LocationConfig & operator=(LocationConfig const & origin);
	~LocationConfig();

	std::string					getRoot() const;
	std::vector<std::string>	getIndeces() const;
	std::vector<std::string>	getMethods() const;
	std::string					getRedirect() const;
	std::string					getCGI() const;
	size_t						getBodySize() const;
	std::string					getDefaultFile() const;
	std::string					getUploadLocation() const;
	std::string					getCGIExtension() const;
	bool						getAllowGET() const;
	bool						getAllowPOST() const;
	bool						getAutoindex() const;

	void						parseLocationDirective(tokeniterator begin, tokeniterator end);
	void						validateRoot(tokeniterator begin, tokeniterator end);
	void						validateIndeces(tokeniterator begin, tokeniterator end);
	void						validateMethods(tokeniterator begin, tokeniterator end);
	void						validateRedirect(tokeniterator begin, tokeniterator end);
	void						validateCGI(tokeniterator begin, tokeniterator end);
	void						validateBodySize(tokeniterator begin, tokeniterator end);
	void						validateDefaultFile(tokeniterator begin, tokeniterator end);
	void						validateUploadLocation(tokeniterator begin, tokeniterator end);
	void						validateCGIExtension(tokeniterator begin, tokeniterator end);
	void						validateAllowGET(tokeniterator begin, tokeniterator end);
	void						validateAllowPOST(tokeniterator begin, tokeniterator end);
	void						validateAutoindex(tokeniterator begin, tokeniterator end);

	bool						hasIndex(std::string index);
	bool						hasMethod(std::string method);
};

std::ostream& operator<<(std::ostream& os, const LocationConfig& locationconf);

#endif
