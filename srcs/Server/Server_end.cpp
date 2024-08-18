/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server_end.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lgiband <lgiband@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/11/28 20:20:45 by lgiband           #+#    #+#             */
/*   Updated: 2022/12/02 18:25:58 by lgiband          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <fstream>

#include <unistd.h>

#include "WebServer.hpp"
#include "Cache.hpp"
#include "utils.hpp"

extern int flags;

int	WebServer::end(void)
{
	std::cout << "=====================END=====================" << std::endl;
	
	close(this->_epoll_fd);
	
	if (flags & FLAG_VERBOSE)
	{
		std::cerr << "Left in cache: " << this->_all_cache.size() << std::endl;
		std::cerr << "Left in request: " << this->_all_request.size() << std::endl;
		std::cerr << "Left in response: " << this->_all_response.size() << std::endl;
	}
	
	for (std::vector<Cache>::iterator it = this->_all_cache.begin(); it != this->_all_cache.end(); it++)
	{
		it->getStream()->close();
		delete it->getStream();
	}
	for (std::vector<Request>::iterator it = this->_all_request.begin(); it != this->_all_request.end(); it++)
		close(it->getFd());
	for (std::vector<Response>::iterator it = this->_all_response.begin(); it != this->_all_response.end(); it++)
		close(it->getFd());
	for (std::vector<VirtualServer>::iterator it = this->_virtual_servers.begin(); it != this->_virtual_servers.end(); it++)
		close(it->getFd());
	for (std::map<int, t_pair>::iterator it = this->_timeout.begin(); it != this->_timeout.end(); it++)
	{
		if (it->second.state == 0)
			close(it->first);
	}
	return (0);
}