/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jschott <jschott@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/04 16:51:38 by jschott           #+#    #+#             */
/*   Updated: 2024/04/18 15:06:48 by jschott          ###   ########.fr       */
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
	std::string									_host;
	std::vector<size_t>							_ports; // ports 0-1023 well-known | 1024-49151 registered | 49152-65535 dynamic & private port
	std::map<std::string, LocationConfig>		_locations;
	

	std::map<std::string, std::vector<size_t> >	_listen;

//OPTIONAL INFORMATION
	std::vector<std::string>					_server_names;
	std::string										_error_path;

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
	ServerConfig(std::deque<std::string> tokens, tokeniterator begin, tokeniterator end);
	ServerConfig & operator= (ServerConfig const & origin);
	
	~ServerConfig();

	// void 				addLocation(std::pair<std::string, LocationConfig> location);
	void 				addLocation(std::string location, LocationConfig config);
	void				setErrorPath(std::string error_path);
	void				setServerName(std::vector<std::string> server_name);

	std::vector<size_t>							getListenPorts() const;
	std::string									getHost() const;
	std::map<std::string, LocationConfig> const	getLocations() const;
	LocationConfig const						getLocation(std::string directory) const;
	std::vector<std::string> const				getServerNames() const;
	std::string	const							getErrorPath() const;

	void	parseServerDirective(tokeniterator begin, tokeniterator end);
	void	validatePort(tokeniterator begin, tokeniterator end);
	void	validateLocation(tokeniterator begin, tokeniterator end);
	void	validateHost(tokeniterator begin, tokeniterator end);
	void	validateServerName(tokeniterator begin, tokeniterator end);
	void	validateErrorPath(tokeniterator begin, tokeniterator end);

	class InvalidConfigException : public std::exception{
		public:
			virtual const char* what() const throw(){
				return ("Error: Invalid data");
			};
	};

	class InvalidDirectiveException : public std::exception{
			// std::string message;
		public:
			// InvalidDirectiveException(const std::string &msg) : message(msg){}
			virtual const char* what() const throw(){
				return ("Error: Invalid directive: ");
				// return message.c_str();
			}
	};
	
	class InvalidDataException : public std::exception{
		public:
			virtual const char* what() const throw(){
				return ("Error: Invalid data");
			};
	};
};

const ServerConfig*	parseServer(std::deque<std::string> tokens, tokeniterator begin, tokeniterator end);

std::ostream& operator<<(std::ostream& os, const ServerConfig& serverconf);

#endif 