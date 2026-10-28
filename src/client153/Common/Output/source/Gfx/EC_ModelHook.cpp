/*
 * FILE: EC_ModelHook.cpp (ported from upstream reference)
 *
 * DESCRIPTION: CECModelHook, the named hook point (id, scale, relative
 * matrix) an ECModel uses to parent child models and gfx. The original
 * shipped inside the prebuilt GfxCommon .lib
 * (references/client-153/Common/Output/Lib/GfxCommon_*.lib); this is a
 * port of the upstream translation unit, adapted for the browser/WASM
 * platform.
 *
 * Provenance: upstream WolfsCore/Source-Cliente-1.5.3 @
 * e3d6d0075d0a745c9f9859f2cb25b01d71dec045, file Common/GfxCommon/EC_ModelHook.cpp,
 * sha256 54dad981234145b330d43d3b9ca641d839f9bd30bea3c3307eee376265e6acb4.
 * Authorization: docs/LEGAL.md operator decision 2026-10-25.
 * Adaptations are marked with `//\t` comments.
 */

//	MSVC friend-name injection: the original relied on `friend class
//	A3DCombinedAction;` inside A3DGFXEx.h to declare this name at global
//	scope before A3DCombinedAction.h uses it. Clang does not inject friend
//	names, so declare it here first.
class A3DCombinedAction;

//	Case-sensitive filesystem: the upstream file is StdAfx.h.
#include "StdAfx.h"
#include "EC_ModelHook.h"

///////////////////////////////////////////////////////////////////////////
//	
//	Local Types and Variables and Global variables
//	
///////////////////////////////////////////////////////////////////////////

static const char * s_formatECMHookName = "ECMHook: %s";
static const char * s_formatECMHookID = "ID: %d";
static const char * s_formatECMHookScale = "Scale: %f";
static const char * s_formatECMHookVec = "%f %f %f";

CECModelHook::CECModelHook()
: m_matRelative(a3d_IdentityMatrix())
{
	m_nID = 0;
	m_fScale = 1.f;
}

CECModelHook::~CECModelHook()
{
	
}

//	Save / Load ECModelHook
bool CECModelHook::Save(AFile* pFileToSave)
{
	if (!pFileToSave)
		return false;

	char szBuf[AFILE_LINEMAXLEN];
	_snprintf(szBuf, AFILE_LINEMAXLEN, s_formatECMHookName, (const char*)m_strName);
	pFileToSave->WriteLine(szBuf);

	_snprintf(szBuf, AFILE_LINEMAXLEN, s_formatECMHookID, m_nID);
	pFileToSave->WriteLine(szBuf);

	_snprintf(szBuf, AFILE_LINEMAXLEN, s_formatECMHookScale, m_fScale);
	pFileToSave->WriteLine(szBuf);

	for (int iRow = 0; iRow < 4; ++iRow)
	{
		A3DVECTOR3 vRow = m_matRelative.GetRow(iRow);
		_snprintf(szBuf, AFILE_LINEMAXLEN, s_formatECMHookVec, vRow.x, vRow.y, vRow.z);
		pFileToSave->WriteLine(szBuf);
	}
	
	return true;
}

bool CECModelHook::Load(AFile* pFileToLoad, DWORD dwVersion)
{
	if (!pFileToLoad)
		return false;

	char szBuf[AFILE_LINEMAXLEN];
	char szName[AFILE_LINEMAXLEN];
	DWORD dwReadLength;
	pFileToLoad->ReadLine(szBuf, AFILE_LINEMAXLEN, &dwReadLength);
	sscanf(szBuf, s_formatECMHookName, szName);
	m_strName = szName;

	pFileToLoad->ReadLine(szBuf, AFILE_LINEMAXLEN, &dwReadLength);
	sscanf(szBuf, s_formatECMHookID, &m_nID);

	pFileToLoad->ReadLine(szBuf, AFILE_LINEMAXLEN, &dwReadLength);
	sscanf(szBuf, s_formatECMHookScale, &m_fScale);

	for (int iRow = 0; iRow < 4; ++iRow)
	{
		A3DVECTOR3 vRow;
		pFileToLoad->ReadLine(szBuf, AFILE_LINEMAXLEN, &dwReadLength);
		sscanf(szBuf, s_formatECMHookVec, &vRow.x, &vRow.y, &vRow.z);
		m_matRelative.SetRow(iRow, vRow);
	}

	return true;
}
