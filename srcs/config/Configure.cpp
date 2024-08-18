/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Configure.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lgiband <lgiband@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/11/04 11:47:09 by gtoubol           #+#    #+#             */
/*   Updated: 2022/12/02 16:57:41 by lgiband          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


/**
 * @file configure.cpp
 */

#include <algorithm>
#include <arpa/inet.h>
#include <fstream>
#include <iostream>
#include <map>
#include <netdb.h>
#include <new>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include "Configure.hpp"
#include "ConfigEntry.hpp"
#include "ConfigTree.hpp"
#include "utils.hpp"

extern int flags;

Configure::Configure(std::string const& file):
	filename(file),
	_ifs(),
	_status(1),
	server_list(),
	duoIVS(),
	n_line(0)
{
	ConfigTree parse_tree;
	this->tree = &parse_tree;
	_ifs.open(filename.c_str());
	if (_ifs.good())
	{
		_status = 0;
		this->readFile();
	}
	else
	{
		//if (flags & FLAG_VERBOSE)
			std::cerr << "Error: " << filename << ": cannot read the file." << std::endl;
		_status = 1;
	}
	_ifs.close();
	this->tree = NULL;
}

std::vector<VirtualServer> const& Configure::getServers(void) const
{
	return (this->server_list);
}

std::map<std::string, std::vector<VirtualServer*> > const& Configure::getDuoIVS(void) const
{
	return (this->duoIVS);
}

int Configure::isGood(void) const
{
	return (_status == 0);
}

/**
 * @brief Read the file and wrap the lexer/parser
 *
 * @return
 * @exception
 */
int Configure::readFile(void)
{
	std::string current_line;

	current_line.reserve(8192);
	while (this->readLine(current_line))
		;
	if (_ifs.fail() && _ifs.bad())
	{
		_status = 1;
		//if (flags & FLAG_VERBOSE)
			std::cerr << "Error: " << this->filename << ": unreadable.\n";
	}
	if (this->isGood())
	{
		this->TreeToServers();
	}
	return (0);
}

/**
 * @fix fin de ficher != \n
 *
 */
bool	Configure::readLine(std::string &current_line)
{
	char	buffer[8192];
	ssize_t	count;

	_ifs.getline(buffer, 8192);
	count = _ifs.gcount();
	if (count >= 0 and count < 8192)
	{
		++this->n_line;
		current_line.append(buffer);
		this->parse(current_line);
		current_line.clear();
	}
	return (_ifs.good());
}

void	Configure::addEntryToTree(ConfigEntry const& entry)
{
	ConfigTree	*current_level;
	size_t		i;

	current_level = this->tree;
	for (i = 0; i < entry.getLevel() / 2; ++i)
	{
		if (current_level->getLeaves().empty())
			break;
		current_level = &(current_level->getLeaves().back());
	}
	if (entry.getKey() != "")
	{
		if (entry.getLevel() / 2 == i and entry.getLevel() % 2 == 0)
			current_level->getLeaves().push_back(ConfigTree(entry));
		else
			this->parseError("bad block level");
	}
	else if (entry.hasDelimiter() or (entry.getValue() != "" and entry.getValue()[0] != '#'))
	{
		this->parseError("invalid entry");
	}
}

void	Configure::parse(std::string const& line)
{
	ConfigEntry	entry(line, this->n_line);

	this->addEntryToTree(entry);
}

void	Configure::TreeToServers(void)
{
	// We go through each server and set each properties
	for (std::vector<ConfigTree>::const_iterator it_server = this->tree->getLeaves().begin();
		 it_server != this->tree->getLeaves().end(); ++it_server)
	{
		VirtualServer	current_server;
		if (it_server->getKey() != "server")
		{
			this->putError(it_server->getKey() + ": bad key level", it_server->getLineNumber());
			continue ;
		}
		if (not it_server->hasDelimiter())
		{
			this->putError(it_server->getKey() + ": missing delimiter", it_server->getLineNumber());
			continue ;
		}
		if (it_server->getValue() != "")
		{
			this->putError("server: unexpected value", it_server->getLineNumber());
			continue ;
		}
		this->setServerProperties(*it_server, current_server);
		this->server_list.push_back(current_server);
		this->duoIVS[current_server.getHost() + ":" + current_server.getPort()];
	}
	this->setDuoIVS();
}

void	Configure::setServerProperties(ConfigTree const& node, VirtualServer& server)
{
	const t_server_pair	function_tab[] = {
		{"listen",			&Configure::addListen},
		{"root",			&Configure::addRoot},
		{"server_name",		&Configure::addServerName},
		{"location",		&Configure::addLocation},
		{"index",			&Configure::addIndex},
		{"permissions",		&Configure::addPermission},
		{"max_body_size",	&Configure::addMaxBodySize},
		{"autoindex",		&Configure::addAutoindex},
		{"error_pages",		&Configure::addErrorPages}
	};
	const std::string	minimal_req[] = {
		"listen",
		"root",
		"index"
	};
	bool				executed;
	bool				has_duplicates;

	// Check duplicates ///////////////////////////////////////////////////////
	has_duplicates = false;
	for (size_t i = 0; i < sizeof(function_tab) / sizeof(function_tab[0]); ++i)
	{
		if (function_tab[i].str != "location"
			and std::count(
				node.getLeaves().begin(),
				node.getLeaves().end(),
				function_tab[i].str) > 1)
		{
			this->putError("server: " + function_tab[i].str + ": multiple definition", node.getLineNumber());
			has_duplicates = true;
		}
	}

	if (has_duplicates)
		return ;

	// Check presence of minimal requirements /////////////////////////////////
	has_duplicates = false;
	for (size_t i = 0; i < sizeof(minimal_req) / sizeof(minimal_req[0]); ++i)
	{
		if (std::count(
				node.getLeaves().begin(),
				node.getLeaves().end(),
				minimal_req[i]) == 0)
		{
			this->putError("server: " + minimal_req[i] + ": missing declaration", node.getLineNumber());
			has_duplicates = true;
		}
	}
	if (has_duplicates)
		return ;

	// Set general properties /////////////////////////////////////////////////
	for (std::vector<ConfigTree>::const_iterator server_prop = node.getLeaves().begin();
		 server_prop != node.getLeaves().end();
		 ++server_prop
		)
	{
		if (server_prop->getKey() == "location")
			continue ;
		executed = false;
		for (size_t i = 0; i < sizeof(function_tab) / sizeof(function_tab[0]); ++i)
		{
			if (server_prop->getKey() == function_tab[i].str)
			{
				(this->*(function_tab[i].fnc))(*server_prop, server);
				executed = true;
				break ;
			}
		}
		if (not executed)
			this->putError(server_prop->getKey() + ": unknown property", node.getLineNumber());
	}

	// Set locations //////////////////////////////////////////////////////////
	for (std::vector<ConfigTree>::const_iterator server_prop = node.getLeaves().begin();
		 server_prop != node.getLeaves().end();
		 ++server_prop
		)
	{
		if (server_prop->getKey() != "location")
			continue ;
		this->addLocation(*server_prop, server);
	}

	if (this->isGood() and server.getLocationPool().find("/") == server.getLocationPool().end())
	{
		server.addLocation("/", Location(server));
	}
}

///////////////////////////////////////////////////////////////////////////////
//                         Listen-related properties                         //
///////////////////////////////////////////////////////////////////////////////
void	Configure::addListen(ConfigTree const& node, VirtualServer& server)
{
	std::string port = "";
	std::string host = "";
	if (not node.getLeaves().empty())
	{
		this->putError("listen: unexpected properties", node.getLineNumber());
		return ;
	}
	if (not node.hasDelimiter())
	{
		this->putError("listen: bad format", node.getLineNumber());
		return ;
	}
	if (not this->setPort(node.getValue(), server, node.getLineNumber()))
	{
		return ;
	}
	this->setHost(node.getValue(), server, node.getLineNumber());
}

bool	Configure::setPort(std::string const& value, VirtualServer& server, size_t line_nb)
{
	std::string port_str;
	std::string::const_reverse_iterator rit;
	std::string::const_iterator it;
	long int port_nbr;
	char *end;

	for (rit = value.rbegin(); rit != value.rend(); ++rit)
	{
		if (not isdigit(*rit))
			break ;
	}
	port_str.assign(rit.base(), value.end());
	if (port_str != "")
	{
		port_nbr = strtol(port_str.c_str(), &end, 10);
		if (*end == '\0' and port_nbr >= 0 and port_nbr < MAX_PORT_NBR)
		{
			server.setPort(port_str);
			return (true);
		}
	}
	this->putError("listen: bad port format", line_nb);
	return (false);
}

void Configure::setHost(std::string const& value, VirtualServer& server, size_t line_nb)
{
	std::string host_str;
	std::string::const_reverse_iterator rit;
	std::string::const_iterator it;

	for (rit = value.rbegin(); rit != value.rend(); ++rit)
	{
		if (not isdigit(*rit))
			break;
	}
	for (it = value.begin(); it != rit.base(); ++it)
	{
		if (not isspace(*it))
			break ;
	}
	if (*rit != ':' && rit.base() != it)
	{
		this->putError("listen: bad format", line_nb);
		return ;
	}
	else if (*rit == ':')
		++rit;
	host_str.assign(it, rit.base());
	if (host_str == "")
	{
		host_str = "0.0.0.0";
	}
	this->validHost(host_str, server, line_nb);
}

bool Configure::validHost(std::string const& address, VirtualServer& server, size_t line_nb)
{
	const struct addrinfo hint = {
		AI_PASSIVE | AI_CANONNAME,
		AF_INET,
		SOCK_STREAM,
		0,
		0,
		NULL,
		NULL,
		NULL
	};
	struct addrinfo *res;
	char buffer[INET_ADDRSTRLEN];

	(void)server;
	(void)hint;
	if (getaddrinfo(address.c_str(), server.getPort().c_str(), &hint, &res))
	{
		this->putError("listen: could not resolve `" + address + "`", line_nb);
		return (false);
	}
	inet_ntop(res->ai_family, &((struct sockaddr_in *)res->ai_addr)->sin_addr, buffer, INET_ADDRSTRLEN);
	freeaddrinfo(res);
	server.setHost(buffer);
	return (true);
}

///////////////////////////////////////////////////////////////////////////////
//                                    Root                                   //
///////////////////////////////////////////////////////////////////////////////
template<class T>
void	Configure::addRoot(ConfigTree const& node, T &server)
{
	std::string root;

	if (not node.getLeaves().empty())
	{
		this->putError("root: unexpected properties", node.getLineNumber());
		return ;
	}
	if (not node.hasDelimiter())
	{
		this->putError("root: missing delimiter", node.getLineNumber());
		return ;
	}
	root = node.getValue();
	if (root == "" or root[0] != '/')
	{
		return (this->putError("root: expect absolut path", node.getLineNumber()));
	}
	server.setRoot(root);
}

///////////////////////////////////////////////////////////////////////////////
//                                   Index                                   //
///////////////////////////////////////////////////////////////////////////////
template<class T>
void	Configure::addIndex(ConfigTree const& node, T& server)
{
	std::string index;

	if (not node.getLeaves().empty())
	{
		this->putError("index: unexpected properties", node.getLineNumber());
		return ;
	}
	if (not node.hasDelimiter())
	{
		this->putError("index: missing delimiter", node.getLineNumber());
		return ;
	}
	index = node.getValue();
	if (index == "")
	{
		return (this->putError("index: empty value", node.getLineNumber()));
	}
	server.setIndex(index);
}

///////////////////////////////////////////////////////////////////////////////
//                                 Autoindex                                 //
///////////////////////////////////////////////////////////////////////////////
template<class T>
void	Configure::addAutoindex(ConfigTree const& node, T& server)
{
	std::string value;

	if (not node.getLeaves().empty())
	{
		this->putError("autoindex: unexpected properties", node.getLineNumber());
		return ;
	}
	if (not node.hasDelimiter())
	{
		this->putError("autoindex: missing delimiter", node.getLineNumber());
		return ;
	}
	value = node.getValue();
	if (value == "on")
		return (server.setAutoindex(true));
	if (value == "off")
		return (server.setAutoindex(false));
	return (this->putError("autoindex: invalid value", node.getLineNumber()));
}

///////////////////////////////////////////////////////////////////////////////
//                                Server_name                                //
///////////////////////////////////////////////////////////////////////////////
// For now: only one name per server-block

void	Configure::addServerName(ConfigTree const& node, VirtualServer& server)
{
	std::string::const_iterator it;
	std::string server_name;

	if (not node.getLeaves().empty())
	{
		this->putError("server_name: unexpected properties", node.getLineNumber());
		return ;
	}
	if (not node.hasDelimiter())
	{
		this->putError("server_name: missing delimiter", node.getLineNumber());
		return ;
	}
	for (it = node.getValue().begin(); it != node.getValue().end(); ++it)
	{
		if (isalnum(*it) or (*it == '.') or (*it == '-') or (*it == '_'))
			server_name.push_back(tolower(*it));
		else
			break ;
	}
	if (it != node.getValue().end())
	{
			this->putError("server_name: invalid character", node.getLineNumber());
			return ;
	}
	if (server_name == "")
	{
		this->putError("server_name: invalid value", node.getLineNumber());
		return ;
	}
	server.setServerName(server_name);
}

///////////////////////////////////////////////////////////////////////////////
//                                Permissions                                //
///////////////////////////////////////////////////////////////////////////////
template<class T>
void	Configure::addPermission(ConfigTree const& node, T& server)
{
	std::string permissions;
	long n;
	char *pos;

	if (not node.getLeaves().empty())
	{
		this->putError("permissions: unexpected properties", node.getLineNumber());
		return ;
	}
	if (not node.hasDelimiter())
	{
		this->putError("permissions: missing delimiter", node.getLineNumber());
		return ;
	}
	permissions = node.getValue();
	n = strtol(permissions.c_str(), &pos, 10);
	if (n < 0 or n >= 8 or *pos != '\0')
	{
		this->putError("permissions: invalid value", node.getLineNumber());
		return;
	}
	server.setPermissions(n);
}

///////////////////////////////////////////////////////////////////////////////
//                               Max Body Size                               //
///////////////////////////////////////////////////////////////////////////////
template<class T>
void	Configure::addMaxBodySize(ConfigTree const& node, T& server)
{
	std::string size_str;
	long n;
	char *pos;

	if (not node.getLeaves().empty())
	{
		this->putError("max_body_size: unexpected properties", node.getLineNumber());
		return ;
	}
	if (not node.hasDelimiter())
	{
		this->putError("max_body_size: missing delimiter", node.getLineNumber());
		return ;
	}
	size_str = node.getValue();
	n = strtol(size_str.c_str(), &pos, 10);
	if (size_str == "" or n < 0 or *pos != '\0')
	{
		this->putError("max_body_size: invalid value", node.getLineNumber());
		return;
	}
	server.setMaxBodySize(static_cast<size_t>(n));
}

///////////////////////////////////////////////////////////////////////////////
//                                  Redirect                                 //
///////////////////////////////////////////////////////////////////////////////
void	Configure::addRedirect(ConfigTree const& node, Location& location)
{
	std::string redirect;

	if (not node.getLeaves().empty())
	{
		this->putError("redirection: unexpected properties", node.getLineNumber());
		return ;
	}
	if (not node.hasDelimiter())
	{
		this->putError("redirection: missing delimiter", node.getLineNumber());
		return ;
	}
	redirect = node.getValue();
	if (redirect == "")
	{
		this->putError("redirection: invalid value", node.getLineNumber());
		return;
	}
	location.setRedirect(redirect);
}

///////////////////////////////////////////////////////////////////////////////
//                                 DefautFile                                //
///////////////////////////////////////////////////////////////////////////////
void	Configure::addDefaultFile(ConfigTree const& node, Location &location)
{
	std::string::const_iterator it;
	std::string ::const_reverse_iterator rit;
	std::string file;

	if (not node.getLeaves().empty())
	{
		this->putError("default_file: unexpected properties", node.getLineNumber());
		return ;
	}
	if (not node.hasDelimiter())
	{
		this->putError("default_file: missing delimiter", node.getLineNumber());
		return ;
	}
	file = node.getValue();
	if (file == "" or file[0] != '/')
	{
		return (this->putError("default_file: expect absolut path", node.getLineNumber()));
	}
	location.setDefaultFile(file);
}

///////////////////////////////////////////////////////////////////////////////
//                                  Post_dir                                 //
///////////////////////////////////////////////////////////////////////////////
void	Configure::addPostDir(ConfigTree const& node, Location &location)
{
	std::string::const_iterator it;
	std::string ::const_reverse_iterator rit;
	std::string file;

	if (not node.getLeaves().empty())
	{
		this->putError("post_dir: unexpected properties", node.getLineNumber());
		return ;
	}
	if (not node.hasDelimiter())
	{
		this->putError("post_dir: missing delimiter", node.getLineNumber());
		return ;
	}
	file = node.getValue();
	if (file == "" or file[0] != '/')
	{
		return (this->putError("post_dir: expect absolut path", node.getLineNumber()));
	}
	location.setPostDir(file);
}

///////////////////////////////////////////////////////////////////////////////
//                                  Location                                 //
///////////////////////////////////////////////////////////////////////////////
void	Configure::addLocation(ConfigTree const& node, VirtualServer& server)
{
	std::string::const_iterator it;
	std::string value;
	Location	location(server);

	if (not node.hasDelimiter())
	{
		this->putError("location: missing delimiter", node.getLineNumber());
		return ;
	}
	for (it = node.getValue().begin(); it != node.getValue().end(); ++it)
	{
		if (isalnum(*it) or (*it == '/') or (*it == '.') or (*it == '-') or (*it == '_'))
		{
			value.push_back(*it);
			continue ;
		}
		break ;
	}
	if (it != node.getValue().end())
	{
		this->putError("location: invalid value", node.getLineNumber());
		return ;
	}
	value = reformatUri(value);
	if (value == "")
	{
		this->putError("location: invalid value", node.getLineNumber());
		return ;
	}
	if (value[0] != '/')
	{
		this->putError("location: location needs to start with a `/`", node.getLineNumber());
		return ;
	}
	if (value[value.size() - 1] != '/')
	{
		value.push_back('/');
	}
	if (server.getLocationPool().find(value) != server.getLocationPool().end())
	{
		this->putError("location: `" + value + "` is already defined", node.getLineNumber());
		return ;
	}
	this->setLocation(node, location);
	server.getLocationPool()[value] = location;
}

void	Configure::setLocation(ConfigTree const& node, Location& location)
{
	const t_location_pair	function_tab[] = {
		{"root",			&Configure::addRoot},
		{"index",			&Configure::addIndex},
		{"permissions",		&Configure::addPermission},
		{"max_body_size",	&Configure::addMaxBodySize},
		{"autoindex",		&Configure::addAutoindex},
		{"redirect",		&Configure::addRedirect},
		{"default_file",	&Configure::addDefaultFile},
		{"post_dir",		&Configure::addPostDir},
		{"cgi",				&Configure::addCGI}
	};
	bool					executed;
	bool					has_duplicates;

	has_duplicates = false;
	for (size_t i = 0; i < sizeof(function_tab) / sizeof(function_tab[0]); ++i)
	{
		if (std::count(node.getLeaves().begin(), node.getLeaves().end(), function_tab[i].str) > 1)
		{
			this->putError("location: " + function_tab[i].str + ": multiple definitions", node.getLineNumber());
			has_duplicates = true;
		}
	}
	if (has_duplicates)
		return ;

	for (std::vector<ConfigTree>::const_iterator location_prop = node.getLeaves().begin();
		 location_prop != node.getLeaves().end();
		 ++location_prop
		)
	{
		executed = false;
		for (size_t i = 0; i < sizeof(function_tab) / sizeof(function_tab[0]); ++i)
		{
			if (location_prop->getKey() == function_tab[i].str)
			{
				(this->*(function_tab[i].fnc))(*location_prop, location);
				executed = true;
				break ;
			}
		}
		if (not executed)
			this->putError(location_prop->getKey() + ": unknown property", node.getLineNumber());
	}
}

///////////////////////////////////////////////////////////////////////////////
//                                Error Pages                                //
///////////////////////////////////////////////////////////////////////////////
void	Configure::addErrorPages(ConfigTree const& node, VirtualServer& server)
{
	if (not node.hasDelimiter())
	{
		this->putError("error_pages: missing delimiter", node.getLineNumber());
		return ;
	}
	if (node.getValue() != "")
	{
		this->putError("error_pages: unexpected value", node.getLineNumber());
		return ;
	}
	for (
		std::vector<ConfigTree>::const_iterator it = node.getLeaves().begin();
		it != node.getLeaves().end();
		++it
		)
	{
		this->addSingleErrorPage(*it, server);
	}
}

void	Configure::addSingleErrorPage(ConfigTree const& node, VirtualServer& server)
{
	long	n;
	char	*pos;

	if (not node.hasDelimiter())
	{
		this->putError("error_page: missing delimiter", node.getLineNumber());
		return ;
	}
	if (not node.getLeaves().empty())
	{
		this->putError("error_page: unexpected properties", node.getLineNumber());
		return ;
	}
	n = strtol(node.getKey().c_str(), &pos, 10);
	if (n < 0 or n > 1000 or pos[0] != '\0')
	{
		this->putError("error_page: invalid key", node.getLineNumber());
		return ;
	}
	if (node.getValue() == "" or node.getValue()[0] != '/')
	{
		this->putError("error_page: invalid value", node.getLineNumber());
		return ;
	}
	server.addErrorPage(static_cast<int>(n), node.getValue());
}

///////////////////////////////////////////////////////////////////////////////
//                                  CGI Path                                 //
///////////////////////////////////////////////////////////////////////////////
void	Configure::addCGI(ConfigTree const& node, Location& location)
{
	if (not node.hasDelimiter())
	{
		this->putError("cgi: missing delimiter", node.getLineNumber());
		return ;
	}
	if (node.getValue() != "")
	{
		this->putError("cgi: unexpected value", node.getLineNumber());
		return ;
	}
	for (
		std::vector<ConfigTree>::const_iterator it = node.getLeaves().begin();
		it != node.getLeaves().end();
		++it
		)
	{
		this->addSingleCGI(*it, location);
	}
}

void	Configure::addSingleCGI(ConfigTree const& node, Location& location)
{
	if (not node.hasDelimiter())
	{
		this->putError("cgi: missing delimiter", node.getLineNumber());
		return ;
	}
	if (not node.getLeaves().empty())
	{
		this->putError("cgi: unexpected properties", node.getLineNumber());
		return ;
	}
	if (node.getValue() == "" or node.getValue()[0] != '/')
	{
		this->putError("cgi: invalid value", node.getLineNumber());
		return ;
	}
	if (location.getCgiPerm().find(node.getKey()) != location.getCgiPerm().end())
	{
		this->putError("cgi: " + node.getKey() + ": execution already defined", node.getLineNumber());
		return ;
	}
	location.addCGIPerm(node.getKey(), node.getValue());
}


///////////////////////////////////////////////////////////////////////////////
//                          Error-related functions                          //
///////////////////////////////////////////////////////////////////////////////
void	Configure::parseError(std::string const& msg)
{
	//if (flags & FLAG_VERBOSE)
		std::cerr << "Bad config: line " << this->n_line
			  << ": " << msg << std::endl;
	_status = 1;
}

void	Configure::putError(std::string const& msg, size_t n_line)
{
	//if (flags & FLAG_VERBOSE)
		std::cerr << "Bad config: line " << n_line << ": " << msg << std::endl;
	_status = 1;
}

//addDuoIVS
void	Configure::addDuoIVS(std::string name, std::vector<VirtualServer*> list)
{
	this->duoIVS[name] = list;
}

bool	str_endswith(std::string const& str, std::string const& suffix)
{
	if (str.length() < suffix.length())
		return (false);
	else
		return (std::equal(suffix.rbegin(), suffix.rend(), str.rbegin()));
}

///////////////////////////////////////////////////////////////////////////////
//                                 SetDuoIVS                                 //
///////////////////////////////////////////////////////////////////////////////

void	Configure::setDuoIVS(void)
{
	std::string suffix;
	std::string fullname;

	for (
		std::vector<VirtualServer>::iterator server_it = this->server_list.begin();
		server_it != this->server_list.end();
		++server_it
		)
	{
		suffix = ":" + server_it->getPort();
		fullname = server_it->getHost() + suffix;
		for (
			std::map<std::string, std::vector<VirtualServer*> >::iterator inter_it = this->duoIVS.begin();
			inter_it != this->duoIVS.end();
			++inter_it
			)
		{
			if (str_endswith(inter_it->first, suffix))
			{
				if (inter_it->first == fullname or server_it->getHost() == "0.0.0.0")
				{
					inter_it->second.push_back(&(*server_it));
				}
			}
		}
	}
	for (std::map<std::string, std::vector<VirtualServer*> >::const_iterator it = this->duoIVS.begin(); it != this->duoIVS.end(); it++)
	{
		//if (flags & FLAG_VERBOSE)
			std::cerr << " [ duoIVS (config side) ] " << it->first << " " << it->second.size() << std::endl;
	}
}
