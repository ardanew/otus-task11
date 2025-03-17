#include "test_join_server.h"
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <thread>
#include "tables.h"
#include "builder.h"
#include "validator.h"
using namespace std;

TEST(TestTables, Intersect1)
{
	Tables tables;
	tables.insert("A", 0, "A_0");
	tables.insert("A", 1, "A_1");	tables.insert("B", 1, "B_1");
	tables.insert("A", 2, "A_2");	tables.insert("B", 2, "B_2");
									tables.insert("B", 3, "B_3");
	std::string res = tables.intersection();
	EXPECT_EQ(res, "1,A_1,B_1\n2,A_2,B_2\n");
}

TEST(TestTables, Intersect2)
{
	Tables tables;
	tables.insert("A", 0, "A_0");
	tables.insert("A", 1, "A_1"); 
									tables.insert("B", 2, "B_2");
									tables.insert("B", 4, "B_4");
	tables.insert("A", 6, "A_6");
	tables.insert("A", 7, "A_7"); tables.insert("B", 7, "B_7");
									tables.insert("B", 8, "B_8");
									tables.insert("B", 9, "B_9");
	std::string res = tables.intersection();
	EXPECT_EQ(res, "7,A_7,B_7\n");
}

void createTablesFromTask(Tables &tables)
{
	tables.insert("A", 0, "lean");
	tables.insert("A", 1, "sweater");
	tables.insert("A", 2, "frank");
	tables.insert("A", 3, "violation");
	tables.insert("A", 4, "quality");
	tables.insert("A", 5, "precision");

	tables.insert("B", 3, "proposal");
	tables.insert("B", 4, "example");
	tables.insert("B", 5, "lake");
	tables.insert("B", 6, "flour");
	tables.insert("B", 7, "wonder");
	tables.insert("B", 8, "selection");
}

TEST(TestTables, Intersect_FromTask)
{
	Tables tables;
	createTablesFromTask(tables);
	std::string res = tables.intersection();
	EXPECT_EQ(res, "3,violation,proposal\n4,quality,example\n5,precision,lake\n");
}

TEST(TestTables, Diff1)
{
	Tables tables;
	tables.insert("A", 0, "A_0");
	tables.insert("A", 1, "A_1");	tables.insert("B", 1, "B_1");
	tables.insert("A", 2, "A_2");	tables.insert("B", 2, "B_2");
	tables.insert("B", 3, "B_3");
	std::string res = tables.symmetricDifference();
	EXPECT_EQ(res, "0,A_0,\n3,,B_3\n");
}

TEST(TestTables, Diff2)
{
	Tables tables;
	tables.insert("A", 0, "A_0");
	tables.insert("A", 1, "A_1"); 
									tables.insert("B", 2, "B_2");
									tables.insert("B", 4, "B_4");
	tables.insert("A", 6, "A_6");
	tables.insert("A", 7, "A_7"); tables.insert("B", 7, "B_7");
									tables.insert("B", 8, "B_8");
									tables.insert("B", 9, "B_9");
	std::string res = tables.symmetricDifference();
	EXPECT_EQ(res, "0,A_0,\n1,A_1,\n2,,B_2\n4,,B_4\n6,A_6,\n8,,B_8\n9,,B_9\n");
}

TEST(TestTables, Diff_FromTask)
{
	Tables tables;
	createTablesFromTask(tables);
	std::string res = tables.symmetricDifference();
	EXPECT_EQ(res, "0,lean,\n1,sweater,\n2,frank,\n6,,flour\n7,,wonder\n8,,selection\n");
}

TEST_F(MocServer_Validate, WrongInput)
{
	Validator validator;
	ValidateResult vr;
	std::string input;
	std::vector<std::string> tokens;

	input = "WRONG INPUT\n";
	tokens = this->tokenize(input);
	vr = validator.parse(tokens);
	EXPECT_EQ(false, vr.ok);

	input = "\n";
	tokens = this->tokenize(input);
	vr = validator.parse(tokens);
	EXPECT_EQ(false, vr.ok);
}

TEST_F(MocServer_Validate, CmdTruncate)
{
	Validator validator;
	ValidateResult vr;
	std::string input;
	std::vector<std::string> tokens;

	// ok
	input = "TRUNCATE A\n";
	tokens = this->tokenize(input);
	vr = validator.parse(tokens);
	EXPECT_EQ(true, vr.ok);
	EXPECT_EQ("A", vr.parameters[0]);

	input = "TRUNCATE B\n";
	tokens = this->tokenize(input);
	vr = validator.parse(tokens);
	EXPECT_EQ(true, vr.ok);
	EXPECT_EQ("B", vr.parameters[0]);

	// no table name
	input = "TRUNCATE\n";
	tokens = this->tokenize(input);
	vr = validator.parse(tokens);
	EXPECT_EQ(false, vr.ok); 

	// it is allowed too, tail is skipped
	input = "TRUNCATE B Something\n";
	tokens = this->tokenize(input);
	vr = validator.parse(tokens);
	EXPECT_EQ(true, vr.ok);
}

TEST_F(MocServer_Validate, CmdInsert)
{
	Validator validator;
	ValidateResult vr;
	std::string input;
	std::vector<std::string> tokens;

	// ok
	input = "INSERT A 15 test\n";
	tokens = this->tokenize(input);
	vr = validator.parse(tokens);
	EXPECT_EQ(true, vr.ok);
	EXPECT_EQ("A", vr.parameters[0]);
	EXPECT_EQ("15", vr.parameters[1]);
	EXPECT_EQ("test", vr.parameters[2]);

	input = "INSERT B 16 test2\n";
	tokens = this->tokenize(input);
	vr = validator.parse(tokens);
	EXPECT_EQ(true, vr.ok);
	EXPECT_EQ("B", vr.parameters[0]);
	EXPECT_EQ("16", vr.parameters[1]);
	EXPECT_EQ("test2", vr.parameters[2]);

	// no table name
	input = "INSERT\n";
	tokens = this->tokenize(input);
	vr = validator.parse(tokens);
	EXPECT_EQ(false, vr.ok);

	// no id
	input = "INSERT A\n";
	tokens = this->tokenize(input);
	vr = validator.parse(tokens);
	EXPECT_EQ(false, vr.ok);

	// not-numeric id
	input = "INSERT A x\n";
	tokens = this->tokenize(input);
	vr = validator.parse(tokens);
	EXPECT_EQ(false, vr.ok);

	// no value
	input = "INSERT A 1\n";
	tokens = this->tokenize(input);
	vr = validator.parse(tokens);
	EXPECT_EQ(false, vr.ok);
}

TEST_F(MocServer_Validate, CmdIntersection)
{
	Validator validator;
	ValidateResult vr;
	std::string input;
	std::vector<std::string> tokens;

	// ok
	input = "INTERSECTION\n";
	tokens = this->tokenize(input);
	vr = validator.parse(tokens);
	EXPECT_EQ(true, vr.ok);
}

TEST_F(MocServer_Validate, CmdSymmetricDifference)
{
	Validator validator;
	ValidateResult vr;
	std::string input;
	std::vector<std::string> tokens;

	// ok
	input = "SYMMETRIC_DIFFERENCE\n";
	tokens = this->tokenize(input);
	vr = validator.parse(tokens);
	EXPECT_EQ(true, vr.ok);
}

class Client
{
public:
	void start()
	{
		m_client = make_shared<boost::asio::ip::tcp::socket>(m_context);

		// start read and connect before running io_context
		// to make io_context have at least one running task always
		// or it will be stopped
		startConnect();
		startRead();

		m_contextThread = std::thread([&] {
			m_context.run();
		});
	}

	void startRead()
	{
		using namespace std::placeholders;
		m_client->async_read_some(boost::asio::buffer(m_recvBuf, std::size(m_recvBuf)),
			std::bind(&Client::onRead, this, _1, _2));
	}

	void startConnect()
	{
		using namespace std::placeholders;
		boost::asio::ip::tcp::endpoint ep{ boost::asio::ip::address_v4::loopback(), 5555 };
		std::vector< boost::asio::ip::tcp::endpoint> eps{ ep };
		boost::asio::async_connect(*m_client, eps, std::bind(&Client::onConnect, this, _1));
	}

	void onConnect(boost::system::error_code ec)
	{
		if (ec)
			return;

		{
			std::lock_guard l(m_mut);
			m_connected = true;
			m_cv.notify_one();
		}
	}

	void onRead(const boost::system::error_code &error, std::size_t len)
	{
		if (!error)
		{
			std::string res{ m_recvBuf, len };
			m_receivedReplies.push_back(std::move(res));
			{
				std::lock_guard l(m_mut);
				m_received = true;
				m_cv.notify_one();
			}
		}

		startRead();
	}

	void stop()
	{
		m_stop = true;
		m_client->shutdown(boost::asio::socket_base::shutdown_both);
		m_context.stop();
		m_contextThread.join();
	}

	void send(const std::string &msg)
	{
		m_client->send(boost::asio::buffer(msg));
	}

	bool waitUntilReply()
	{
		std::unique_lock l(m_mut);
		bool waitRes = m_cv.wait_for(l, 1s, [=]() { return m_received; });
		if (!waitRes)
			return false;
		m_received = false;
		return true;
	}

	bool waitUntilConnected()
	{
		std::unique_lock l(m_mut);
		return m_cv.wait_for(l, 1s, [=]() { return m_connected; });
	}

	void clearReplies()
	{
		lock_guard l(m_mut);
		m_receivedReplies.clear();
	}

public:
	bool m_connected = false;
	std::vector<std::string> m_receivedReplies;

protected:
	std::atomic<bool> m_stop = false;
	boost::asio::io_context m_context = {};
	std::shared_ptr<boost::asio::ip::tcp::socket> m_client = {};
	std::thread m_contextThread = {};

	char m_recvBuf[1024] = {};

	std::mutex m_mut = {};
	std::condition_variable m_cv = {};
	bool m_received = false;
};

TEST(TestServer, ClientConnect)
{
	auto server = Builder::build(5555);
	server->start();

	Client client;
	client.start();
	client.waitUntilConnected();

	EXPECT_EQ(true, client.m_connected);
	
	client.stop();
	server->stop();
}

TEST(TestServer, ClientSimpleCommands)
{
	auto server = Builder::build(5555);
	server->start();
	this_thread::sleep_for(100ms);

	Client client;
	client.start();
	EXPECT_TRUE(client.waitUntilConnected());

	this_thread::sleep_for(100ms);

	client.send("TRUNCATE A\n");
	ASSERT_TRUE(client.waitUntilReply());
	EXPECT_EQ("OK\n", client.m_receivedReplies[0]);
	client.clearReplies();

	client.send("TRUNCATE B\n");
	EXPECT_TRUE(client.waitUntilReply());
	EXPECT_EQ("OK\n", client.m_receivedReplies[0]);
	client.clearReplies();

	client.send("INSERT A 1 A1\n");
	EXPECT_TRUE(client.waitUntilReply());
	EXPECT_EQ("OK\n", client.m_receivedReplies[0]);
	client.clearReplies();

	client.send("INSERT A 2 A2\n");
	EXPECT_TRUE(client.waitUntilReply());
	EXPECT_EQ("OK\n", client.m_receivedReplies[0]);
	client.clearReplies();

	client.send("INSERT B 2 B2\n");
	EXPECT_TRUE(client.waitUntilReply());
	EXPECT_EQ("OK\n", client.m_receivedReplies[0]);
	client.clearReplies();

	client.send("INSERT B 3 B3\n");
	EXPECT_TRUE(client.waitUntilReply());
	EXPECT_EQ("OK\n", client.m_receivedReplies[0]);
	client.clearReplies();

	client.send("INTERSECTION\n");
	EXPECT_TRUE(client.waitUntilReply());
	EXPECT_TRUE(client.waitUntilReply());
	EXPECT_EQ("2,A2,B2\n", client.m_receivedReplies[0]);
	EXPECT_EQ("OK\n", client.m_receivedReplies[1]);
	client.clearReplies();

	client.send("SYMMETRIC_DIFFERENCE\n");
	EXPECT_TRUE(client.waitUntilReply());
	EXPECT_TRUE(client.waitUntilReply());
	EXPECT_EQ("1,A1,\n3,,B3\n", client.m_receivedReplies[0]);
	EXPECT_EQ("OK\n", client.m_receivedReplies[1]);
	client.clearReplies();

	client.send("TRUNCATE B\n");
	EXPECT_TRUE(client.waitUntilReply());
	EXPECT_EQ("OK\n", client.m_receivedReplies[0]);
	client.clearReplies();

	client.send("SYMMETRIC_DIFFERENCE\n");
	EXPECT_TRUE(client.waitUntilReply());
	EXPECT_TRUE(client.waitUntilReply());
	EXPECT_EQ("1,A1,\n2,A2,\n", client.m_receivedReplies[0]);
	EXPECT_EQ("OK\n", client.m_receivedReplies[1]);
	client.clearReplies();

	client.stop();
	server->stop();
}