/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lgiband <lgiband@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/11/25 11:44:18 by lgiband           #+#    #+#             */
/*   Updated: 2022/12/05 12:56:23 by lgiband          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef UTILS_HPP
# define UTILS_HPP

# pragma once

# include <string>
# include <sstream>

# define BUFFER_SIZE 4096
# define REQUEST_TIMEOUT 180 //3min
# define WEBSERV_COOKIE "WebServCookie="
# define BASE_GENERATOR "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"
# define FLAG_VERBOSE	0x01
# define FLAG_TEST		0x02

template <typename T>
std::string to_string ( T Number )
{
	std::ostringstream	ss;

	ss << Number;
	return ss.str();
}

typedef struct s_flags
{
	char	character;
	int		value;
}	t_flags;


void		derror(std::string const& msg);
std::string uriDecode(const std::string &src);
std::string	reformatUri(std::string const& uri);
std::string	generateRandomCookie(int size, std::string base);

int			myToUpper(int c);

bool	doesPathExist(std::string const& path);
bool	isPathReadable(std::string const& path);
bool	isPathWriteable(std::string const& path);

bool	isDirectory(std::string const& path);
bool	isFile(std::string const& path);
#endif