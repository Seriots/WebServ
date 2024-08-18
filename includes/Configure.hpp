/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Configure.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lgiband <lgiband@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/11/04 11:42:29 by gtoubol           #+#    #+#             */
/*   Updated: 2022/11/30 13:58:50 by lgiband          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/** @file configure.hpp */

#ifndef CONFIGURE_HPP
#define CONFIGURE_HPP
#pragma once

#include <cstddef>
#include <fstream>
#include <map>
#include <string>
#include <vector>
#include "ConfigEntry.hpp"
#include "ConfigTree.hpp"
#include "VirtualServer.hpp"

// number of available ports
#define MAX_PORT_NBR 65536
#define MAX_LINE_SIZE 8192

/**
 * @class Configure
 * @brief Tool to load and parse the configuration file.
 *
 * Wrapper around the lexer/parser of the configuration files (yaml style)
 */
class Configure
{
public:
	Configure(std::string const&);
	virtual ~Configure() {}

	int	isGood(void) const;
	std::vector<VirtualServer>							const& getServers(void) const;
	std::map<std::string, std::vector<VirtualServer*> >	const& getDuoIVS(void) const;

	void	addDuoIVS(std::string, std::vector<VirtualServer*>);

private:
	int		readFile(void);
	bool	readLine(std::string &);
	void	parse(std::string const&);
	void	parseError(std::string const&);
	void	putError(std::string const&, size_t);
	void	addEntryToTree(ConfigEntry const&);
	void	TreeToServers(void);
	void	setServerProperties(ConfigTree const&, VirtualServer&);

	// List of all properties addition to a single VirtualServer
	void	addListen(ConfigTree const&, VirtualServer&);
	bool	setPort(std::string const&, VirtualServer&, size_t);
	void	setHost(std::string const&, VirtualServer&, size_t);
	bool	validHost(std::string const&, VirtualServer&, size_t);

	template<class T>
	void	addRoot(ConfigTree const&, T&);
	template<class T>
	void	addIndex(ConfigTree const&, T&);
	template<class T>
	void	addPermission(ConfigTree const&, T&);
	template<class T>
	void	addMaxBodySize(ConfigTree const&, T&);
	template<class T>
	void	addAutoindex(ConfigTree const&, T&);

	void	addServerName(ConfigTree const&, VirtualServer&);
	void	addLocation(ConfigTree const&, VirtualServer&);
	void	setLocation(ConfigTree const&, Location&);

	void	addRedirect(ConfigTree const&, Location&);
	void	addDefaultFile(ConfigTree const&, Location&);
	void	addPostDir(ConfigTree const&, Location&);
	void	setDuoIVS(void);
	void	addErrorPages(ConfigTree const&, VirtualServer&);
	void	addSingleErrorPage(ConfigTree const&, VirtualServer&);

	void	addCGI(ConfigTree const&, Location&);
	void	addSingleCGI(ConfigTree const&, Location&);


	std::string											filename;
	std::ifstream										_ifs;
	int													_status;
	std::vector<VirtualServer>							server_list;
	std::map<std::string, std::vector<VirtualServer*> >	duoIVS;
	size_t												n_line;
	ConfigTree											*tree;
};

bool	str_endswith(std::string const&, std::string const&);

typedef void (Configure::*t_server_func)(ConfigTree const&, VirtualServer&);
typedef void (Configure::*t_location_func)(ConfigTree const&, Location&);

typedef struct s_server_pair
{
	std::string		str;
	t_server_func	fnc;
}	t_server_pair;

typedef struct s_location_pair
{
	std::string str;
	t_location_func fnc;
}	t_location_pair;


#endif
