/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Cache.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lgiband <lgiband@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/11/25 15:59:22 by lgiband           #+#    #+#             */
/*   Updated: 2022/11/25 16:18:20 by lgiband          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Cache.hpp"


Cache::Cache(): _size(0), _active_users(0), _path(""), _stream(NULL) {}

Cache::~Cache() {}

/*Accesseurs*/
size_t const& Cache::getSize() const
{
	return (this->_size);
}

size_t const& Cache::getUsers() const
{
	return (this->_active_users);
}

std::string const& Cache::getUri() const
{
	return (this->_path);
}

std::ifstream *Cache::getStream() const
{
	return (this->_stream);
}

void Cache::setSize(size_t const& size)
{
	this->_size = size;
}

void Cache::setUsers(size_t const& users)
{
	this->_active_users = users;
}

void Cache::setUri(std::string const& uri)
{
	this->_path = uri;
}

void Cache::setStream(std::ifstream *file)
{
	this->_stream = file;
}