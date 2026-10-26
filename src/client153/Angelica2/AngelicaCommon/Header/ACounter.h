/*
 * FILE: ACounter.h
 *
 * DESCRIPTION: An counter class for A3D Engine
 *
 * CREATED BY: Hedi, 2001/12/3
 *
 * HISTORY:
 *
 * Copyright (c) 2001 Archosaur Studio, All Rights Reserved.	
 */

#ifndef _ACounter_H_
#define _ACounter_H_

#include "ABaseDef.h"
//	ACPlatform.h umbrella replaced by direct includes in this port.

class ACounter
{
private:
	DWORD				m_dwFPSLastTick;
	DWORD				m_dwFPSCounter;
	FLOAT				m_vFPS;

protected:
	static std::int64_t	m_nCPUFrequency;

public:
	ACounter();
	virtual ~ACounter();

	bool Init();
	bool Release();

	static DWORD GetMilliSecondNow();
	static std::int64_t GetMicroSecondNow();
	static std::int64_t GetCPUCycle();
	static std::int64_t GetCPUFrequency();

	bool ResetFPSCounter();
	bool UpdateFPSCounter(bool * pFPSChanged = NULL);
	inline FLOAT GetFPS() { return m_vFPS; }
};

#endif