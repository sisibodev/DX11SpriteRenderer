#include "core/Timer.h"
#include <Windows.h>

void Timer::Reset()
{
	LARGE_INTEGER li;
	QueryPerformanceFrequency(&li);
	m_frequency = li.QuadPart;

	QueryPerformanceCounter(&li);
	m_lastTime = li.QuadPart;
	m_baseTime = li.QuadPart;

	m_deltaTime = 0.0f;
	m_rawDeltaTime = 0.0f;
}

void Timer::Tick()
{
	LARGE_INTEGER li;
	QueryPerformanceCounter(&li);

	m_rawDeltaTime = static_cast<float>(li.QuadPart - m_lastTime)
		/ static_cast<float>(m_frequency);
	m_lastTime = li.QuadPart;

	m_deltaTime = m_rawDeltaTime;
	//델타가 최대 델타보다 커지면 최대 델타값으로 잡아준다.
	if (m_deltaTime > kMaxDeltaTime)
	{
		m_deltaTime = kMaxDeltaTime;
	}
}

float Timer::GetTotalTime() const
{
	return static_cast<float>(m_lastTime - m_baseTime)
		/ static_cast<float>(m_frequency);
}
