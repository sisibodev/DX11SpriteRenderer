#pragma once

class Timer
{
public:
	void Reset();
	void Tick();
	float GetDeltaTime() const { return m_deltaTime; }
	float GetTotalTime() const;

private:
	long long m_frequency = 0;
	long long m_lastTime = 0;
	long long m_baseTime = 0;
	float m_deltaTime = 0.0f;

	static constexpr float kMaxDeltaTime = 0.1f;
};
