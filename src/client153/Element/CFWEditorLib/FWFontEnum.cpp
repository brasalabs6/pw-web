/* Original Element/CFWEditorLib/FWFontEnum.cpp, upstream e3d6d0075d0a745c9f9859f2cb25b01d71dec045, SHA-256 f41318439284b7fbbf676cac3afdf6d88660601370ec7e2f8e45a6e18c1b5b2e. Authorization: docs/LEGAL.md (2026-10-25). ASCII upstream, verbatim. */
// FontEnum.cpp: implementation of the FWFontEnum class.
//
//////////////////////////////////////////////////////////////////////

#include "FWFontEnum.h"

#define new A_DEBUG_NEW



IMPLEMENT_SINGLETON(FWFontEnum)



FWFontEnum::FWFontEnum()
{
	m_bInited = false;
}

FWFontEnum::~FWFontEnum()
{

}

bool FWFontEnum::Init(HDC hDC)
{
	if (m_bInited) return true;
	
	LOGFONT logFont;
	ZeroMemory(&logFont, sizeof(LOGFONT));
	logFont.lfFaceName[0] = '\0';

	logFont.lfCharSet = GB2312_CHARSET;
	EnumFontFamiliesEx(
		hDC,
		&logFont,
		(FONTENUMPROC)EnumFontFamExProc,
		(LPARAM)this,
		0);

	logFont.lfCharSet = ANSI_CHARSET;
	EnumFontFamiliesEx(
		hDC,
		&logFont,
		(FONTENUMPROC)EnumFontFamExProc,
		(LPARAM)this,
		0);
	return true;
}

int CALLBACK FWFontEnum::EnumFontFamExProc(
	ENUMLOGFONTEX *lpelfe,    // logical-font data
	NEWTEXTMETRICEX *lpntme,  // physical-font data
	DWORD FontType,           // type of font
	LPARAM lParam             // application-defined data
	)
{
	if (FontType != TRUETYPE_FONTTYPE)
		return 1;

	FWFontEnum *pThis = (FWFontEnum *)(lParam);

	//	AList2's ARG_T is a non-const reference (ACString &); MSVC bound the
	//	temporary, the standard form does not - named local, the list node
	//	copies it synchronously.
	ACString strName(lpelfe->elfFullName);
	if (!pThis->m_lstFonts.Find(strName))
		pThis->m_lstFonts.AddTail(strName);

	return 1;
}
