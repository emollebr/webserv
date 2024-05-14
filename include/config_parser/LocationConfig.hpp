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
#include <iterator>
#include "ReadConfig.hpp"

class LocationConfig
{
private:
	std::string									_root; 				// /var/ww/html
	
	// OPTIONAL VALUES
	std::string									_index;				// index.html index.php
	std::set<std::string>						_methods_allowed;	// GET POST DELETE
	std::pair<int, std::string>					_redirect; 			// [statusCode, directory/to/file.html]
	size_t										_max_body_size; 	// 42 | 42M | 42G
	std::string									_upload_location; 	// /uploads
	bool										_autoindex; 		// 0 | off
	std::map<std::string, LocationConfig>		_locations;			// nested location blocks

	size_t										_indent_lvl; 		// level of nestedness for printing purpose
	std::map<std::string, bool>					_directives_set;	// which directives have been set in config
	std::map<std::string, void (LocationConfig::*)(tokeniterator begin, tokeniterator end)> 
												_directives_validation_funcs;	// registry with functions that validate 

	void										init();

public:
	LocationConfig();
	LocationConfig(std::string root);
	LocationConfig(tokeniterator begin, tokeniterator end);
	LocationConfig(LocationConfig const & origin);
	LocationConfig & operator=(LocationConfig const & origin);
	~LocationConfig();

	//GETTER
	std::string									getRoot() const;
	std::string									getIndex() const;
	std::set<std::string>						getMethods() const;
	std::pair<int, std::string>					getRedirect() const;
	size_t										getBodySize() const;
	std::string									getUploadLocation() const;
	std::map<std::string, LocationConfig> const	getLocations() const;
	bool										getAllowGET() const;
	bool										getAllowPOST() const;
	bool										getAllowDELETE() const;
	bool										getAutoindex() const;
	size_t										getIndent() const;

	//SETTER
	void										set_indent(size_t new_level);

	//PARSE & VALIDATE
	void										parseLocationDirective(tokeniterator begin, tokeniterator end);
	void										validateRoot(tokeniterator begin, tokeniterator end);
	void										validateIndex(tokeniterator begin, tokeniterator end);
	void										validateMethods(tokeniterator begin, tokeniterator end);
	void										validateRedirect(tokeniterator begin, tokeniterator end);
	void										validateBodySize(tokeniterator begin, tokeniterator end);
	void										validateUploadLocation(tokeniterator begin, tokeniterator end);
	void										validateAutoindex(tokeniterator begin, tokeniterator end);

	//CHECK FOR VALUES
	bool										hasIndex(std::string index);
	bool										hasMethod(std::string method);
	bool										directiveSet(std::string directive) const;

	//FILL NESTED LOCATIONS
	void										fillNestedLocation();
};

std::ostream& operator<<(std::ostream& os, const LocationConfig& locationconf);

#endif