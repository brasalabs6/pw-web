/* Original Element/CFWEditorLib/FWGfxWithModifier.cpp, upstream e3d6d0075d0a745c9f9859f2cb25b01d71dec045, SHA-256 a37a6908487a8a4d56ddfc60e2be5480d64e129ffd64b3641058e60f9431fe83. Authorization: docs/LEGAL.md (2026-10-25). GB18030 comments transcoded to UTF-8. */
// FWGfxWithModifier.cpp: implementation of the FWGfxWithModifier class.
//
//////////////////////////////////////////////////////////////////////

#include "FWGfxWithModifier.h"
#include "FWModifier.h"
#include "A3DGFXElement.h"

#define new A_DEBUG_NEW




FWGfxWithModifier::FWGfxWithModifier() :
	m_pModifier(NULL)
{

}

FWGfxWithModifier::~FWGfxWithModifier()
{
	SAFE_DELETE(m_pModifier);
}

bool FWGfxWithModifier::Start(bool bForceStop)
{
	if (m_pModifier)
	{
		m_pModifier->Play(bForceStop);
	}

	return A3DGFXEx::Start(bForceStop);
}

bool FWGfxWithModifier::Stop()
{
	if (m_pModifier)
	{
		m_pModifier->Stop();
	}
	SetParentTM(m_matParentOld);

	return A3DGFXEx::Stop();
}

bool FWGfxWithModifier::TickAnimation(DWORD dwTickTime)
{
	if (m_iState != ST_PLAY) return true;

	// let modifier compute the new pos
	if (m_pModifier)
		m_pModifier->Tick(dwTickTime);

	// update gfx position
	const A3DVECTOR3 &offset = m_pModifier->GetOffset();
	A3DMATRIX4 mat;
	mat.Translate(offset.x, offset.y, offset.z);
	A3DMATRIX4 matOld = GetParentTM();
	mat = mat * matOld;
	SetParentTM(mat);

	// base tick
	A3DGFXEx::TickAnimation(dwTickTime);

	return true;
}

void FWGfxWithModifier::SetModifier(FWModifier * pSrc)
{
	SAFE_DELETE(m_pModifier);
	m_pModifier = pSrc;
}

FWGfxWithModifier & FWGfxWithModifier::operator = (const FWGfxWithModifier & src)
{
	A3DGFXEx::operator = (src);

	SetModifier(src.GetModifier()->Clone());

	return *this;
}

FWGfxWithModifier::FWGfxWithModifier(const A3DGFXEx *pGfx) :
	m_pModifier(NULL)
{
	A3DGFXEx::operator = (*pGfx);
}

const FWModifier * FWGfxWithModifier::GetModifier() const
{
	return m_pModifier;
}

void FWGfxWithModifier::SaveOriginalParentTM()
{
	m_matParentOld = GetParentTM();
}
