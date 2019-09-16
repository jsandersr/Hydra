//---------------------------------------------------------------
//
// Timer.cpp
//

#include "common/Timer.h"

namespace Common {

//===============================================================================

Timer::Timer()
{
}

Timer::~Timer()
{
	Stop();
}

void Timer::Pause()
{
	if (!m_isRunning)
	{
		return;
	}

	m_pauseTimeStamp = SteadyClock::now();
	m_isPaused = true;
}

void Timer::Unpause()
{
	if (!m_isPaused)
	{
		return;
	}

	using namespace std::chrono;

	m_isPaused = false;
	m_totalPauseDuration += duration_cast<microseconds>(SteadyClock::now() - m_pauseTimeStamp);
	m_pauseTimeStamp = SteadyClock::time_point{};
}

void Timer::Start()
{
	if (m_isRunning)
	{
		return;
	}

	Clear();
	m_isRunning = true;
	m_startTimeStamp = SteadyClock::now();
	m_endTimeStamp = SteadyClock::time_point{};
}

void Timer::Restart()
{
	Clear();
	Start();
}

void Timer::Stop()
{
	if (!m_isRunning)
	{
		return;
	}

	m_isRunning = false;
	m_endTimeStamp = SteadyClock::now();
}

void Timer::Clear()
{
	using namespace std::chrono;

	m_isRunning = false;
	m_isPaused = false;
	m_startTimeStamp = SteadyClock::time_point{};
	m_endTimeStamp = SteadyClock::time_point{};
	m_pauseTimeStamp = SteadyClock::time_point{};
	m_totalPauseDuration = microseconds::zero();
}

std::chrono::milliseconds Timer::GetElapsedMs() const
{
	using namespace std::chrono;
	return duration_cast<milliseconds>(GetElapsedUs());
}

std::chrono::microseconds Timer::GetElapsedUs() const
{
	// If we're currently paused we need to consider how long we've been paused for.
	if (m_isPaused)
	{
		return DoGetElapsedTime() - (m_totalPauseDuration + GetElapsedTimePausedUs());
	}

	return DoGetElapsedTime() - m_totalPauseDuration;
}

std::chrono::milliseconds Timer::GetElapsedTimePausedMs() const
{
	using namespace std::chrono;
	return duration_cast<milliseconds>(GetElapsedTimePausedUs());
}

std::chrono::microseconds Timer::GetElapsedTimePausedUs() const
{
	using namespace std::chrono;

	// Running time stops keeping track once paused. Return only up to paused point.
	if (m_isPaused)
	{
		return duration_cast<microseconds>(SteadyClock::now() - m_pauseTimeStamp);
	}

	// We're not paused, so this query doesn't make sense. Return default value (0).
	return microseconds::zero();
}

std::chrono::milliseconds Timer::GetTotalElapsedTimePausedMs() const
{
	using namespace std::chrono;
	return duration_cast<milliseconds>(GetTotalElapsedTimePausedUs());
}

std::chrono::microseconds Timer::GetTotalElapsedTimePausedUs() const
{
	// If we're currently paused, the clock is still running for this pause session.
	if (m_isPaused)
	{
		return m_totalPauseDuration + GetElapsedTimePausedUs();
	}

	return m_totalPauseDuration;
}

std::chrono::milliseconds Timer::GetTotalElapsedTimeMs() const
{
	using namespace std::chrono;
	return duration_cast<milliseconds>(GetTotalElapsedTimeUs());
}

std::chrono::microseconds Timer::GetTotalElapsedTimeUs() const
{
	return DoGetElapsedTime();
}

bool Timer::IsPaused() const
{
	return m_isPaused;
}

bool Timer::IsRunning() const
{
	return m_isRunning;
}

std::chrono::microseconds Timer::DoGetElapsedTime() const
{
	using namespace std::chrono;

	if (!m_isRunning)
	{
		// If we stopped, then we only care about the time when we were running until we stopped.
		return duration_cast<microseconds>(m_endTimeStamp - m_startTimeStamp);
	}

	// We're currently running and just want an update on how much time has elapsed.
	return duration_cast<microseconds>(SteadyClock::now() - m_startTimeStamp);
}

//===============================================================================

} // namespace Common
