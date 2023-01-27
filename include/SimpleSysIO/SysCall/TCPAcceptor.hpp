// Copyright (c) 2022 Haofan Zheng
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#include "../Config.hpp"


#ifdef SIMPLESYSIO_ENABLE_SYSCALL_NETWORKING


#include "../StreamAcceptorBase.hpp"

#include <memory>

#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>

#include "../Exceptions.hpp"
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
	static std::unique_ptr<TCPAcceptor> Create(
		std::shared_ptr<boost::asio::io_service> ioService
	)
	{
		return std::unique_ptr<TCPAcceptor>(
			new TCPAcceptor(std::move(ioService))
		);
	}


	/**
	 * @brief Create and bind a TCP acceptor to a local endpoint
	 *
	 * @param endpoint The local endpoint to bind to
	 * @return A unique pointer to the bound acceptor
	 */
	static std::unique_ptr<TCPAcceptor> Bind(
		boost::asio::ip::tcp::endpoint endpoint,
		std::shared_ptr<boost::asio::io_service> ioService =
			std::make_shared<boost::asio::io_service>()
	)
	{
		auto acceptor = Create(std::move(ioService));
		acceptor->m_acceptor.open(endpoint.protocol());
		acceptor->m_acceptor.bind(endpoint);
		acceptor->m_acceptor.listen();
		return acceptor;
	}


	static std::unique_ptr<TCPAcceptor> Bind(
		boost::asio::ip::address_v4 ip,
		uint16_t port,
		std::shared_ptr<boost::asio::io_service> ioService =
			std::make_shared<boost::asio::io_service>()
	)
	{
		return Bind(
			boost::asio::ip::tcp::endpoint(ip, port),
			std::move(ioService)
		);
	}


	static std::unique_ptr<TCPAcceptor> Bind(
		boost::asio::ip::address_v6 ip,
		uint16_t port,
		std::shared_ptr<boost::asio::io_service> ioService =
			std::make_shared<boost::asio::io_service>()
	)
	{
		return Bind(
			boost::asio::ip::tcp::endpoint(ip, port),
			std::move(ioService)
		);
	}


	static std::unique_ptr<TCPAcceptor> BindV4(
		const std::string& ipv4,
		uint16_t port,
		std::shared_ptr<boost::asio::io_service> ioService =
			std::make_shared<boost::asio::io_service>()
	)
	{
		return Bind(
			boost::asio::ip::address_v4::from_string(ipv4),
			port,
			std::move(ioService)
		);
	}


	static std::unique_ptr<TCPAcceptor> BindV6(
		const std::string& ipv6,
		uint16_t port,
		std::shared_ptr<boost::asio::io_service> ioService =
			std::make_shared<boost::asio::io_service>()
	)
	{
		return Bind(
			boost::asio::ip::address_v6::from_string(ipv6),
			port,
			std::move(ioService)
		);
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


	virtual void AsyncAccept(AsyncAcceptCallback callback) override
	{
		m_asyncSocket = TCPSocket::Create();
		m_asyncAcceptCallback = callback;
		m_acceptor.async_accept(
			m_asyncSocket->m_socket,
			std::bind(
				&TCPAcceptor::OnAsyncAccept,
				this,
				std::placeholders::_1
			)
		);
	}

protected:


	TCPAcceptor(std::shared_ptr<boost::asio::io_service> ioService) :
		StreamAcceptorBase(),
		m_ioService(std::move(ioService)),
		m_acceptor(*m_ioService),
		m_asyncSocket(),
		m_asyncAcceptCallback()
	{}


	void OnAsyncAccept(const boost::system::error_code& error)
	{
		if (!error)
		{
			m_asyncSocket->SetDefaultOptions();
			m_asyncAcceptCallback(std::move(m_asyncSocket));
		}
		// else
		// {
		// 	throw Exception(
		// 		"Failed to accept a new connection: " + error.message()
		// 	);
		// }
	}


private:


	std::shared_ptr<boost::asio::io_service> m_ioService;
	boost::asio::ip::tcp::acceptor m_acceptor;

	std::unique_ptr<TCPSocket> m_asyncSocket;
	AsyncAcceptCallback m_asyncAcceptCallback;

}; // class TCPAcceptor


} // namespace SysCall
} // namespace SimpleSysIO

#endif // SIMPLESYSIO_ENABLE_SYSCALL_NETWORKING
