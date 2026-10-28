/* Original Angelica2/AngelicaCommon/Source/APerlinNoiseBase.cpp, upstream e3d6d0075d0a745c9f9859f2cb25b01d71dec045, SHA-256 a6eee262e284c9ed04a20f6154f6a83046b30edcc38a47fe2cd7f4e7de87f95f. Authorization: docs/LEGAL.md (2026-10-25). Verbatim TU. */
/*
 * FILE: APerlinNoiseBase.cpp
 *
 * DESCRIPTION: class for generating 1D, 2D, and 3D PerlinNoise
 *				This class is the base class for all kinds of perlin noise
 *
 * CREATED BY: Hedi, 2002/9/13
 *
 * HISTORY:
 *
 * Copyright (c) 2001~2008 Archosaur Studio, All Rights Reserved.	
 */

#include "APerlinNoiseBase.h"

APerlinNoiseBase::APerlinNoiseBase()
{
	m_bTurbulence = true;
	m_nActiveOctave = -1;
}

APerlinNoiseBase::~APerlinNoiseBase()
{
}

DWORD APerlinNoiseBase::RandInteger()
{
	m_dwSeed = DWORD(((__int64)16807 * m_dwSeed) % (__int64)2147483647);

	return m_dwSeed;
}

float APerlinNoiseBase::RandFloat()
{
	float vValue = (int(RandInteger() % 2001) - 1000) / 1000.0f;

	return vValue;
}

bool APerlinNoiseBase::InitParams(float vAmplitude, int nWaveLength, float vPersistence, int nOctaveNum)
{
	if( nOctaveNum > MAX_OCTAVE )
		return false;

	int			i;

	m_vBaseAmplitude	= vAmplitude;
	m_nBaseWaveLength   = nWaveLength;
	m_vPersistence		= vPersistence;
	m_nOctaveNum		= nOctaveNum;

	if( m_vBaseAmplitude < 0.0f )
		m_vBaseAmplitude = -m_vBaseAmplitude;
	if( m_nBaseWaveLength < 0 )
		m_nBaseWaveLength = -m_nBaseWaveLength;
	if( m_vPersistence < 0.0f )
		m_vPersistence = -m_vPersistence;

	float	vTotalAmplitude = 0.0f;
	float	vThisAmplitude = 1.0f;
	int		nThisWaveLen = nWaveLength;
	for(i=0; i<m_nOctaveNum; i++)
	{
		vTotalAmplitude += vThisAmplitude;
		m_vAmplitude[i]		= vThisAmplitude;
		m_nWaveLength[i]	= nThisWaveLen;
		m_nStartPos[i]		= RandInteger() % 1023;

		vThisAmplitude *= vPersistence;
		nThisWaveLen /= 2;

		if( nThisWaveLen <= 0 )
		{
			m_nOctaveNum = i + 1;
			break;
		}
	}

	for(i=0; i<m_nOctaveNum; i++)
	{
		m_vAmplitude[i] = m_vAmplitude[i] / vTotalAmplitude * m_vBaseAmplitude;	
	}
	return true;
}

