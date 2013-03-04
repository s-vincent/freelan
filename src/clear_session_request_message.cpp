/*
 * libfscp - C++ portable OpenSSL cryptographic wrapper library.
 * Copyright (C) 2010-2011 Julien Kauffmann <julien.kauffmann@freelan.org>
 *
 * This file is part of libfscp.
 *
 * libfscp is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of
 * the License, or (at your option) any later version.
 *
 * libfscp is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program. If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * In addition, as a special exception, the copyright holders give
 * permission to link the code of portions of this program with the
 * OpenSSL library under certain conditions as described in each
 * individual source file, and distribute linked combinations
 * including the two.
 * You must obey the GNU General Public License in all respects
 * for all of the code used other than OpenSSL.  If you modify
 * file(s) with this exception, you may extend this exception to your
 * version of the file(s), but you are not obligated to do so.  If you
 * do not wish to do so, delete this exception statement from your
 * version.  If you delete this exception statement from all source
 * files in the program, then also delete it here.
 *
 * If you intend to use libfscp in a commercial software, please
 * contact me : we may arrange this for a small fee or no fee at all,
 * depending on the nature of your project.
 */

/**
 * \file clear_session_request_message.cpp
 * \author Julien Kauffmann <julien.kauffmann@freelan.org>
 * \brief A clear session request message class.
 */

#include "clear_session_request_message.hpp"

#include <cryptoplus/random/random.hpp>

#include <cassert>
#include <stdexcept>

namespace fscp
{
	namespace
	{
		template <typename Type>
		uint8_t to_byte(const Type& value)
		{
			return static_cast<uint8_t>(value);
		}

		template <typename Type>
		Type from_byte(uint8_t value)
		{
			return value;
		}
	}

	size_t clear_session_request_message::write(void* buf, size_t buf_len, session_number_type _session_number, const challenge_type& _challenge, const cipher_algorithm_list_type& _cipher_capabilities, const message_digest_algorithm_list_type& _message_digest_capabilities)
	{
		const size_t result_size = MIN_BODY_LENGTH + _cipher_capabilities.size() + _message_digest_capabilities.size();

		if (buf_len < result_size)
		{
			throw std::runtime_error("buf_len");
		}

		buffer_tools::set<session_number_type>(buf, 0, htonl(_session_number));
		std::copy(_challenge.begin(), _challenge.end(), static_cast<char*>(buf) + sizeof(_session_number));
		buffer_tools::set<uint16_t>(buf, sizeof(_session_number) + challenge_type::static_size, htons(static_cast<uint16_t>(_cipher_capabilities.size())));
		std::transform(_cipher_capabilities.begin(), _cipher_capabilities.end(), static_cast<uint8_t*>(buf) + sizeof(_session_number) + challenge_type::static_size + sizeof(uint16_t), &to_byte<cipher_algorithm_type>);
		buffer_tools::set<uint16_t>(buf, sizeof(_session_number) + challenge_type::static_size + sizeof(uint16_t) + _cipher_capabilities.size(), htons(static_cast<uint16_t>(_message_digest_capabilities.size())));
		std::transform(_message_digest_capabilities.begin(), _message_digest_capabilities.end(), static_cast<uint8_t*>(buf) + sizeof(_session_number) + challenge_type::static_size + sizeof(uint16_t) + _cipher_capabilities.size() + sizeof(uint16_t), &to_byte<message_digest_algorithm_type>);

		return result_size;
	}

	clear_session_request_message::clear_session_request_message(const void* buf, size_t buf_len) :
		m_data(buf)
	{
		if (buf_len < MIN_BODY_LENGTH)
		{
			throw std::runtime_error("buf_len");
		}

		if (buf_len < MIN_BODY_LENGTH + cipher_capabilities_size())
		{
			throw std::runtime_error("buf_len");
		}

		if (buf_len < MIN_BODY_LENGTH + cipher_capabilities_size() + message_digest_capabilities_size())
		{
			throw std::runtime_error("buf_len");
		}
	}

	cipher_algorithm_list_type clear_session_request_message::cipher_capabilities() const
	{
		cipher_algorithm_list_type result(cipher_capabilities_size());

		std::transform(
				data() + sizeof(session_number_type) + challenge_type::static_size + sizeof(uint16_t),
				data() + sizeof(session_number_type) + challenge_type::static_size + sizeof(uint16_t) + cipher_capabilities_size(),
				result.begin(),
				&from_byte<cipher_algorithm_type>
				);

		return result;
	}

	message_digest_algorithm_list_type clear_session_request_message::message_digest_capabilities() const
	{
		message_digest_algorithm_list_type result(message_digest_capabilities_size());

		std::transform(
				data() + sizeof(session_number_type) + challenge_type::static_size + sizeof(uint16_t) + cipher_capabilities_size() + sizeof(uint16_t),
				data() + sizeof(session_number_type) + challenge_type::static_size + sizeof(uint16_t) + cipher_capabilities_size() + sizeof(uint16_t) + message_digest_capabilities_size(),
				result.begin(),
				&from_byte<message_digest_algorithm_type>
				);

		return result;
	}
}
