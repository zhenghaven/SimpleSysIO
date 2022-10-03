// Copyright (c) 2022 Haofan Zheng
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#include "../Config.hpp"


#ifdef SIMPLESYSIO_ENABLE_SYSCALL_NETWORKING


#include "../StreamSocketBase.hpp"

#include <memory>

#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>


#ifndef SIMPLESYSIO_CUSTOMIZED_NAMESPACE
namespace SimpleSysIO
#else
namespace SIMPLESYSIO_CUSTOMIZED_NAMESPACE
#endif
{

namespace SysCall
{

class TCPSocket : virtual public StreamSocketBase
{
public: // static members:


	friend class TCPAcceptor;


	/**
	 * @brief create a TCP socket that is neither opened, connected to any remote
	 *        endpoint nor bound (accepted) to any local endpoint
	 *
	 * @return A unique pointer to the created socket
	 */
	static std::unique_ptr<TCPSocket> Create()
	{
		return std::unique_ptr<TCPSocket>(new TCPSocket());
	}

	/**
	 * @brief Create and connect a TCP socket to a remote endpoint
	 *
	 * @param endpoint The remote endpoint to connect to
	 * @return A unique pointer to the connected socket
	 */
	static std::unique_ptr<TCPSocket> Connect(
		boost::asio::ip::tcp::endpoint endpoint
	)
	{
		auto socket = Create();
		socket->m_socket.connect(endpoint);
		socket->SetDefaultOptions();
		return socket;
	}


	static std::unique_ptr<TCPSocket> Connect(
		boost::asio::ip::address_v4 ip,
		uint16_t port
	)
	{
		return Connect(boost::asio::ip::tcp::endpoint(ip, port));
	}


	static std::unique_ptr<TCPSocket> Connect(
		boost::asio::ip::address_v6 ip,
		uint16_t port
	)
	{
		return Connect(boost::asio::ip::tcp::endpoint(ip, port));
	}


	static std::unique_ptr<TCPSocket> ConnectV4(
		const std::string& ipv4,
		uint16_t port
	)
	{
		return Connect(boost::asio::ip::address_v4::from_string(ipv4), port);
	}


	static std::unique_ptr<TCPSocket> ConnectV6(
		const std::string& ipv6,
		uint16_t port
	)
	{
		return Connect(boost::asio::ip::address_v6::from_string(ipv6), port);
	}


public:


	virtual ~TCPSocket() = default;


	/**
	 * @brief Set default options on the opened socket
	 *        NOTE: Exception will be thrown if the socket is not opened
	 *        NOTE: this function should be called automatically
	 *              by `Connect()` and `Accept()`
	 *
	 * @exception boost::wrapexcept<boost::system::system_error> Thrown when
	 *            this function is called while this socket is not opened
	 */
	virtual void SetDefaultOptions()
	{
		m_socket.set_option(boost::asio::ip::tcp::no_delay(true));
	}


protected:


	TCPSocket() :
		StreamSocketBase(),
		m_ioService(),
		m_socket(m_ioService)
	{}


	virtual size_t SendRaw(const void* data, size_t size) override
	{
		return m_socket.send(boost::asio::buffer(data, size));
	}


	virtual size_t RecvRaw(void* data, size_t size) override
	{
		return m_socket.receive(boost::asio::buffer(data, size));
	}


private:


	boost::asio::io_service m_ioService;
	boost::asio::ip::tcp::socket m_socket;


}; // class TCPSocket

} // namespace SysCall
} // namespace SimpleSysIO

#endif // SIMPLESYSIO_ENABLE_SYSCALL_NETWORKING
