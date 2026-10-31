/* Original Element/CFWEditorLib/FWStream.cpp, upstream e3d6d0075d0a745c9f9859f2cb25b01d71dec045, SHA-256 2452be373d2a531547fa9edd490e7aed5fbf7e9a75394cd3475d1c36ad50cc10. Authorization: docs/LEGAL.md (2026-10-25). GB18030 comments transcoded to UTF-8. */
// FWStream.cpp: implementation of the FWStream class.
//
//////////////////////////////////////////////////////////////////////

#include "FWStream.h"
#include "CodeTemplate.h"

#define new A_DEBUG_NEW


static const int DEFAULT_VERT_COUNT = 500;
static const int DEFAULT_INDEX_COUNT = 500;



FWStream::FWStream()
{

}

FWStream::~FWStream()
{
	Release();
}

bool FWStream::QueryVertBuf(int nVertBufSize)
{
	ASSERT(nVertBufSize >= 0);

	if (nVertBufSize <= m_nVertCount) return true;
	
	SAFE_RELEASE(m_pVertexBuffer);
	
	m_nVertCount = nVertBufSize;
	if (!CreateVertexBuffer())
	{
		m_nVertCount = 0;
		m_pVertexBuffer = NULL;
		return false;
	}
	return true;
}

bool FWStream::QueryIndexBuf(int nIndexBufSize)
{
	ASSERT(nIndexBufSize >= 0);
	
	if (nIndexBufSize <= m_nIndexCount) return true;

	SAFE_RELEASE(m_pIndexBuffer);

	m_nIndexCount = nIndexBufSize;
	if (!CreateIndexBuffer())
	{
		m_nIndexCount = 0;
		m_pIndexBuffer = NULL;
		return false;
	}
	return true;
}


bool FWStream::Init(A3DDevice* pDevice)
{
	ASSERT(pDevice);

	return A3DStream::Init(
		pDevice,
		A3DVT_TLVERTEX,
		DEFAULT_VERT_COUNT,
		DEFAULT_INDEX_COUNT,					
		A3DSTRM_REFERENCEPTR,	
		A3DSTRM_REFERENCEPTR);
}
