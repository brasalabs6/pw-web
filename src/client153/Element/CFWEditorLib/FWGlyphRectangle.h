/* Original Element/CFWEditorLib/FWGlyphRectangle.h, upstream e3d6d0075d0a745c9f9859f2cb25b01d71dec045, SHA-256 188d911f51afc67195b8995edcef602177cb80ede21435b7a2f988c17f2d96cc. Authorization: docs/LEGAL.md (2026-10-25). GB18030 comments transcoded to UTF-8. */
#ifndef _FWGLYPHRECTANGLE_H_
#define _FWGLYPHRECTANGLE_H_

#include "FWGlyphRectBase.h"
#include "FWGlyph.h"

class FWGlyphRectangle : public FWGlyphRectBase  
{
	FW_DECLARE_SERIAL(FWGlyphRectangle);
public:
	// base class interface
	virtual CCharOutline * CreateOutline() const;

	FWGlyphRectangle();
	virtual ~FWGlyphRectangle();
};

#endif 
