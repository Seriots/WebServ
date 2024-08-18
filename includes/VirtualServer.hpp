/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   VirtualServer.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lgiband <lgiband@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/11/17 11:56:16 by gtoubol           #+#    #+#             */
/*   Updated: 2022/11/28 11:24:23 by lgiband          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef VIRTUALSERVER_HPP
# define VIRTUALSERVER_HPP
#pragma once

#include <map>
#include <string>
#include <map>
#include <vector>

#include "Location.hpp"

class VirtualServer
{
public:
	VirtualServer(void);
	virtual ~VirtualServer(void) {}

	std::string 						const& getServerName() const;
	std::string 						const& getRoot() const;
	std::string 						const& getHost() const;
	std::string 						const& getPort() const;
	int 								const& getFd()	const;
	std::map<std::string, Location>		const& getLocationPool() const;
	std::map<std::string, Location>&	getLocationPool();
	std::map<int, std::string>			const& getErrorPage() const;

	void	setServerName(std::string const&);
	void	setRoot(std::string const&);
	void	setHost(std::string const&);
	void	setPort(std::string const&);
	void	setFd(int const&);
	void	addLocation(std::string, Location const&);

	std::string const& getIndex(void) const;
	void	setIndex(std::string const&);

	int		getPermissions(void) const;
	void	setPermissions(int);
	size_t	getMaxBodySize(void) const;
	void	setMaxBodySize(size_t);
	bool	getAutoindex(void) const;
	void	setAutoindex(bool);
	void	addErrorPage(int, std::string const&);

private:
	int			fd;
	std::string	server_name;
	std::string	root;
	std::string	host;
	std::string	port;
	std::string index;
	int			permissions;
	size_t		max_body_size;
	bool		autoindex;


	std::map<std::string, Location> location_pool;
	std::map<int, std::string> error_page; // chemin abs
	//locations
		//redirect
		//default_file directory
		//post_dir
		//cgi permissions (.php/.py....)
};

#endif /* VIRTUALSERVER_H */
