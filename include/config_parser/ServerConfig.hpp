/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jschott <jschott@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/04 16:51:38 by jschott           #+#    #+#             */
/*   Updated: 2024/04/17 17:57:17 by jschott          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVERCONFIG_HPP
# define SERVERCONFIG_HPP

#include <string>
#include <map>
#include <vector>
#include <iostream>
// #include "LocationConfig.hpp"
#include "ReadConfig.hpp"

class LocationConfig;

class ServerConfig
{
private:
	std::vector<size_t>						_ports; // ports 0-1023 well-known | 1024-49151 registered | 49152-65535 dynamic & private port
	std::map<std::string, LocationConfig>	_locations;
	std::string								_host;
	

	std::map<std::string, std::vector<size_t> >	_listen;

//OPTIONAL INFORMATION
	std::vector<std::string>	_server_names;
	std::string					_error_path;

//INDECES
	// typedef	void	(LocationConfig::*FuncPtr())(std::string);
	std::map<std::string, bool>							_directives_index;
	// std::map<std::string, FuncPtr> 						_validation_index;
	std::map<std::string, void (ServerConfig::*)(std::deque<std::string>::iterator begin, std::deque<std::string>::iterator end)> _validation_index;
	// std::map<std::string, std::pair<bool, FuncPtr> >	_index;

	void init();


public:

	ServerConfig();
	ServerConfig(std::vector<size_t> ports);
	ServerConfig(ServerConfig const & origin);
	ServerConfig(std::vector<std::string> name, std::vector<size_t> listen, std::string err, std::map<std::string, LocationConfig> location);
	ServerConfig(std::deque<std::string> tokens, std::deque<std::string>::iterator begin, std::deque<std::string>::iterator end);
	ServerConfig & operator= (ServerConfig const & origin);
	
	~ServerConfig();

	// void 				addLocation(std::pair<std::string, LocationConfig> location);
	void 				addLocation(std::string location, LocationConfig config);
	void				setErrorPath(std::string error_path);
	void				setServerName(std::vector<std::string> server_name);

	std::vector<size_t>							getListenPorts() const;
	std::string									getHost() const;
	std::map<std::string, LocationConfig> const	getLocations() const;
	LocationConfig const							getLocation(std::string directory);
	std::vector<std::string> const				getServerName();
	std::string	const							getErrorPath() const;

	void	parseServerDirective(std::deque<std::string>::iterator begin, std::deque<std::string>::iterator end);
	void	validatePort(std::deque<std::string>::iterator begin, std::deque<std::string>::iterator end);
	void	validateLocation(std::deque<std::string>::iterator begin, std::deque<std::string>::iterator end);
	void	validateHost(std::deque<std::string>::iterator begin, std::deque<std::string>::iterator end);
	void	validateServerName(std::deque<std::string>::iterator begin, std::deque<std::string>::iterator end);
	void	validateErrorPath(std::deque<std::string>::iterator begin, std::deque<std::string>::iterator end);
};

const ServerConfig*	parseServer(std::deque<std::string> tokens, std::deque<std::string>::iterator begin, std::deque<std::string>::iterator end);

std::ostream& operator<<(std::ostream& os, const ServerConfig& serverconf);

#endif 