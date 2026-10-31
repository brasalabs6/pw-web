/* Original Element/CFWEditorLib/GLUContour.h, upstream e3d6d0075d0a745c9f9859f2cb25b01d71dec045, SHA-256 dff645434ea964efb0e3b25c02b2e8da013f1ac157217974e9888d6dc599210a. Authorization: docs/LEGAL.md (2026-10-25). GB18030 comments transcoded to UTF-8. */
#ifndef _FWGLUCONTOUR_H_
#define _FWGLUCONTOUR_H_

class CCharContour;
class CVertex;

class CGLUContour  
{
public:
	int m_nVerticesCount;
	double (* m_paGLUVertices)[3];
public:
	bool FindInVertices(double x, double y, int nCount);
	int ComputeVerticesArraySize() const;
	int GetVerticesCount() const { return m_nVerticesCount; }
	int GetVerticesArraySize() const;
	void Destory();
	bool CreateFromCharContour(const CCharContour *pCharContour);
	CGLUContour();
	virtual ~CGLUContour();
};

#endif 