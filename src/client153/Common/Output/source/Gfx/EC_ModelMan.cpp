/*
 * FILE: EC_ModelMan.cpp (ported from upstream reference)
 *
 * DESCRIPTION: CECModelMan, the shared CECModelStaticData cache in front
 * of ECM loading. The original shipped inside the prebuilt GfxCommon .lib
 * (references/client-153/Common/Output/Lib/GfxCommon_*.lib); this is a
 * port of the upstream translation unit, adapted for the browser/WASM
 * platform.
 *
 * Provenance: upstream WolfsCore/Source-Cliente-1.5.3 @
 * e3d6d0075d0a745c9f9859f2cb25b01d71dec045, file Common/GfxCommon/EC_ModelMan.cpp,
 * sha256 045b40f542d3a88e3773eb8e19428c9e8130878e4981f95df7e0b9f812b46e88.
 * Authorization: docs/LEGAL.md operator decision 2026-10-25.
 * Adaptations are marked with `//\t` comments.
 */

//	MSVC friend-name injection: the original relied on `friend class
//	A3DCombinedAction;` inside A3DGFXEx.h to declare this name at global
//	scope before A3DCombinedAction.h uses it. Clang does not inject friend
//	names, so declare it here first.
class A3DCombinedAction;

#include "StdAfx.h"
#include "Windows.h"
#include "EC_Model.h"
#include "EC_ModelMan.h"

inline AString GetRelativePath(const char* szPath)
{
	AString strRelPath;
	af_GetRelativePath(szPath, strRelPath);
	strRelPath.MakeLower();
	return strRelPath;
}

bool CECModelMan::Init()
{
	m_bScriptInit = m_ScriptEventTbl.Init("ECMActScriptTbl", false);

	if (!m_bScriptInit)
		return false;

	m_bScriptInit = m_ScriptEventTblConfigState.Init("ECMActScriptTbl");
	return m_bScriptInit;
}

CECModelStaticData* CECModelMan::GetAndRef(const char* strName)
{
	AString strRelPath = GetRelativePath(strName);
	Lock();
	CECModelStaticData* p = m_SharedModelMan.GetAndRefItem(strRelPath);
	Unlock();
	return p;
}

void CECModelMan::AddModel(const char* strName, CECModelStaticData* pModel)
{
	AString strRelPath = GetRelativePath(strName);
	Lock();
	m_SharedModelMan.AddItem(strRelPath, pModel);
	Unlock();
}

void CECModelMan::ReleaseModel(const char* strName)
{
	AString strRelPath = GetRelativePath(strName);
	Lock();
	m_SharedModelMan.RemoveItem(strRelPath);
	Unlock();
}


void CECModelMan::Release()
{
	Lock();
	m_SharedModelMan.Release();
	Unlock();

	if (m_bScriptInit)
	{
		m_ScriptEventTbl.Release();
		m_ScriptEventTblConfigState.Release();
		m_bScriptInit = false;
	}
}

CECModelStaticData* CECModelMan::LoadModelData(const char* szModelFile, bool bLoadAdditionalSkin)
{
	int nLen = strlen(szModelFile);

	if (nLen < 4 || stricmp(szModelFile + nLen - 4, ".ecm") != 0)
		return NULL;

	CECModelStaticData* pMapModel = GetAndRef(szModelFile);

	if (pMapModel)
		return pMapModel;

	pMapModel = new CECModelStaticData;

	if (!pMapModel->LoadData(szModelFile, bLoadAdditionalSkin))
	{
		delete pMapModel;
		return NULL;
	}

	Lock();
	CECModelStaticData* pMapModelOld = GetAndRef(szModelFile);

	if (pMapModelOld)
	{
		Unlock();
		delete pMapModel;
		return pMapModelOld;
	}

	AddModel(szModelFile, pMapModel);
	Unlock();
	return pMapModel;
}
