/* Original Element/CFWEditorLib/FWFireworkSimple.cpp, upstream e3d6d0075d0a745c9f9859f2cb25b01d71dec045, SHA-256 b825f26307b68856852b1b332b7a96f089117aca92b45e245f7ae87e055c0fe5. Authorization: docs/LEGAL.md (2026-10-25). ASCII upstream, verbatim. */
// FWFireworkSimple.cpp: implementation of the FWFireworkSimple class.
//
//////////////////////////////////////////////////////////////////////

#include "FWFireworkSimple.h"
#include "FWAssemblyCache.h"
#include "FWConfig.h"
#include "FWArchive.h"

#ifdef _FW_EDITOR
	#include "FWDialogSimple.h"
#endif

#define new A_DEBUG_NEW



FW_IMPLEMENT_SERIAL(FWFireworkSimple, FWFireworkLeaf)



FWFireworkSimple::FWFireworkSimple()
{
	m_profile.Init(
		FWAssemblyCache::GetInstance()->GetDefaultProfileIDByType(
			FWAssemblyCache::TYPE_SIMPLE));
}

FWFireworkSimple::~FWFireworkSimple()
{

}

#ifdef _FW_EDITOR

const char * FWFireworkSimple::GetDlgToShow()
{
	return TO_STRING(FWDialogSimple);
}

#endif
