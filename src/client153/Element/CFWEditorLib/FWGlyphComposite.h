/* Original Element/CFWEditorLib/FWGlyphComposite.h, upstream e3d6d0075d0a745c9f9859f2cb25b01d71dec045, SHA-256 a2c9d8b9ef5cff39e4cb429075a7a11bf0f7efd0ba35675ad2086eaf7935bbf3. Authorization: docs/LEGAL.md (2026-10-25). GB18030 comments transcoded to UTF-8. */
#ifndef _FWGLYPHCOMPOSITE_H_
#define _FWGLYPHCOMPOSITE_H_

#include "FWGlyphGraphBase.h"
#include "FWObPtrList.h"

class FWGlyphComposite : public FWGlyphGraphBase
{
	FW_DECLARE_SERIAL(FWGlyphComposite)
protected:
	// glyphs combined in this composite
	FWObPtrList<FWGlyph *> m_lstGlyph;
	
	// used for scaling/moving
	// we have to record the relative position between
	// parent and children, and use it to compute children's
	// position after move/scale, or children can not get
	// right position after multi-move/scale due to the
	// error of rounding
	struct Ratio
	{
		float l, t, r, b;
	};
	AList2<Ratio, Ratio &> m_lstRatio;
	
	// rect that all children covered
	ARectI m_rectTrue;
	ARectI m_rectSize;
public:
	FWGlyphComposite();
	virtual ~FWGlyphComposite();
public:
	// compute m_rect and m_lstRatio
	void RecalculateRect();

	// base class interface when used as composite
	virtual ALISTPOSITION GetHeadPosition();
	virtual FWGlyph * GetHead();
	virtual ALISTPOSITION GetTailPosition();
	virtual FWGlyph * GetTail();
	virtual FWGlyph * GetNext(ALISTPOSITION &pos);
	virtual FWGlyph * GetPrev(ALISTPOSITION &pos);
	virtual FWGlyph * GetAt(ALISTPOSITION pos);
	virtual void RemoveAll();
	virtual ALISTPOSITION Find(FWGlyph *pGlyph, ALISTPOSITION startAfter = NULL);
	virtual ALISTPOSITION AddHead(FWGlyph *pGlyph);
	virtual ALISTPOSITION AddTail(FWGlyph *pGlyph);
	virtual int GetCount();
	virtual bool IsEmpty();
	virtual void DeleteAll();

	// base class interface
	virtual CCharOutline * CreateOutline() const;
	virtual void MoveHandleTo(int nHandle, APointI newPoint);
	virtual ARectI GetSizeRect() const;
	virtual void SetSizeRect(ARectI rect);
	virtual double GetAngle() const;
	virtual void SetAngle(double angle);
	virtual ARectI GetTrueRect() const;
	virtual int GetHandleCount() const;
	virtual APointI GetHandle(int nPoint) const;
	virtual bool IsHit(const APointI& point) const;
	virtual void Serialize(FWArchive &ar);
};

#endif 