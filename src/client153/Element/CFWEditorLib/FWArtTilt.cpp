/* Original Element/CFWEditorLib/FWArtTilt.cpp, upstream e3d6d0075d0a745c9f9859f2cb25b01d71dec045, SHA-256 37aee6fc3423ee72c8d8e5bd029b21b451e05f9f9a91e915f43a7ff396b0c5bc. Authorization: docs/LEGAL.md (2026-10-25). GB18030 comments transcoded to UTF-8. */
// FWArtTilt.cpp: implementation of the FWArtTilt class.
//
//////////////////////////////////////////////////////////////////////

#include "FWArtTilt.h"
#include "CharContour.h"
#include "CharOutline.h"
#include "CharVerticesBuffer.h"
#include "FWArchive.h"
#include <A3DMacros.h>
#include <AList2.h>

#define new A_DEBUG_NEW

FW_IMPLEMENT_SERIAL(FWArtTilt, FWArt)

FWArtTilt::FWArtTilt() :
	m_nAngle(30)
{
	
}

FWArtTilt::~FWArtTilt()
{

}


FWArtTilt & FWArtTilt::operator = (const FWArtTilt & src)
{
	m_nAngle = src.GetAngle();
	return *this;
}

void FWArtTilt::Serialize(FWArchive &ar)
{
	FWArt::Serialize(ar);

	if (ar.IsStoring()) 
		ar << m_nAngle;
	else 
		ar >> m_nAngle;
}

void FWArtTilt::FillParamArray(FWParamArray &arrayParam) const
{
	{ FWParam param(m_nAngle, -60, 60, _AL("倾斜角度")); arrayParam.Add(param); }
}

void FWArtTilt::UpdateFromParamArray(const FWParamArray &arrayParam)
{
	m_nAngle = arrayParam[0].GetVal();
}

void FWArtTilt::MakeArtistic(CCharOutline *pOutline) const
{
	float fRadian = m_nAngle * A3D_PI / 180.f;
	float s = float(tan(fRadian));

	FOR_EACH_POINT;

	y += s * x;

	END_FOR_EACH_POINT;
}
