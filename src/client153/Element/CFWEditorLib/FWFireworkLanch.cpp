/* Original Element/CFWEditorLib/FWFireworkLanch.cpp, upstream e3d6d0075d0a745c9f9859f2cb25b01d71dec045, SHA-256 9b74e7e949ddb115e72aaf15959197208fad404c2591f2ea679951169cdb726f. Authorization: docs/LEGAL.md (2026-10-25). ASCII upstream, verbatim. */
// FWFireworkLanch.cpp: implementation of the FWFireworkLanch class.
//
//////////////////////////////////////////////////////////////////////

#include "FWFireworkLanch.h"
#include "FWArchive.h"

#ifdef _FW_EDITOR
#include "FWDialogLanch.h"
#endif

#define new A_DEBUG_NEW


FW_IMPLEMENT_SERIAL(FWFireworkLanch, FWFireworkLeaf)



FWFireworkLanch::FWFireworkLanch()
{
	m_profile.Init(
		FWAssemblyCache::GetInstance()->GetDefaultProfileIDByType(
			FWAssemblyCache::TYPE_LANCH));
}

FWFireworkLanch::~FWFireworkLanch()
{

}

#ifdef _FW_EDITOR

const char * FWFireworkLanch::GetDlgToShow()
{
	return TO_STRING(FWDialogLanch);
}

#endif