/* Original Element/CFWEditorLib/FWTemplate.cpp, upstream e3d6d0075d0a745c9f9859f2cb25b01d71dec045, SHA-256 9cc6d7895f9ad7c15ce9f6f0a057e4f33814aa5a4a6fbb664d901697e62a7ed5. Authorization: docs/LEGAL.md (2026-10-25). GB18030 comments transcoded to UTF-8. */
// FWTemplate.cpp: implementation of the FWTemplate class.
//
//////////////////////////////////////////////////////////////////////

#include "FWTemplate.h"
#include "FWGlyphList.h"
#include "FWGlyphText.h"
#include "FWGlyphFirework.h"
#include "FWGlyphList.h"
#include "FWFireworkLanch.h"
#include "CodeTemplate.h"
#include "FWOutterProperty.h"
#include "FWAssemblyCache.h"
#include "FWModifierCreator.h"
#include "FWArchive.h"
#include <AMemFile.h>
#include "FWLog.h"

#define new A_DEBUG_NEW


FWTemplate::FWTemplate()
{
	m_pGlyphList = new FWGlyphList;
	m_pGlyphLanch = new FWGlyphFirework;
	m_pGlyphLanch->Init(new FWFireworkLanch, _AL("Lanch"));
	m_matParentTMLanch.Identity();
	m_matParentTMMain.Identity();

	// {{ for test
//	m_matParentTM.RotateX(-A3D_PI / 2);
//	A3DMATRIX4 mat;
//	mat.Translate(0, 60, 30);
//	m_matParentTM = m_matParentTM * mat;
	// }}
}

FWTemplate::~FWTemplate()
{
	SAFE_DELETE(m_pGlyphList);
	SAFE_DELETE(m_pGlyphLanch);
}


bool FWTemplate::LoadFrom(AFile *pFile)
{
	FWArchive ar(pFile, FWArchive::MODE_LOAD);
	
	ar.ReadVesoin();

	try
	{
		Serialize(ar);
	}
	catch(...)
	{
		return false;
	}
	return true;
}

bool FWTemplate::SaveTo(AFile *pFile)
{
	FWArchive ar(pFile, FWArchive::MODE_STORE);
	
	ar.WriteVersion(FW_FILE_VERSION);

	try
	{
		Serialize(ar);
	}
	catch(...)
	{
		return false;
	}
	return true;
}

void FWTemplate::SetText(const ACHAR * szText)
{
	AString strClassName = "FWGlyphText";
	FWGlyphList *pListGlyph = m_pGlyphList;
	ALISTPOSITION pos = pListGlyph->GetHeadPosition();
	while (pos)
	{
		FWGlyph *pGlyph = pListGlyph->GetNext(pos);
		if (strClassName == pGlyph->GetRuntimeClass()->m_lpszClassName)
		{
			((FWGlyphText *) pGlyph)->SetText(szText);
			//	The original passed the AString returned by S2AS through
			//	varargs (UB); cast to the buffer pointer like the other
			//	port call sites.
			FWLog::GetInstance()->Log("FWTemplate::SetText(), text changed to \"%s\"", (const char*)S2AS(szText));
			return;
		}
	}
}

void FWTemplate::SetFontName(const ACHAR * szFontName)
{
	AString strClassName = "FWGlyphText";
	FWGlyphList *pListGlyph = m_pGlyphList;
	ALISTPOSITION pos = pListGlyph->GetHeadPosition();
	while (pos)
	{
		FWGlyph *pGlyph = pListGlyph->GetNext(pos);
		if (strClassName == pGlyph->GetRuntimeClass()->m_lpszClassName)
		{
			((FWGlyphText *) pGlyph)->SetFontName(szFontName);
			return;
		}
	}
}

void FWTemplate::SetPointSize(int nPoint)
{
	AString strClassName = "FWGlyphText";
	FWGlyphList *pListGlyph = m_pGlyphList;
	ALISTPOSITION pos = pListGlyph->GetHeadPosition();
	while (pos)
	{
		FWGlyph *pGlyph = pListGlyph->GetNext(pos);
		if (strClassName == pGlyph->GetRuntimeClass()->m_lpszClassName)
		{
			((FWGlyphText *) pGlyph)->SetPointSize(nPoint);
			return;
		}
	}
}

void FWTemplate::SetFontStyle(int nStyle)
{
	AString strClassName = "FWGlyphText";
	FWGlyphList *pListGlyph = m_pGlyphList;
	ALISTPOSITION pos = pListGlyph->GetHeadPosition();
	while (pos)
	{
		FWGlyph *pGlyph = pListGlyph->GetNext(pos);
		if (strClassName == pGlyph->GetRuntimeClass()->m_lpszClassName)
		{
			((FWGlyphText *) pGlyph)->SetFontStyle(nStyle);
			return;
		}
	}

}

void FWTemplate::Serialize(FWArchive& ar)
{
	m_pGlyphList->Serialize(ar);
	
	// since lanch do not have position on board
	// we just need to store the firework it contains
	m_pGlyphLanch->GetFirework()->Serialize(ar);

}


void FWTemplate::SetParentTM(const A3DMATRIX4& matLanch, const A3DMATRIX4& matMain)
{
	m_matParentTMLanch = matLanch;
	m_matParentTMMain = matMain;
}
	
float FWTemplate::GetBlastHeight()
{
	const FWFireworkLanch *pFirework = static_cast<const FWFireworkLanch *>(m_pGlyphLanch->GetFirework());
	const FWAssemblyProfile & profile = pFirework->GetAssemblyProfile();
	const FWOutterPropertyList *pPropList = profile.GetOutterPropertyList();
	const FWOutterProperty *pProp = pPropList->FindByName("上升距离");
	if (!pProp)
	{
		ASSERT(pProp);
		return 0;
	}
	
	return pProp->Data;
}
