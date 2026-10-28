/* Original Element/CFWEditorLib/FWCommandCreate.h, upstream e3d6d0075d0a745c9f9859f2cb25b01d71dec045, SHA-256 006607b57781045e8d14fb8a4e7843a10a3e15ededb9efdf344cbc9407de280b. Authorization: docs/LEGAL.md (2026-10-25). GB18030 comments transcoded to UTF-8. */
#ifndef _FWCOMMANDCREATE_H_
#define _FWCOMMANDCREATE_H_

#include "FWCommand.h"
#include "FWObPtrList.h"

class FWGlyph;
class FWDialogBoard;
class FWDoc;

// FWCreateCommand represents the action of 
// creating a new glyph.
// note : the class should just put/remove glyph to/from 
// docutment, but not actually new/delete glyphs, because 
// other command will contain pointers of glyphs, so 
// new/delete glyphs will make undo/redo operate incorrectly
class FWCommandCreate : public FWCommand  
{
public:
	typedef FWObPtrList<FWGlyph*> FWGlyphList;

	// pGlyph is the glyph to be added into 
	// current file.
	// rect is the size of the glyph.
	// strText is the text of the glyph
	FWCommandCreate(FWDoc *pDoc, FWGlyph* pGlyph);
	FWCommandCreate(FWDoc *pDoc, FWGlyphList& glyphList);

	virtual ~FWCommandCreate();

	virtual void Execute();

	virtual void Unexecute();

private:
	// glyphs to create
	FWGlyphList m_glyphList;

	// if the glyphs has been stored into document so
	// that the destructor should not delete them since
	// the document will do it
	bool m_bGlyphUsed;

	FWDoc * m_pDoc;
};

#endif 
