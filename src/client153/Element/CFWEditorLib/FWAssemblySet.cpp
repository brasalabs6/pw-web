/*
 * FILE: FWAssemblySet.cpp (ported from upstream reference)
 *
 * DESCRIPTION: The firework-editor assembly set (original upstream
 * translation unit; the class declaration lives in FWAssemblySet.h).
 *
 * Provenance: upstream WolfsCore/Source-Cliente-1.5.3 @
 * e3d6d0075d0a745c9f9859f2cb25b01d71dec045, file
 * Element/CFWEditorLib/FWAssemblySet.cpp,
 * sha256 67e692facf5461c1914a5a44154eb0567c8eacdac43f97bd7f4caa31551a0ea3.
 * Authorization: docs/LEGAL.md operator decision 2026-10-25. GB18030 string
 * literals transcoded to UTF-8, otherwise verbatim.
 */

// FWAssemblySet.cpp: implementation of the FWAssemblySet class.
//
//////////////////////////////////////////////////////////////////////

#include "FWAssemblySet.h"
//\tThe original resolved <A3DQuaternion.h> to the SDK copy, which includes
//\tA3DTypes.h; the port's adapted engine quaternion does not, so GfxCommonTypes.h
//\t(via A3DGFXEx.h below) saw no A3DCOLORVALUE. Include it explicitly, first.
#include <A3DTypes.h>
#include "A3DGFXEx.h"
#include "CodeTemplate.h"
#include "A3DGFXExMan.h"
#include <A3DGFXElement.h>
#include "FWAssemblySet.h"
#include "FWFireworkLanch.h"
#include "FWTemplate.h"
#include "FWGlyphFirework.h"
#include "FWOutterProperty.h"
#include "FWGlyphList.h"

#define new A_DEBUG_NEW




FWAssemblySet::FWAssemblySet(int iPoolSize) :
	FWAssemblySetBase(iPoolSize),
	m_dwTimeStart(0)
{

}

FWAssemblySet::~FWAssemblySet()
{
	DeleteAll();
}

void FWAssemblySet::DeleteAll()
{
	ALISTPOSITION pos = GetHeadPosition();
	while (pos)
	{
		FWAssembly *pAssembly = GetNext(pos);
		A3DRELEASE(pAssembly);
	}
	RemoveAll();
}

bool FWAssemblySet::TickAnimation(DWORD dwTickTime)
{
	if (GetTickCount() - m_dwTimeStart > 1000 * 30)
	{
		Stop();
		return true;
	}

	ALISTPOSITION pos = GetHeadPosition();
	while (pos)
	{
		FWAssembly *pAssembly = GetNext(pos);
		pAssembly->TickAnimation(dwTickTime);
	}
	return true;
}

bool FWAssemblySet::Start(bool bForceStop)
{
	m_dwTimeStart = GetTickCount();

	ALISTPOSITION pos = GetHeadPosition();
	while (pos)
	{
		FWAssembly *pAssembly = GetNext(pos);
		// do not start a paused gfx, if bForceStop is not set
		if (bForceStop || !bForceStop && pAssembly->GetState() == ST_PAUSE)
			pAssembly->Start(bForceStop);
	}
	return true;
}

bool FWAssemblySet::Pause()
{
	ALISTPOSITION pos = GetHeadPosition();
	while (pos)
	{
		FWAssembly *pAssembly = GetNext(pos);
		// do not force ST_STOP gfx to be the state ST_PAUSE
		if (pAssembly->GetState() == ST_PLAY)
			pAssembly->Pause();
	}
	return true;
}

void FWAssemblySet::Stop()
{
	ALISTPOSITION pos = GetHeadPosition();
	while (pos)
	{
		FWAssembly *pAssembly = GetNext(pos);
		pAssembly->Stop();
	}
}


bool FWAssemblySet::IsAllAtState(int nState) const
{
	if (GetCount() == 0)
	{
		if (nState = ST_STOP)
			return true;
		else
			return false;
	}
	
	ALISTPOSITION pos = GetHeadPosition();
	while (pos)
	{
		FWAssembly *pAssembly = GetNext(pos);
		if (pAssembly->GetState() != nState)
			return false;
	}
	return true;
}


void FWAssemblySet::RegisterGfx(A3DGFXExMan *pGfxExMan)
{
	ALISTPOSITION pos = GetHeadPosition();
	while (pos)
	{
		FWAssembly *pAssembly = GetNext(pos);
		if (pAssembly->GetState() != ST_STOP)
			pGfxExMan->RegisterGfx(pAssembly);
	}
}

bool FWAssemblySet::CreateFromTemplate(FWTemplate *pTemplate)
{
	BEGIN_FAKE_WHILE;

	DeleteAll();

	// get lanch prop
	const FWFireworkLanch *pFirework = static_cast<const FWFireworkLanch *>(pTemplate->m_pGlyphLanch->GetFirework());
	const FWAssemblyProfile & profile = pFirework->GetAssemblyProfile();
	const FWOutterPropertyList *pPropList = profile.GetOutterPropertyList();

	
	// assume the starttime and startpos of lanch gfx is 0!

	// get start time
	const FWOutterProperty *pProp = pPropList->FindByName("上升时间");
	CHECK_BREAK(pProp);
	float fStartTime = pProp->Data;
	
	// get the height that other gfx should be offset
	pProp = pPropList->FindByName("上升距离");
	CHECK_BREAK(pPropList);
	A3DMATRIX4 matOffset;
	matOffset.Translate(0.f, float(pProp->Data), 0.f);
	
	// build set
	A3DMATRIX4 matTM = matOffset * pTemplate->m_matParentTMMain;
	CHECK_BREAK(pTemplate->m_pGlyphList->FillGfxSet(this, matTM, fStartTime));
	
	// copy lanch effect only if starttime is bigger enough
	if (fStartTime > .1f)
		pTemplate->m_pGlyphLanch->GetFirework()->CopyTo(this, pTemplate->m_matParentTMLanch, 0.f);

	END_FAKE_WHILE;

	RETURN_FAKE_WHILE_RESULT;
}

