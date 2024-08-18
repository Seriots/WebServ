/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Location.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lgiband <lgiband@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/11/24 15:46:41 by lgiband           #+#    #+#             */
/*   Updated: 2022/11/30 13:57:44 by lgiband          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOCATION_HPP
# define LOCATION_HPP

# pragma once

#include <cstring>
# include <map>
# include <string>

class VirtualServer;

# define GET_PERM 4
# define POST_PERM 2
# define DEL_PERM 1

class Location
{
public:
	Location();
	Location(VirtualServer const&);
	~Location();

	Location& operator=(Location const&);

	/*Accesseurs*/
		int									const&	getPermission() const;
		std::string							const&	getRoot() const;
		std::string							const&	getIndex() const;
		std::string							const&	getRedirect() const;
		std::map<std::string, std::string>	const&	getCgiPerm() const;
		bool								const&	getAutoindex() const;
		std::string							const&	getDefaultFile() const;
		std::string							const&	getPostDir() const;
		std::string::size_type				const&	getMaxBodySize() const;


	void	setPermissions(int);
	void	setAutoindex(bool);
	void	setRoot(std::string const&);
	void	setIndex(std::string const&);
	void	setMaxBodySize(size_t);
	void	setRedirect(std::string const&);
	void	setDefaultFile(std::string const&);
	void	setPostDir(std::string const&);
	void	addCGIPerm(std::string const&, std::string const&);


private:
		int									_permissions;
		bool								_autoindex;
		std::string							_default_file;
		std::string							_root;
		std::string							_index;
		std::string							_post_dir;
		std::string							_redirect;
		std::string::size_type				_max_body_size;
		std::map<std::string, std::string>	_cgi_path;
};

#endif
