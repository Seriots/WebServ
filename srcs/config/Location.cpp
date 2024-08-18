/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Location.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lgiband <lgiband@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/11/24 15:50:42 by lgiband           #+#    #+#             */
/*   Updated: 2022/11/30 17:23:05 by lgiband          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Location.hpp"
#include "VirtualServer.hpp"

Location::Location(): _permissions(GET_PERM | POST_PERM), _autoindex(1), _default_file(""),
					_root("/"), _index("index.html"), _post_dir("/"),
					 _redirect(""), _max_body_size(32000000)  {}

Location::Location(VirtualServer const& server):
	_permissions(server.getPermissions()),
	_autoindex(server.getAutoindex()),
	_default_file(""),
	_root(server.getRoot()),
	_index(server.getIndex()),
	_post_dir("/"),
	_redirect(""),
	_max_body_size(server.getMaxBodySize()),
	_cgi_path()
{
	return ;
}

Location::~Location() {}

Location &Location::operator=(Location const& other)
{
	_permissions = other._permissions;
	_autoindex = other._autoindex;
	_default_file = other._default_file;
	_root = other._root;
	_index = other._index;
	_post_dir = other._post_dir;
	_max_body_size = other._max_body_size;
	_redirect = other._redirect;
	_cgi_path = other._cgi_path;
	return (*this);
}

std::string	const& Location::getRoot() const
{
	return (this->_root);
}

std::string	const& Location::getIndex() const
{
	return (this->_index);
}

std::string	const& Location::getRedirect() const
{
	return (this->_redirect);
}

int	const& Location::getPermission() const
{
	return (this->_permissions);
}

std::map<std::string, std::string>	const& Location::getCgiPerm() const
{
	return (this->_cgi_path);
}

void	Location::setPermissions(int perm)
{
	this->_permissions = perm;
}

bool	const& Location::getAutoindex() const
{
	return (this->_autoindex);
}

std::string	const& Location::getDefaultFile() const
{
	return (this->_default_file);
}

std::string	const& Location::getPostDir() const
{
	return (this->_post_dir);
}

std::string::size_type	const& Location::getMaxBodySize() const
{
	return (this->_max_body_size);
}

void	Location::setRoot(std::string const& str)
{
	this->_root = str;
}

void	Location::setIndex(std::string const& str)
{
	this->_index = str;
}

void	Location::setMaxBodySize(size_t size)
{
	this->_max_body_size = size;
}

void	Location::setAutoindex(bool autoindex)
{
	this->_autoindex = autoindex;
}

void	Location::setRedirect(std::string const& redirect)
{
	this->_redirect = redirect;
}

void	Location::setDefaultFile(std::string const& file)
{
	this->_default_file = file;
}

void	Location::setPostDir(std::string const& file)
{
	this->_post_dir = file;
}

void	Location::addCGIPerm(std::string const& cgi, std::string const& path)
{
	this->_cgi_path[cgi] = path;
}
