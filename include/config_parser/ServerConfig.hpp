/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jschott <jschott@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/04 16:51:38 by jschott           #+#    #+#             */
/*   Updated: 2024/05/08 14:51:46 by jschott          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVERCONFIG_HPP
# define SERVERCONFIG_HPP

#include <string>
#include <map>
#include <vector>
#include <iostream>
#include <set>
#include "ReadConfig.hpp"

class LocationConfig;

class ServerConfig
{
private:
	std::string									_host; // 0.0.0.0 - 255.255.255.255 validation DONE
	std::set<size_t>							_ports; // ports 0-1023 well-known | 1024-49151 registered | 49152-65535 dynamic & private port
	std::map<std::string, LocationConfig>		_locations; // uri -> locationvalidation  validation DONE

	std::set<std::pair <std::string, size_t> > 	_host_ports_registry;

//OPTIONAL INFORMATION
	std::set<std::string>						_server_names; // www.uri.com
	std::map<unsigned int, std::string>			_error_pages;// /directory/to/errorfiles.html validation DONE
	std::map<std::string, std::string>			_cgi_extension;

//INDECES
	std::map<std::string, bool>					_directives_set;
	std::map<std::string, void (ServerConfig::*)(tokeniterator begin, tokeniterator end)> _directives_validation_funcs;
	// std::map<std::string, std::pair<bool, FuncPtr> >	_index;

	void init();


public:

	ServerConfig();
	ServerConfig(std::vector<size_t> ports);
	ServerConfig(ServerConfig const & origin);
	ServerConfig(std::vector<std::string> name, std::vector<size_t> listen, std::string err, std::map<std::string, LocationConfig> location);
	ServerConfig(tokeniterator begin, tokeniterator end);
	ServerConfig & operator= (ServerConfig const & origin);
	
	~ServerConfig();

	//SETTER
	void 										addLocation(std::string location, LocationConfig config);
	// void										setErrorPath(std::string error_path);
	void										setServerName(std::set<std::string> server_name);
	
	//GETTER
	std::set<std::pair <std::string, size_t> > 	getListen() const;
	std::set<size_t>							getListenPorts() const;
	std::string									getHost() const;
	std::map<std::string, LocationConfig> const	getLocations() const;
	LocationConfig const						getLocation(std::string directory) const;
	std::set<std::string> const					getServerNames() const;
	std::map<uint, std::string>	const 			getErrorPages() const;
	std::string	const							getErrorPath(int StatusCode) const;
	std::map<std::string, std::string>			getCGIExtention() const;

	//
	void	parseServerDirective(tokeniterator begin, tokeniterator end);
	void	validatePort(tokeniterator begin, tokeniterator end);
	void	validateLocation(tokeniterator begin, tokeniterator end);
	void	validateHost(tokeniterator begin, tokeniterator end);
	void	validateServerName(tokeniterator begin, tokeniterator end);
	void	validateErrorPath(tokeniterator begin, tokeniterator end);
	void	validateCGIExtension(tokeniterator begin, tokeniterator end);
	void	validateHostPort(tokeniterator begin, tokeniterator end);

	//UTILS
	void	deletePort(size_t port);
	void	fillDirectives(ServerConfig &reference);
	bool	isValidHost(char* IP);
	bool	isValidPort(char* port);

};

const ServerConfig*	parseServer(std::deque<std::string> tokens, tokeniterator begin, tokeniterator end);

std::ostream& operator<<(std::ostream& os, const ServerConfig& serverconf);

#endif 