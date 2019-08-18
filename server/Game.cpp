//---------------------------------------------------------------
//
// Game.cpp
//

#include "server/Game.h"

#include "common/Log.h"
#include "server/ClientSessionEvents.h"
#include "server/GameServer.h"

namespace Server {

//===============================================================================

namespace {

std::shared_ptr<spdlog::logger> s_logger;

} // anon namespace

Game::Game()
	: m_server(std::make_unique<GameServer>())
{
	auto& events = m_server->GetEvents();
	events.GetSessionCreatedEvent().subscribe(
		[this](uint32_t clientId)
		{
			SPDLOG_LOGGER_INFO(s_logger, "OnSessionCreatedEvent");
		});

	REGISTER_LOGGER("Server::Game");
	s_logger = Log::Logger("Server::Game");
	m_server->Start();
}

Game::~Game()
{
}

void Game::Run()
{
	while (m_isRunning)
	{
		ProcessCallbackQueue();
	}
}

void Game::PostToMainThread(const std::function<void()>& cb)
{
	m_callbackQueue.Push(cb);
}

void Game::ProcessCallbackQueue()
{
	m_callbackQueue.SwapWithEmpty(m_processCbQueue);

	for (auto cb : m_processCbQueue)
	{
		if (cb)
		{
			cb();
		}
	}

	if (!m_processCbQueue.empty())
	{
		std::deque<std::function<void()>>().swap(m_processCbQueue);
	}
}

//===============================================================================

} // namespace Server
