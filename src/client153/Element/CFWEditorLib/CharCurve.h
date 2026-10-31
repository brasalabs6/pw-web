/* Original Element/CFWEditorLib/CharCurve.h, upstream e3d6d0075d0a745c9f9859f2cb25b01d71dec045, SHA-256 c81f3e579e11c7e443c1b8d70b32f458758fb29a7f209536b1747375b41fe679. Authorization: docs/LEGAL.md (2026-10-25). GB18030 comments transcoded to UTF-8. */
#ifndef _CHARCURVE_H_
#define _CHARCURVE_H_

#include "CharVerticesBuffer.h"

struct tagTTPOLYCURVE;
typedef struct tagTTPOLYCURVE TTPOLYCURVE;

// convert windows provided char glyph info to 
// a curve in CCharVerticesBuffer format.
class CCharCurve
{
private:
	bool BuildPolyline(double *pStartPoint, TTPOLYCURVE *pCurveHeader);
	void ConvertQBToCB(double *p0, double *p1, double *p2, int nDeepth);
	bool BuildQBLine(double *pStartPoint, TTPOLYCURVE *pCurveHeader, int nDeepth);
	CCharVerticesBuffer * m_pCharVerticesBuffer;
	int m_nCurrentIndex; // used to split qb line
public:
	void Destroy();
	const CCharVerticesBuffer * GetCharVerticesBuffer() const { return m_pCharVerticesBuffer; }
	CCharVerticesBuffer * GetCharVerticesBuffer() { return m_pCharVerticesBuffer; }
	bool CreateFromFont(double *pStartPoint, TTPOLYCURVE *pCurveHeader);
	CCharCurve();
	~CCharCurve();

};

#endif
