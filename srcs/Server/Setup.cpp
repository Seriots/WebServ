/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Setup.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lgiband <lgiband@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/11/23 20:13:01 by lgiband           #+#    #+#             */
/*   Updated: 2022/12/02 17:22:54 by lgiband          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>

#include "Setup.hpp"
#include "utils.hpp"

Setup::Setup(): _code(0), _uri(""), _fields(""), _server(0) {}

Setup::~Setup() {}

void	Setup::setUserSession(std::string const& cookie)
{
	if (cookie.find(WEBSERV_COOKIE) == std::string::npos)
	{
		this->addField("Set-Cookie", WEBSERV_COOKIE + generateRandomCookie(32, BASE_GENERATOR) + "; SameSite=Strict; Max-Age=3600");
		this->addField("Pragma", "Bienvenue");
	}
	else
		this->addField("Pragma", "Bon retour");
}

int	Setup::addField(std::string const& key, std::string const& value)
{
	this->_fields += key + ": " + value + "\r\n";
	return (0);
}

int	Setup::eraseUri(std::string::size_type pos)
{
	this->_uri.erase(pos);
	return (0);
}

int	Setup::replaceUri(std::string::size_type pos, std::string::size_type size, std::string const& str)
{
	this->_uri.replace(pos, size - 1, str);
	return (0);
}

/*Accesseurs*/
int		const&	Setup::getCode() const
{
	return (this->_code);
}

std::string const&	Setup::getUri() const
{
	return (this->_uri);
}

std::string const&	Setup::getQuery() const
{
	return (this->_query);
}

std::string const&	Setup::getExtension() const
{
	return (this->_extension);
}

std::string	const Setup::getExtensionName()
{
	this->setExtension();
	if (this->_extension.size() > 1 && this->_extension[0] == '.')
		return (this->_extension.substr(1));
	else
		return (this->_extension);
}

std::string const&	Setup::getFields() const
{
	return (this->_fields);
}

VirtualServer const	*Setup::getServer() const
{
	return (this->_server);
}

void	Setup::setCode(int const& code)
{
	this->_code = code;
}

void	Setup::setUri(std::string const& uri)
{
	this->_uri = uri;
	if (this->_uri[0] != '/')
		this->_uri.insert(0, "/");
}

void	Setup::addUri(std::string const& uri)
{
	if (this->_uri.size() > 0 && this->_uri[this->_uri.size() - 1] != '/' && uri[0] != '/')
		this->_uri += '/' + uri;
	else
		this->_uri += uri;
}

void	Setup::setUri(int code)
{
	if (this->_server->getErrorPage().find(code) != this->_server->getErrorPage().end())
		this->_uri = this->_server->getErrorPage().find(code)->second;
	else
		this->_uri = "";
}

void	Setup::setQuery(std::string const& query)
{
	this->_query = query;
}

void	Setup::setExtension(std::string const& extension)
{
	this->_extension = extension;
}

void	Setup::setExtension(void)
{
	std::string::size_type	pos;
	std::string::size_type	pos2;

	pos = this->_uri.find_last_of('/');
	if (pos != std::string::npos)
	{
		pos2 = this->_uri.find_last_of('.');
		if (pos != std::string::npos && pos2 > pos && pos2 != std::string::npos)
			this->_extension = this->_uri.substr(pos2);
		else
			this->_extension = "";
	}
	else
		this->_extension = "";
}

void	Setup::setFields(std::string const& fields)
{
	this->_fields = fields;
}

void	Setup::setServer(VirtualServer server)
{
	this->_server = &server;
	if (this->_server && (this->_server->getRoot().size() == 0 || *(this->_server->getRoot().end() - 1) != '/'))
		this->_server->setRoot(this->_server->getRoot() + "/");
}

void	Setup::setServer(VirtualServer *server)
{
	this->_server = server;
	if (this->_server && (this->_server->getRoot().size() == 0 || *(this->_server->getRoot().end() - 1) != '/'))
		this->_server->setRoot(this->_server->getRoot() + "/");
}
