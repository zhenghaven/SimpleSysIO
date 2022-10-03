// Copyright (c) 2022 Haofan Zheng
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#include "../Config.hpp"


#ifdef SIMPLESYSIO_ENABLE_SYSCALL_NETWORKING


#include "../StreamAcceptorBase.hpp"

#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>

#include "TCPSocket.hpp"


#ifndef SIMPLESYSIO_CUSTOMIZED_NAMESPACE
namespace SimpleSysIO
#else
namespace SIMPLESYSIO_CUSTOMIZED_NAMESPACE
#endif
{


namespace SysCall
{


class TCPAcceptor : virtual public StreamAcceptorBase
{
public: // static members:


	/**
	 * @brief Create a TCP acceptor that is neither opened nor bound to
	 *        any local endpoint
	 *
	 * @return A unique pointer to the created acceptor
	 */
	static std::unique_ptr<TCPAcceptor> Create()
	{
		return std::unique_ptr<TCPAcceptor>(new TCPAcceptor());
	}


	/**
	 * @brief Create and bind a TCP acceptor to a local endpoint
	 *
	 * @param endpoint The local endpoint to bind to
	 * @return A unique pointer to the bound acceptor
	 */
	static std::unique_ptr<TCPAcceptor> Bind(
		boost::asio::ip::tcp::endpoint endpoint
	)
	{
		auto acceptor = Create();
		acceptor->m_acceptor.open(endpoint.protocol());
		acceptor->m_acceptor.bind(endpoint);
		acceptor->m_acceptor.listen();
		return acceptor;
	}


	static std::unique_ptr<TCPAcceptor> Bind(
		boost::asio::ip::address_v4 ip,
		uint16_t port
	)
	{
		return Bind(boost::asio::ip::tcp::endpoint(ip, port));
	}


	static std::unique_ptr<TCPAcceptor> Bind(
		boost::asio::ip::address_v6 ip,
		uint16_t port
	)
	{
		return Bind(boost::asio::ip::tcp::endpoint(ip, port));
	}


	static std::unique_ptr<TCPAcceptor> BindV4(
		const std::string& ipv4,
		uint16_t port
	)
	{
		return Bind(boost::asio::ip::address_v4::from_string(ipv4), port);
	}


	static std::unique_ptr<TCPAcceptor> BindV6(
		const std::string& ipv6,
		uint16_t port
	)
	{
		return Bind(boost::asio::ip::address_v6::from_string(ipv6), port);
	}


public:


	// LCOV_EXCL_START
	virtual ~TCPAcceptor() = default;
	// LCOV_EXCL_STOP


	virtual std::unique_ptr<TCPSocket> TCPAccept()
	{
		auto socket = TCPSocket::Create();
		m_acceptor.accept(socket->m_socket);
		socket->SetDefaultOptions();
		return socket;
	}


	virtual std::unique_ptr<StreamSocketBase> Accept() override
	{
		return TCPAccept();
	}


	virtual uint16_t GetLocalPort() const
	{
		return m_acceptor.local_endpoint().port();
	}


protected:


	TCPAcceptor() :
		StreamAcceptorBase(),
		m_ioService(),
		m_acceptor(m_ioService)
	{}


private:


	boost::asio::io_service m_ioService;
	boost::asio::ip::tcp::acceptor m_acceptor;


}; // class TCPAcceptor


} // namespace SysCall
} // namespace SimpleSysIO

#endif // SIMPLESYSIO_ENABLE_SYSCALL_NETWORKING
