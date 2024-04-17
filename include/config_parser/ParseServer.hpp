/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ParseServer.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jschott <jschott@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/04 16:51:38 by jschott           #+#    #+#             */
/*   Updated: 2024/04/17 15:40:15 by jschott          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PARSESERVER_HPP
# define PARSESERVER_HPP

#include <string>
#include <map>
#include <vector>
#include <iostream>
// #include "ParseLocation.hpp"
#include "ReadConfig.hpp"

class ParseLocation;

class ParseServer
{
private:
	std::vector<size_t>						_ports; // ports 0-1023 well-known | 1024-49151 registered | 49152-65535 dynamic & private port
	std::map<std::string, ParseLocation*>	_locations;
	std::string								_host;
	

	std::map<std::string, std::vector<size_t> >	_listen;

//OPTIONAL INFORMATION
	std::vector<std::string>	_server_names;
	std::string					_error_path;

//INDECES
	// typedef	void	(ParseLocation::*FuncPtr())(std::string);
	std::map<std::string, bool>							_directives_index;
	// std::map<std::string, FuncPtr> 						_validation_index;
	std::map<std::string, void (ParseServer::*)(std::deque<std::string>::iterator begin, std::deque<std::string>::iterator end)> _validation_index;
	// std::map<std::string, std::pair<bool, FuncPtr> >	_index;

	void init();


public:

	ParseServer(std::vector<size_t> ports);
	ParseServer(ParseServer const & origin);
	ParseServer(std::vector<std::string> name, std::vector<size_t> listen, std::string err, std::map<std::string, ParseLocation*> location);
	ParseServer(std::deque<std::string> tokens, std::deque<std::string>::iterator begin, std::deque<std::string>::iterator end);
	ParseServer & operator= (ParseServer const & origin);
	
	~ParseServer();

	// void 				addLocation(std::pair<std::string, ParseLocation*> location);
	void 				addLocation(std::string location, ParseLocation* config);
	void				setErrorPath(std::string error_path);
	void				setServerName(std::vector<std::string> server_name);

	std::vector<size_t>							getListenPorts() const;
	std::map<std::string, ParseLocation*> const	getLocations() const;
	ParseLocation const							getLocation(std::string directory);
	std::vector<std::string> const				getServerName();
	std::string	const							getErrorPath() const;

	void	parseServerDirective(std::deque<std::string>::iterator begin, std::deque<std::string>::iterator end);
	void	validatePort(std::deque<std::string>::iterator begin, std::deque<std::string>::iterator end);
	void	validateLocation(std::deque<std::string>::iterator begin, std::deque<std::string>::iterator end);
	void	validateHost(std::deque<std::string>::iterator begin, std::deque<std::string>::iterator end);
	void	validateServerName(std::deque<std::string>::iterator begin, std::deque<std::string>::iterator end);
	void	validateErrorPath(std::deque<std::string>::iterator begin, std::deque<std::string>::iterator end);
};

const ParseServer*	parseServer(std::deque<std::string> tokens, std::deque<std::string>::iterator begin, std::deque<std::string>::iterator end);

std::ostream& operator<<(std::ostream& os, const ParseServer& serverconf);

#endif 