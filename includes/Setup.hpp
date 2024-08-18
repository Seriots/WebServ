/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Setup.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lgiband <lgiband@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/11/23 20:06:50 by lgiband           #+#    #+#             */
/*   Updated: 2022/12/02 17:21:29 by lgiband          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SETUP_HPP
# define SETUP_HPP

# pragma once

# include <string>

# include "VirtualServer.hpp"

class Setup
{
	public:
		Setup();
		~Setup();

		/*Accesseurs*/
		int				const&	getCode() const;
		std::string		const&	getUri() const;
		std::string		const&	getQuery() const;
		std::string		const&	getExtension() const;
		std::string		const	getExtensionName() ;
		std::string		const&	getFields() const;
		VirtualServer	const	*getServer() const;
		
		void				setCode(int const& code);
		void				setUri(std::string const& uri);
		void				setUri(int code);
		void				setQuery(std::string const& query);
		void				setExtension(std::string const& extension);
		void				setExtension();
		void				setFields(std::string const& fields);
		void				setServer(VirtualServer server);
		void				setServer(VirtualServer *server);

		/*Fonctions*/
		void	setUserSession(std::string const& cookie);
		void	addUri(std::string const& uri);
		int	addField(std::string const& key, std::string const& value);
		int	eraseUri(std::string::size_type pos);
		int	replaceUri(std::string::size_type pos, std::string::size_type size, std::string const& str);

	private:
		int				_code;
		std::string		_uri;
		std::string		_query;
		std::string		_extension;
		std::string		_fields;
		VirtualServer	*_server;
};

#endif