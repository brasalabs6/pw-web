/* Original Element/CFWEditorLib/FWArtWave.cpp, upstream e3d6d0075d0a745c9f9859f2cb25b01d71dec045, SHA-256 5939175dd10e641f0a3d9248bc5a2cde4a68825bfeac14450a63a89a32707b37. Authorization: docs/LEGAL.md (2026-10-25). GB18030 comments transcoded to UTF-8. */
// FWArtWave.cpp: implementation of the FWArtWave class.
//
//////////////////////////////////////////////////////////////////////

#include "FWArtWave.h"
#include "FWArchive.h"
#include "CharContour.h"
#include "CharOutline.h"
#include "CharVerticesBuffer.h"
#include "FWAlgorithm.h"
#include <AList2.h>
#include <A3DMacros.h>

#define new A_DEBUG_NEW


FW_IMPLEMENT_SERIAL(FWArtWave, FWArt)

FWArtWave::FWArtWave() :
	m_nWaveCount(2L),
	m_fWaveHeightRatio(.5f)
{

}

FWArtWave::~FWArtWave()
{

}

FWArtWave & FWArtWave::operator = (const FWArtWave & art)
{
	m_nWaveCount = art.GetWaveCount();
	m_fWaveHeightRatio = art.GetWaveHeightRatio();
	return *this;
}

void FWArtWave::MakeArtistic(CCharOutline *pOutline) const
{
	ARectI rect = FWAlgorithm::GetOutlineRect(pOutline);

	float radianPerUnit = A3D_PI * m_nWaveCount / rect.Width();
	float heightRatio = rect.Height() * m_fWaveHeightRatio * .5f;

	FOR_EACH_POINT;

	y += sin(x * radianPerUnit) * heightRatio;

	END_FOR_EACH_POINT;

}

void FWArtWave::Serialize(FWArchive &ar)
{
	FWArt::Serialize(ar);

	if (ar.IsStoring())
	{
		ar << (char)m_nWaveCount;
		ar << m_fWaveHeightRatio;
	}
	else
	{
		char tmp = 0;
		ar >> tmp;
		m_nWaveCount = tmp;
		ar >> m_fWaveHeightRatio;
	}
}

void FWArtWave::FillParamArray(FWParamArray &arrayParam) const
{
	{ FWParam param(m_nWaveCount, 1, 4, _AL("波浪个数")); arrayParam.Add(param); }
	{ FWParam param(m_fWaveHeightRatio, 0.f, 1.f, _AL("波浪高度比例")); arrayParam.Add(param); }
}

void FWArtWave::UpdateFromParamArray(const FWParamArray &arrayParam)
{
	m_nWaveCount = arrayParam[0].GetVal();
	m_fWaveHeightRatio = arrayParam[1].GetVal();
}

