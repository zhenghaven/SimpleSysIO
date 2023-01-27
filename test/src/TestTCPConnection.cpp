// Copyright (c) 2022 Haofan Zheng
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#include <gtest/gtest.h>

#include <atomic>
#include <thread>

#include <SimpleSysIO/SysCall/TCPSocket.hpp>
#include <SimpleSysIO/SysCall/TCPAcceptor.hpp>


#ifdef SIMPLESYSIO_ENABLE_SYSCALL_NETWORKING


#ifndef SIMPLESYSIO_CUSTOMIZED_NAMESPACE
using namespace SimpleSysIO;
#else
using namespace SIMPLESYSIO_CUSTOMIZED_NAMESPACE;
#endif


namespace SimpleSysIO_Test
{
	extern size_t g_numOfTestFile;
}


GTEST_TEST(TestTCPConnection, CountTestFile)
{
	static auto tmp = ++SimpleSysIO_Test::g_numOfTestFile;
	(void)tmp;
}


class TestingServer : public ::testing::Test
{
public:


	TestingServer(bool isIPv6 = false):
		m_isIPv6(isIPv6),
		m_acceptor(),
		m_testSocket(),
		m_thread()
	{
	}


	virtual ~TestingServer() = default;


	virtual void SetUp() override
	{
		if (m_isIPv6)
		{
			m_acceptor = SysCall::TCPAcceptor::BindV6("::1", 0);
		}
		else
		{
			m_acceptor = SysCall::TCPAcceptor::BindV4("127.0.0.1", 0);
		}

		m_thread.reset(new std::thread([&]()
			{
				m_testSocket = m_acceptor->Accept();
			}
		));
	}


	void AfterClientConnected()
	{
		m_thread->join();
		m_thread.reset();
	}


	bool m_isIPv6;
	std::unique_ptr<SysCall::TCPAcceptor> m_acceptor;
	std::unique_ptr<StreamSocketBase> m_testSocket;
	std::unique_ptr<std::thread> m_thread;

}; // class TestingServer


class TestingServerV4 : public TestingServer
{
public:
	TestingServerV4():
		TestingServer(false)
	{}
}; // class TestingServerV4


class TestingServerV6 : public TestingServer
{
public:
	TestingServerV6():
		TestingServer(true)
	{}
}; // class TestingServerV6


TEST_F(TestingServerV4, TestConnect)
{
	auto client = SysCall::TCPSocket::ConnectV4(
		"127.0.0.1", m_acceptor->GetLocalPort()
	);
	AfterClientConnected();
}


TEST_F(TestingServerV6, TestConnect)
{
	auto client = SysCall::TCPSocket::ConnectV6(
		"::1", m_acceptor->GetLocalPort()
	);
	AfterClientConnected();
}


static void TestSendAndReceive(StreamSocketBase& clt, StreamSocketBase& srv)
{
	// SendBytes & RecvBytes
	std::string testStr = "Hello, world!";
	clt.SendBytes(testStr);
	std::string recvStr = srv.RecvBytes<std::string>(testStr.size());
	EXPECT_EQ(testStr, recvStr);


	// SendBytes & RecvSomeBytes
	clt.SendBytes(testStr);
	std::string recvSomeStr = srv.RecvSomeBytes<std::string>(1);
	while(recvSomeStr.size() < testStr.size())
	{
		recvSomeStr += srv.RecvSomeBytes<std::string>(testStr.size());
	}
	EXPECT_EQ(testStr, recvSomeStr);


	// SizedSendBytes & SizedRecvBytes
	std::vector<uint8_t> testVec = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
	clt.SizedSendBytes(testVec);
	std::vector<uint8_t> recvVec = srv.SizedRecvBytes<std::vector<uint8_t> >();
	EXPECT_EQ(testVec, recvVec);
}


TEST_F(TestingServerV4, SendAndReceive)
{
	auto client = SysCall::TCPSocket::ConnectV4(
		"127.0.0.1", m_acceptor->GetLocalPort()
	);
	AfterClientConnected();

	TestSendAndReceive(*client, *m_testSocket);
}


TEST_F(TestingServerV6, SendAndReceive)
{
	auto client = SysCall::TCPSocket::ConnectV6(
		"::1", m_acceptor->GetLocalPort()
	);
	AfterClientConnected();

	TestSendAndReceive(*client, *m_testSocket);
}


TEST(TestTCPConnection, AsyncAccept)
{
	std::shared_ptr<boost::asio::io_service> ioService =
		std::make_shared<boost::asio::io_service>();

	auto acceptor1 = SysCall::TCPAcceptor::BindV4("127.0.0.1", 0, ioService);
	{
		acceptor1->AsyncAccept(
			[](std::unique_ptr<StreamSocketBase>)
			{
				// do nothing
			}
		);
	}

	auto acceptor = SysCall::TCPAcceptor::BindV4("127.0.0.1", 0, ioService);

	std::unique_ptr<StreamSocketBase> testSvrSocket;
	std::atomic_bool isAccepted(false);
	acceptor->AsyncAccept(
		[&](std::unique_ptr<StreamSocketBase> socket)
		{
			testSvrSocket = std::move(socket);
			isAccepted = true;
		}
	);

	std::thread ioThread([&]()
		{
			ioService->run();
		}
	);

	auto testCltSocket = SysCall::TCPSocket::ConnectV4(
		"127.0.0.1", acceptor->GetLocalPort()
	);

	// wait for connection
	while(!isAccepted)
	{}

	// stop acceptor 1
	acceptor1.reset();

	// stop io service
	ioService->stop();
	ioThread.join();

	TestSendAndReceive(*testCltSocket, *testSvrSocket);
}


#endif // SIMPLESYSIO_ENABLE_SYSCALL_NETWORKING
