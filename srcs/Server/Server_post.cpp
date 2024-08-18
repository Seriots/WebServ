/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server_post.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lgiband <lgiband@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/11/29 11:44:06 by lgiband           #+#    #+#             */
/*   Updated: 2022/12/05 09:33:46 by lgiband          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <cstdlib>

#include <sys/socket.h>
#include <sys/epoll.h>
#include <unistd.h>

#include "WebServer.hpp"
#include "utils.hpp"

extern int flags;

int	WebServer::sendPostResponse(Request *request, Setup *setup, int client_fd)
{
	Response response;
	std::string origin;

	(void)request;
	if (setup->getCode() == 0)
	{
		setup->setCode(201);
		response.setBody("OK");
	}
	else
	{
		if (flags & FLAG_VERBOSE)
			std::cerr << setup->getCode() << std::endl;
		setup->setCode(200);
		response.setBody("KO");
	}

	response.setFd(client_fd);
	response.setStatus(0);
	response.setPosition(0);

	setup->setExtension("");

	response.setHeader(setup, this->_status_codes, this->_mimetypes, response.getBodySize());

	send(client_fd, response.getHeader().c_str(), response.getHeader().size(), MSG_NOSIGNAL | MSG_MORE);
	send(client_fd, response.getBody().c_str(), response.getBody().size(), MSG_NOSIGNAL);

	epoll_ctl(this->_epoll_fd, EPOLL_CTL_DEL, client_fd, 0);
	this->_timeout.erase(client_fd);
	close(client_fd);
	return (0);
}

int WebServer::setPostUri(Request *request, Setup *setup)
{
	std::string dir_path;
	std::string file_path;
	std::string::size_type pos;

	pos = setup->getServer()->getRoot().size();
	dir_path = setup->getUri();
	if (request->getLocation()->getPostDir() != "")
	{
		if (*(request->getLocation()->getPostDir().end() - 1) != '/')
			dir_path.replace(0, pos, request->getLocation()->getPostDir() + '/');
		else
			dir_path.replace(0, pos, request->getLocation()->getPostDir());
	}
	pos = dir_path.find_last_of('/');
	if (pos == std::string::npos || pos == dir_path.size() - 1)
		file_path = "default_file";
	else
	{
		file_path = dir_path.substr(pos + 1);
		dir_path.erase(pos + 1);
	}
	if (flags & FLAG_VERBOSE)
		std::cerr << "[ Post dir path : " << dir_path << " ]" << std::endl
			<< "[ Post file name : " << file_path << " ]" << std::endl;
	setup->setUri(dir_path);
	if (!doesPathExist(setup->getUri()))
		return (setup->setCode(404), 404);
	if (!isPathWriteable(setup->getUri()))
		return (setup->setCode(403), 403);
	setup->addUri(file_path);
	if (doesPathExist(setup->getUri()) && !isPathWriteable(setup->getUri()))
		return (setup->setCode(403), 403);
	return (0);
}

int WebServer::checkPostRequest(Request *request, Setup *setup)
{
	std::string field;
	std::string mode;

	field = request->getField("Content-Length");
	mode = request->getField("Transfer-Encoding");
	if (field == "" && mode != "chunked")
		return (setup->setCode(411), 411);
	if (flags & FLAG_VERBOSE)
		std::cerr << "[ Content-Length : " << field << " ]" << std::endl;
	if (field != "" && std::strtol(field.c_str(), NULL, 10) > (int)request->getLocation()->getMaxBodySize())
		return (setup->setCode(413), 413);
	return (0);
}

int WebServer::urlEncodedPost(Request *request, Setup *setup)
{
	std::ofstream file;
	std::string decoded;

	file.open(setup->getUri().c_str(), std::ios::out | std::ios::trunc);
	if (!file.is_open())
		return (derror("/!\\ Open Fail"), setup->setCode(500), 500);
	if (flags & FLAG_VERBOSE)
		std::cerr << "[ Post body : " << request->getBody() << " ]" << std::endl;
	request->replaceAllBody("&", "\n");
	decoded = uriDecode(request->getBody());
	if (flags & FLAG_VERBOSE)
		std::cerr << "[ Post body decoded : " << decoded << " ]" << std::endl;
	file << decoded;
	file.close();
	return (200);
}

std::vector<std::string> WebServer::splitFormdata(std::string const &file, std::string const &boundary)
{
	std::string tmp;
	std::vector<std::string> lines;
	size_t pos;

	tmp = file;
	while (tmp.size() > 2)
	{
		pos = tmp.find(boundary);
		if (pos == std::string::npos)
		{
			lines.push_back(tmp);
			break;
		}
		if (pos > 0)
			lines.push_back(tmp.substr(0, pos - 2));
		tmp.erase(0, pos + boundary.size() + 2);
	}
	return (lines);
}

std::string WebServer::parseChamp(Setup *setup, Request *request, std::string const &str)
{
	std::string header = "";
	std::string body = "";
	std::string filename = "";
	std::string name = "";
	std::string path;
	std::ofstream file;
	std::string::size_type start;
	std::string::size_type end;

	(void)request;
	start = str.find("\r\n\r\n");
	if (start == std::string::npos)
		return ("");
	header = str.substr(0, std::max(start, static_cast<std::string::size_type>(0)));
	body = str.substr(start + 4, str.size() - 2);
	start = header.find("name=\"");
	if (start != std::string::npos)
	{
		end = header.find("\"", start + 6);
		if (end != std::string::npos)
			name = header.substr(start + 6, end - start - 6);
	}

	start = header.find("filename=\"");
	if (start != std::string::npos)
	{
		end = header.find("\"", start + 10);
		if (end != std::string::npos)
			filename = header.substr(start + 10, end - start - 10);
	}
	if (filename != "")
	{
		start = setup->getUri().find_last_of("/");
		path = setup->getUri().substr(0, start + 1) + filename;
		file.open(path.c_str(), std::ios::out | std::ios::trunc);
		if (!file.is_open())
			return ("");
		file << body;
		file.close();
		return (name + "=" + filename + "\n");
	}
	else
		return (name + "=" + body + "\n");
}

int WebServer::multipartPost(Request *request, Setup *setup)
{
	std::ofstream file;
	std::string boundary;
	std::vector<std::string> champ;
	std::string line;

	boundary = request->getField("Content-Type").substr(request->getField("Content-Type").find("boundary=") + 9);
	if (flags & FLAG_VERBOSE)
		std::cerr << "[ Post boundary : " << boundary << " ]" << std::endl;
	if (flags & FLAG_VERBOSE)
		std::cerr << "[ Post body size : " << request->getBody().size() << " ]" << std::endl;
	file.open(setup->getUri().c_str(), std::ios::out | std::ios::trunc);
	if (!file.is_open())
		return (derror("/!\\ Open Fail"), setup->setCode(500), 500);
	champ = this->splitFormdata(request->getBody(), "--" + boundary);
	for (std::vector<std::string>::iterator it = champ.begin(); it != champ.end(); it++)
	{
		line = this->parseChamp(setup, request, *it);
		file << line;
	}
	file.close();
	return (0);
}

int WebServer::plainTextPost(Request *request, Setup *setup)
{
	std::ofstream file;

	(void)request;
	file.open(setup->getUri().c_str(), std::ios::out | std::ios::trunc);
	if (!file.is_open())
		return (derror("/!\\ Open Fail"), setup->setCode(500), 500);
	file << request->getBody();
	file.close();

	return (0);
}
