//---------------------------------------------------------------
//
// NetworkHelper.cpp
//

#include "NetworkHelper.h"

#include "common/Log.h"
#include "common/NetworkTypes.h"
#include "common/NetworkMessageParser.h"
#include "common/Log.h"

#include <SFML/Network/TcpSocket.hpp>
#include <SFML/Network/IpAddress.hpp>
#include <SFML/Network/SocketSelector.hpp>
#include <SFML/Network/TcpListener.hpp>
#include <SFML/System/Clock.hpp>

#include <utility>
#include <windows.h>

namespace Common {

//===============================================================================

namespace {
	const int s_maxSizeReceived = 512;
}

NetworkHelper::NetworkHelper()
	: m_parser(new NetworkMessageParser)
{
}

NetworkHelper::~NetworkHelper()
{
}

void NetworkHelper::SendMessages(sf::TcpSocket* socket)
{
	if (!socket)
	{
		return;
	}

	bool empty = m_messageQueue.empty();
	bool notEmpty = !empty;
	if (!m_messageQueue.empty())
	{
		const auto& message = m_messageQueue.front();

		std::vector<char> fullMessage(sizeof(message.first) + message.second.size());
		std::memcpy(fullMessage.data(), &message.first, sizeof(message.first));
		std::memcpy(fullMessage.data() + sizeof(message.first), message.second.c_str(), message.second.size());

		uint32_t sizeSent = 0;
		sf::TcpSocket::Status status = socket->send(fullMessage.data(), fullMessage.size(), sizeSent);
		if (status == sf::TcpSocket::Done)
		{
			m_messageQueue.pop_front();
		}
		else if (status == sf::TcpSocket::Partial)
		{
			// TODO: Handle this case.
		}
		else if (status == sf::TcpSocket::Disconnected)
		{
			m_messageQueue.clear();
			socket->disconnect();
		}
	}
}

void NetworkHelper::ReceiveMessages(sf::TcpSocket* socket)
{
	if (!socket)
	{
		return;
	}

	uint32_t received = 0;
	char data[s_maxSizeReceived];
	socket->receive(data, s_maxSizeReceived, received);

	std::vector<NetworkMessage> messages;
	m_parser->ExtractMessages(std::string(data, received), messages);

	if (!messages.empty())
	{
		HandleMessages(messages);
		messages.clear();
	}
}

void NetworkHelper::QueueMessage(MessageId type, const std::string& message)
{
	auto messageLength = message.size();
	// Add the data to our message header.
	MessageHeader messageHeader { type, messageLength };

	m_messageQueue.emplace_front(messageHeader, message);
}

void NetworkHelper::HandleMessages(const std::vector<NetworkMessage>& messages)
{
	// Test each message type and handle accordingly.
	NotifyObservers([&messages](NetworkObserver* observer)
	{
		observer->OnMessagesReceived(std::move(messages));
	});
}

//===============================================================================

} // namespace Common
