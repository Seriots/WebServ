/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Cache.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lgiband <lgiband@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/11/25 15:50:31 by lgiband           #+#    #+#             */
/*   Updated: 2022/11/25 16:18:06 by lgiband          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CACHE_HPP
# define CACHE_HPP

# pragma once

# include <string>

class Cache
{
	public:
		Cache();
		~Cache();

		size_t			const& getSize() const;
		size_t			const& getUsers() const;
		std::string		const& getUri() const;
		std::ifstream*		   getStream() const;

		void			setSize(size_t const& size);
		void			setUsers(size_t const& users);
		void			setUri(std::string const& uri);
		void			setStream(std::ifstream	*stream);

	private:
		size_t			_size;
		size_t			_active_users;
		std::string		_path;
		std::ifstream	*_stream;
};

#endif