#pragma once
#include <memory>
#include <vector>
#include <deque>
#include "clientdata.h"

/// \brief class to create client data
class ClientDataFactory
{
public:
	/// \brief initializes the client data with default values and client id
	std::unique_ptr<ClientData> createClientData(std::shared_ptr<boost::asio::ip::tcp::socket> socket);

	/// \brief constructs client id from socket's remote endpoint
	static std::string asClientId(const std::shared_ptr<boost::asio::ip::tcp::socket> &socket);
};