#pragma once
#include <chrono>

struct GlobalValues
{
	float m_DT;
	float m_ElapsedTime;

	std::chrono::high_resolution_clock::time_point m_StartTime;
	std::chrono::high_resolution_clock::time_point m_CurrentTime;
};

class GlobalValuesManager
{
public:
	GlobalValuesManager();

	void Update();

	const GlobalValues& GetGlobals() const { return m_Globals; }

private:
	GlobalValues m_Globals;
};