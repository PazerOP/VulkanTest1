#include "stdafx.h"
#include "GlobalValues.h"

GlobalValuesManager::GlobalValuesManager()
{
	m_Globals.m_StartTime = std::chrono::high_resolution_clock::now();

	constexpr std::chrono::nanoseconds singleFrame(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::duration<double>(1.0 / 60)));
	m_Globals.m_CurrentTime = m_Globals.m_StartTime + singleFrame;

	m_Globals.m_ElapsedTime = m_Globals.m_DT = std::chrono::duration<float>(singleFrame).count();
}

void GlobalValuesManager::Update()
{
	const auto lastFrameTime = m_Globals.m_CurrentTime;
	m_Globals.m_CurrentTime = std::chrono::high_resolution_clock::now();

	m_Globals.m_DT = std::chrono::duration<float>(m_Globals.m_CurrentTime - lastFrameTime).count();
	m_Globals.m_ElapsedTime = std::chrono::duration<float>(m_Globals.m_CurrentTime - m_Globals.m_StartTime).count();
}
