/*
 * FILE: ACounter.cpp
 *
 * DESCRIPTION: An counter class for A3D Engine
 *
 * CREATED BY: Hedi, 2001/12/3
 *
 * HISTORY:
 *
 * Copyright (c) 2001 Archosaur Studio, All Rights Reserved.
 *
 * Adapted from original ACounter.cpp, revision and hashes in source-manifest.json.
 * Windows timing (timeGetTime, QueryPerformanceCounter, RDTSC) maps to the
 * portable monotonic clock in platform/common/clock.h. The CPU-cycle API has no
 * portable equivalent yet; it reports zero frequency until its consumers are
 * reviewed, matching the pending ATime cycle API. The original constructor's
 * wall-clock validation loop only applies to Windows counter backends and is
 * covered by the platform clock tests here.
 */

#include "ACounter.h"
#include "ATime.h"
#include "clock.h"

std::int64_t ACounter::m_nCPUFrequency = 0;

ACounter::ACounter()
{
	m_dwFPSLastTick  = 0;
	m_vFPS = 60.0f;
	m_dwFPSCounter = 0;
}

ACounter::~ACounter()
{
}

bool ACounter::Init()
{
	m_dwFPSLastTick = a_GetTime();
	m_vFPS = 60.0f;
	m_dwFPSCounter = 0;
	return true;
}

bool ACounter::Release()
{
	return true;
}

bool ACounter::ResetFPSCounter()
{
	m_dwFPSLastTick = a_GetTime();
	m_vFPS = 60.0f;
	m_dwFPSCounter = 0;

	return true;
}

bool ACounter::UpdateFPSCounter(bool * pFPSChanged)
{
	DWORD dwTickNow = a_GetTime();

	m_dwFPSCounter ++;

	if( dwTickNow > m_dwFPSLastTick + 1000 )
	{
		m_vFPS				= m_dwFPSCounter * 1000.0f / (dwTickNow - m_dwFPSLastTick);
		m_dwFPSLastTick		= dwTickNow;
		m_dwFPSCounter		= 0;

		if( pFPSChanged )
			*pFPSChanged = true;
	}
	else
	{
		if( pFPSChanged )
			*pFPSChanged = false;
	}

	return true;
}

std::int64_t ACounter::GetMicroSecondNow()
{
	return static_cast<std::int64_t>(pw153::platform::monotonic_microseconds());
}

std::int64_t ACounter::GetCPUCycle()
{
	//	Portable CPU-cycle sampling is pending until its consumers are reviewed.
	return 0;
}

std::int64_t ACounter::GetCPUFrequency()
{
	return m_nCPUFrequency;
}

DWORD ACounter::GetMilliSecondNow()
{
	return (DWORD)GetMicroSecondNow() / 1000;
}
