/* Original Element/CFWEditorLib/FWRuntimeClass.cpp, upstream e3d6d0075d0a745c9f9859f2cb25b01d71dec045, SHA-256 956b5c5e5ba57e090f69ce29061feb38751c9b9b647fdefeca4bc3cfbc4e65e5. Authorization: docs/LEGAL.md (2026-10-25). GB18030 comments transcoded to UTF-8. */
// FWRuntimeClass.cpp: implementation of the FWRuntimeClass class.
//
//////////////////////////////////////////////////////////////////////

#include "FWRuntimeClass.h"
#include "FWArchive.h"


FWRuntimeClass * FWRuntimeClass::s_pFirstClass = NULL;

FWObject* FWRuntimeClass::CreateObject() const
{
	ASSERT(m_pfnCreateObject);

	return (*m_pfnCreateObject)();
}

bool FWRuntimeClass::IsDerivedFrom(const FWRuntimeClass* pBaseClass) const
{
	const FWRuntimeClass *pIter = this;
	while (pIter)
	{
		if (pIter == pBaseClass)
			return true;

		pIter = pIter->m_pBaseClass;
	}

	return false;
}

void FWRuntimeClass::Store(FWArchive& ar) const
{
	ASSERT(ar.IsStoring());

	ar.WriteStringA(m_lpszClassName);
}

FWRuntimeClass* FWRuntimeClass::Load(FWArchive& ar)
{
	ASSERT(ar.IsLoading());

	char szClassName[64];
	if (!ar.ReadStringA(szClassName, 64))
		return NULL;
	AString strClassName = szClassName;

	FWRuntimeClass *pClass = FWRuntimeClass::s_pFirstClass;
	for (; pClass != NULL; pClass = pClass->m_pNextClass)
	{
		if (strClassName == pClass->m_lpszClassName)
			return pClass;
	}
	return NULL;
}
