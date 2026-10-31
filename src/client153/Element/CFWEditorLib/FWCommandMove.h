/* Original Element/CFWEditorLib/FWCommandMove.h, upstream e3d6d0075d0a745c9f9859f2cb25b01d71dec045, SHA-256 af189019fe7e2cf192a633fa01d63f7c1cdf994041b3abd648e58f0e7d4aa5a6. Authorization: docs/LEGAL.md (2026-10-25). GB18030 comments transcoded to UTF-8. */
#ifndef _FWCOMMANDMOVE_H_
#define _FWCOMMANDMOVE_H_

#include "FWCommand.h"
#include <AArray.h>
#include <APoint.h>

class FWDialogBoard;
class FWGlyph;

// FWCommandMove represents the action of 
// moving a glyph
class FWCommandMove : public FWCommand  
{
public:
	typedef AArray<FWGlyph *, FWGlyph *> AGlyphArray;
	typedef AArray<APointI, APointI&> APointArray;

	FWCommandMove(const AGlyphArray& glyphArray, const APointArray& oldCenters, const APointArray& newCenters);
	virtual ~FWCommandMove();

	virtual void Execute();
	virtual void Unexecute();

private:
	AGlyphArray m_glyphArray;
	APointArray m_oldCenters;
	APointArray m_newCenters;
};

#endif 
