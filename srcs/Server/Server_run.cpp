/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server_run.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lgiband <lgiband@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/11/23 12:53:55 by lgiband           #+#    #+#             */
/*   Updated: 2022/12/05 11:58:28 by lgiband          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <cstring>

#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <fcntl.h>
#include <unistd.h>

#include "WebServer.hpp"
#include "Setup.hpp"
#include "utils.hpp"

extern int running;
extern int flags;

/*
* Take the connection from the server socket and add it to the epoll
*/
int	WebServer::newConnection(int server_fd)
{
	int					client_socket;
	struct epoll_event	event;
	struct sockaddr_in	address;
	socklen_t			sock_len = sizeof(address);
	char buffer[INET_ADDRSTRLEN];

	if (flags & FLAG_VERBOSE)
		std::cerr << "[ New connection on server " << server_fd << " ]" << std::endl;
	client_socket = accept(server_fd, (struct sockaddr *)&address, &sock_len);
	if (client_socket == -1 && errno != EWOULDBLOCK)
		return (perror("/!\\ Accept failed"), -1);
	else
	{
		if (fcntl(client_socket, F_SETFL, O_NONBLOCK) == -1)
			return (close(client_socket), perror("/!\\ Fcntl failed"), 1);
		std::memset(&buffer, 0, INET_ADDRSTRLEN);
		inet_ntop(address.sin_family, &address.sin_addr.s_addr, buffer, INET_ADDRSTRLEN);
		if (flags & FLAG_VERBOSE)
			std::cerr << "[ Client IP: " << buffer << " ]" << std::endl;
		this->addClientIP(client_socket, buffer);
		std::memset(&event, 0, sizeof(event));
		event.data.fd = client_socket;
		event.events = EPOLLIN;
		epoll_ctl(this->_epoll_fd, EPOLL_CTL_ADD, client_socket, &event);
		this->addDuoCS(client_socket, server_fd);
		if (this->_timeout.find(client_socket) != this->_timeout.end())
			this->_timeout.erase(client_socket);
		this->_timeout.insert( std::make_pair(client_socket, (t_pair){time(NULL), 0} ) );
	}
	if (flags & FLAG_VERBOSE)
		std::cerr << "[ New client connected on " << client_socket << " ]" << std::endl;
	return (0);
}

int	WebServer::setResponse(int client_fd, Request *request)
{
	int					ret;
	Setup				setup;

	derror("[ Set response ]");
	ret = request->parsing(&setup);
	if (ret != 0)
		return (this->buildResponseDefault(client_fd, request, &setup));
	derror("[ Parsing OK ]");
	ret = request->setServer(&setup, this->getAccessibleServer(client_fd));
	if (ret != 0)
		return (this->buildResponseDefault(client_fd, request, &setup));
	derror("[ Set server OK ]");
	ret = request->setLocation(&setup);
	if (ret != 0)
		return (this->buildResponseDefault(client_fd, request, &setup));
	derror("[ Set location OK ]");
	ret = request->setUri(&setup);
	if (ret != 0)
		return (this->buildResponseDefault(client_fd, request, &setup));
	derror("[ Set uri OK ]");
	ret = request->basicCheck(&setup);
	if (ret != 0)
		return (this->buildResponseDefault(client_fd, request, &setup));
	if (flags & FLAG_VERBOSE)
		std::cerr << "[ Uri: " << setup.getUri() << " ]" << std::endl;
	derror("[ Basic check OK ]");
	ret = this->modeChoice(request, &setup, client_fd);
	if (ret != 0)
		return (this->buildResponseDefault(client_fd, request, &setup));

	return (0);
}

/*
* Receive the request from the client and and stock it in the request vector
*/
int	WebServer::getRequest(int client_fd)
{
	int		ret;
	int		state;
	Request	*request;
	std::string	body;
	
	ret = recv(client_fd, this->_buffer, BUFFER_SIZE, MSG_NOSIGNAL);
	if (ret == -1)
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK)
			return (0);
		epoll_ctl(this->_epoll_fd, EPOLL_CTL_DEL, client_fd, NULL);
		close(client_fd);
		std::cerr << "Recv fail: " << client_fd << std::endl;
		perror("/!\\ Recv failed");
	}
	else
	{
		this->_buffer[ret] = '\0';
		if (flags & FLAG_VERBOSE)
			std::cerr << "[ Recv " << ret << " bytes ]" << std::endl;
		request = this->get_fd_request(client_fd);
		if (request == NULL)
			return (derror("/!\\ Request not found"), -1);
		if (request->getChunkedMode() == 1)
		{
			if (flags & FLAG_VERBOSE)
				std::cerr << "[ Chunked mode ]" << std::endl;
			state = request->addChunkedData(this->_buffer, ret);
			if (state == 1)
			{
				if (flags & FLAG_VERBOSE)
					std::cerr << "[ All Request received on " << client_fd << " ]" << std::endl;
				this->setResponse(client_fd, request);
				this->remove_fd_request(client_fd);
			}
		}
		else
		{
			state = request->addContent(this->_buffer, ret);
			if (state == 1)
			{
				if (flags & FLAG_VERBOSE)
					std::cerr << "[ All Request received on " << client_fd << " ]" << std::endl;
				this->setResponse(client_fd, request);
				this->remove_fd_request(client_fd);
			}
			if (state == 2)
			{
				request->setChunkedMode(1);
				if (flags & FLAG_VERBOSE)
					std::cerr << "[ Chunked mode ]" << std::endl;
				
				if(request->getContent().find("\r\n\r\n") != std::string::npos)
				{
					body = request->getContent().substr(request->getContent().find("\r\n\r\n") + 4);
					request->setContent(request->getContent().substr(0, request->getContent().find("\r\n\r\n") + 4));
					if (body.size() > 0)
					{
						
						state = request->addChunkedData(body.c_str(), body.size());
						if (state == 1)
						{
							if (flags & FLAG_VERBOSE)
								std::cerr << "[ All Request received on " << client_fd << " ]" << std::endl;
							this->setResponse(client_fd, request);
							this->remove_fd_request(client_fd);
						}
					}
				}
			}
		}
		if (this->_timeout.find(client_fd) != this->_timeout.end())
		{
			this->_timeout.find(client_fd)->second.time = time(NULL);
			this->_timeout.find(client_fd)->second.state = 1;
		}	
	}
	return (0);
}

int	WebServer::sendResponse(int client_fd)
{
	Response *response;

	response = this->getResponse(client_fd);
	if (response == NULL)
	{
		this->_timeout.erase(client_fd);
		epoll_ctl(this->_epoll_fd, EPOLL_CTL_DEL, client_fd, NULL);
		close(client_fd);
		std::cout << client_fd << " disconnected" << std::endl;
		return (derror("/!\\ Response not found"), 1);
	}
	if (response->getStatus() == 0)
		this->sendHeader(client_fd, response);
	if (response->getStatus() == 1 && response->getBody() != "")
		this->sendBody(client_fd, response);
	else if (response->getStatus() == 1 && response->getFilename() != "")
		this->sendFile(client_fd, response);
	else
		this->removeResponse(client_fd);

	if (this->_timeout.find(client_fd) != this->_timeout.end())
	{
		this->_timeout.find(client_fd)->second.time = time(NULL);
		this->_timeout.find(client_fd)->second.state = 2;
	}	
	return (0);
}

int	WebServer::event_loop(struct epoll_event *events, int nb_events)
{
	int					listen_sock;

	for (int i = 0; i < nb_events; i++)
	{
		listen_sock = this->is_server(events[i].data.fd);
		if (listen_sock != -1)
			this->newConnection(listen_sock);
		else if (this->isCgi(events[i].data.fd))
			this->cgiSetResponse(events[i].data.fd);
		else if (this->isCgiClient(events[i].data.fd))
			this->cgiSendResponse(events[i].data.fd);
		else if (events[i].events & EPOLLIN)
			this->getRequest(events[i].data.fd);
		else if (events[i].events & EPOLLOUT)
			this->sendResponse(events[i].data.fd);
		else
		{
			close(events[i].data.fd);
			epoll_ctl(this->_epoll_fd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
			this->_timeout.erase(events[i].data.fd);
		}
	}
	return (0);
}

int	WebServer::run(void)
{
	int					frame = 0;
	std::string			wait[] = {"⠋", "⠙", "⠸", "⠴", "⠦", "⠇"};
	struct epoll_event	events[MAX_CLIENTS];
	int					nb_events;
	
	std::cout << "\n=====================RUN=====================\n" << std::endl;
	
	while (running)
	{
		std::memset(events, 0, sizeof(events));
		nb_events = epoll_wait(this->_epoll_fd, events, MAX_CLIENTS, TIMEOUT);
		if (nb_events == -1)
			perror("Epoll_wait failed");
		else if (nb_events == 0)
		{
			std::cout  << "\33[2K\r" << wait[++frame % 6] << "\033[0;32m" << " Waiting for Connection... " << "\033[0m" << std::flush;
			if (frame >= 6)
				frame = 0;
		}
		else
		{
			if (this->event_loop(events, nb_events) == -1 && flags & FLAG_VERBOSE)
				std::cerr << "Error in requests" << std::endl;
		}
		this->clearTimeout();	
		this->clearCache();
	}
	return (0);
}