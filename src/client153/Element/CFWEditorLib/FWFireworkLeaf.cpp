/* Original Element/CFWEditorLib/FWFireworkLeaf.cpp, upstream e3d6d0075d0a745c9f9859f2cb25b01d71dec045, SHA-256 6359fda8613d68689d74a6b28c810bdfb2288bdcad4c66577da3af11fd271858. Authorization: docs/LEGAL.md (2026-10-25). GB18030 comments transcoded to UTF-8. */
// SingleFirework.cpp: implementation of the FWFireworkLeaf class.
//
//////////////////////////////////////////////////////////////////////

#include "FWFireworkLeaf.h"
#include "FWAssemblyProfile.h"
#include "A3DGFXEx.h"
#include "FWAssemblySet.h"
#include "A3DParticleSystemEx.h"
#include "FWConfig.h"
#include "FWArchive.h"

#ifdef _FW_EDITOR
#include "FWDialogPropBase.h"
#endif

#define new A_DEBUG_NEW


FW_IMPLEMENT_SERIAL(FWFireworkLeaf, FWFirework)



FWFireworkLeaf::FWFireworkLeaf()
{
	
}

FWFireworkLeaf::~FWFireworkLeaf()
{

}

FWAssembly * FWFireworkLeaf::LoadAndInitAssembly()
{
	return m_profile.CreateAssembly();
}

bool FWFireworkLeaf::DoCopy(FWAssemblySet *pSet, FWAssembly *pAssembly, const A3DMATRIX4& matTM, float fStartTime)
{
	if (!UpdateAssemblyStartParam(pAssembly, matTM, fStartTime))
		return false;

	pSet->AddTail(pAssembly);

	return true;
}


bool FWFireworkLeaf::CopyTo(FWAssemblySet *pSet, const A3DMATRIX4& matTM, float fStartTime)
{
	FWAssembly * pAssembly = NULL;

	BEGIN_FAKE_WHILE;
	// load from file
	pAssembly = LoadAndInitAssembly();
	CHECK_BREAK(pAssembly);
	
	// copy to destination
	CHECK_BREAK(DoCopy(pSet, pAssembly, matTM, fStartTime));

	END_FAKE_WHILE;

	BEGIN_ON_FAIL_FAKE_WHILE;

	A3DRELEASE(pAssembly);

	END_ON_FAIL_FAKE_WHILE;

	RETURN_FAKE_WHILE_RESULT;
}

void FWFireworkLeaf::Serialize(FWArchive &ar)
{
	FWFirework::Serialize(ar);

	m_profile.Serialize(ar);
}

int FWFireworkLeaf::GetParticalCount()
{
	return m_profile.GetParticalCount();
}

int FWFireworkLeaf::GetSoundCount()
{
	return m_profile.GetSoundCount();
}

#ifdef _FW_EDITOR

void FWFireworkLeaf::PrepareDlg(FWDialogPropBase *pDlg)
{
	pDlg->m_profile = m_profile;
}


void FWFireworkLeaf::UpdateFromDlg(FWDialogPropBase * pDlg)
{
	m_profile = pDlg->m_profile;
}

#endif