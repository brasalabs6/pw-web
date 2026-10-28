/* Original Element/CFWEditorLib/FWModifier.cpp, upstream e3d6d0075d0a745c9f9859f2cb25b01d71dec045, SHA-256 4565866ecd743655d6c5a1e50b2462f0278348209ddaf3480075b5f936002d24. Authorization: docs/LEGAL.md (2026-10-25). GB18030 comments transcoded to UTF-8. */
// Modifier.cpp: implementation of the FWModifier class.
//
//////////////////////////////////////////////////////////////////////

#include "FWModifier.h"
#include "FWModifierCreator.h"

#define new A_DEBUG_NEW



FW_IMPLEMENT_DYNAMIC(FWModifier, FWObject)




FWModifier::FWModifier()
{
	m_dwElapsed = 0;
}

FWModifier::~FWModifier()
{

}

FWModifier & FWModifier::operator = (const FWModifier & src)
{
	m_vecOffset = src.GetOffset();
	return *this;
}


const char * FWModifier::GetName()
{
	FWModifierCreator *pCreator = FWModifierCreator::GetInstance();
	return pCreator->FindByID(GetID())->Name;
}

void FWModifier::Serialize(FWArchive &ar)
{
	FWObject::Serialize(ar);
}

bool FWModifier::Tick(DWORD dwTickTime)
{
	if ((m_dwElapsed += dwTickTime) < m_dwTimeWait)
		return false;

	return true;
}
