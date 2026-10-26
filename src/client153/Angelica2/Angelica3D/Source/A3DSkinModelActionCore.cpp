/*
 * FILE: A3DSkinModelAct.cpp
 *
 * DESCRIPTION: A3DSkinModelActionCore extracted from A3DSkinModelAct.cpp.
 * Other action/model methods remain to be ported.
 *
 * CREATED BY: Duyuxin, 2005/6/15
 *
 * HISTORY: 
 *
 * Copyright (c) 2005 Archosaur Studio, All Rights Reserved.
 */

#include "A3DSkinModelAct.h"
#include "A3DPI.h"
#include "A3DSkeleton.h"
#include "A3DJoint.h"
#include "A3DTrackMan.h"
#include "A3DBone.h"
#include "A3DFuncs.h"
#include "AAssist.h"
#include "AMemory.h"
#include "AFile.h"
#include "AFI.h"
#include "ATime.h"

///////////////////////////////////////////////////////////////////////////
//	
//	Define and Macro
//	
///////////////////////////////////////////////////////////////////////////

#define new A_DEBUG_NEW

///////////////////////////////////////////////////////////////////////////
//	
//	Reference to External variables and functions
//	
///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////
//	
//	Local Types and Variables and Global variables
//	
///////////////////////////////////////////////////////////////////////////

//	Action data in file. when SKMDFILE_VERSION < 6
struct FILEACTIONDATA
{
	int			iActGroup;		//	Action group ID
	int			iNumJoint;		//	Number of joint
	DWORD		dwFlags;		//	Action flags
};

//	Action data in file. when SKMDFILE_VERSION >= 6 && < 7
struct FILEACTIONDATA6
{
	float		fStartFrame;	//	Start frame
	float		fEndFrame;		//	End frame
};

//	Joint action data in file
struct FILEJOINTACTION
{
	int			iJoint;			//	Joint's index
	float		fStartFrame;	//	Start frame
	float		fEndFrame;		//	End frame
	int			iNumLoop;		//	Loop number, -1 means loop forever
	float		fSpeedFactor;	//	Speed factor
};

///////////////////////////////////////////////////////////////////////////
//	
//	Local functions
//	
///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////
//
//	Implement A3DSkinModelActionCore
//
///////////////////////////////////////////////////////////////////////////

A3DSkinModelActionCore::A3DSkinModelActionCore(int iFrameRate)
{
	m_dwClassID		= A3D_CID_SKINMODELACTIONCORE;
	m_fStartFrame	= 0.0f;
	m_fEndFrame		= 0.0f;
	m_dwStartTime	= 0;
	m_dwEndTime		= 0;
	m_dwNameID		= 0;
	m_iFrameRate	= iFrameRate;
	m_dwActTime		= 0;
	m_dwTckNameID	= 0;
	m_pTrackSet		= NULL;
	m_bOwnTckFile	= false;
	m_iRefCnt		= 0;
	m_dwRefTime		= 0;
	
	memset(m_aRanks, 0, sizeof (m_aRanks));
}

A3DSkinModelActionCore::~A3DSkinModelActionCore()
{
	//	Release track set
	if (m_pTrackSet && m_dwTckNameID && m_bOwnTckFile)
	{
		A3D::g_A3DTrackMan.ReleaseSkeletonTrackSet(m_dwTckNameID, m_strTrackName);
		m_pTrackSet = NULL;
	}

	if (m_iRefCnt)
	{
		ASSERT(!m_iRefCnt);
	}
}

//	Set name
void A3DSkinModelActionCore::SetName(const char* szName)
{
	ASSERT(szName);
	m_strName	= szName;
	m_dwNameID	= a_MakeIDFromLowString(szName);
}

//	Set start and end frame
void A3DSkinModelActionCore::SetStartAndEndFrame(float fStartFrame, float fEndFrame)
{
	m_fStartFrame	= fStartFrame;
	m_fEndFrame		= fEndFrame;
	m_dwStartTime	= FrameToTime(fStartFrame, m_iFrameRate);
	m_dwEndTime		= FrameToTime(fEndFrame, m_iFrameRate);
	m_dwActTime		= m_dwEndTime - m_dwStartTime + 1;
}

//	Load model from file
bool A3DSkinModelActionCore::Load(AFile* pFile, DWORD dwVer, const char* szTckDir)
{
	//	Read action name
	pFile->ReadString(m_strName);
	m_dwNameID = a_MakeIDFromLowString(m_strName);

	if (dwVer < 6)
	{
		//	Read action data
		FILEACTIONDATA ad;
		DWORD dwRead;

		if (!pFile->Read(&ad, sizeof (ad), &dwRead) || dwRead != sizeof (ad))
		{
			g_A3DErrLog.Log("A3DSkinModelActionCore::Load, Failed to read action data");
			return false;
		}

		//	Read joint action data
		if (ad.iNumJoint >= 1)
		{
			//	Read the first joint-act data and use it's data as action's data
			FILEJOINTACTION Src;
			if (!pFile->Read(&Src, sizeof (Src), &dwRead) || dwRead != sizeof (Src))
			{
				g_A3DErrLog.Log("A3DSkinModelActionCore::Load, Failed to load joint data");
				return false;
			}

			SetStartAndEndFrame(Src.fStartFrame, Src.fEndFrame);

			//	Skip left joint action data
			if (ad.iNumJoint > 1)
			{
				pFile->Seek((ad.iNumJoint - 1) * sizeof (FILEJOINTACTION), AFILE_SEEK_CUR);
			}
		}
		else
		{
			//	This is a very special case
			SetStartAndEndFrame(0.0f, 0.0f);
		}
	}
	else	//	dwVer >= 6
	{
		//	Read action data
		FILEACTIONDATA6 ad;
		DWORD dwRead;

		if (!pFile->Read(&ad, sizeof (ad), &dwRead) || dwRead != sizeof (ad))
		{
			g_A3DErrLog.Log("A3DSkinModelActionCore::Load, Failed to read action data");
			return false;
		}

		SetStartAndEndFrame(ad.fStartFrame, ad.fEndFrame);
	}

	//	Load track set file name ...
	if (dwVer >= 7)
	{
		//	Read track file relative to skin model file
		AString str;
		pFile->ReadString(str);

		if (szTckDir[0])
		{
			str = "\\" + str;
			str = szTckDir + str;
		}
		
		SetTrackSetFileName(str, true);
	}
	else
		m_bOwnTckFile = false;

	return true;
}

//	Save model from file
bool A3DSkinModelActionCore::Save(AFile* pFile)
{
	//	Write action name
	pFile->WriteString(m_strName);

	FILEACTIONDATA6 ad;
	memset(&ad, 0, sizeof (ad));

	ad.fStartFrame	= m_fStartFrame;
	ad.fEndFrame	= m_fEndFrame;

	DWORD dwWrite;
	if (!pFile->Write(&ad, sizeof (ad), &dwWrite) || dwWrite != sizeof (ad))
	{
		g_A3DErrLog.Log("A3DSkinModelActionCore::Save, Failed to save action data");
		return false;
	}

	if (af_CheckFileExt(m_strTrackName, ".stck"))
	{
		//	Save track set file name (not including path) !
		AString strTitle;
		af_GetFileTitle(m_strTrackName, strTitle);
		pFile->WriteString(strTitle);
	}
	else
	{
		//	This shouldn't happen when SKMDFILE_VERSION >= 7
		ASSERT(0);
		pFile->WriteString("");
	}

	return true;
}

//	Skip action data in file
bool A3DSkinModelActionCore::SkipFileData(AFile* pFile, DWORD dwVer)
{
	AString str;

	//	Skip action name
	pFile->ReadString(str);

	if (dwVer < 6)
	{
		//	Skip action data
		FILEACTIONDATA ad;
		DWORD dwRead;

		if (!pFile->Read(&ad, sizeof (ad), &dwRead) || dwRead != sizeof (ad))
		{
			g_A3DErrLog.Log("A3DSkinModelActionCore::Load, Failed to read action data");
			return false;
		}

		//	Skip joint action data
		pFile->Seek(ad.iNumJoint * sizeof (FILEJOINTACTION), AFILE_SEEK_CUR);
	}
	else	//	dwVer >= 6
	{
		//	Skip action data
		FILEACTIONDATA6 ad;
		DWORD dwRead;

		if (!pFile->Read(&ad, sizeof (ad), &dwRead) || dwRead != sizeof (ad))
		{
			g_A3DErrLog.Log("A3DSkinModelActionCore::Load, Failed to read action data");
			return false;
		}
		
		if (dwVer >= 7)
		{
			//	Skip track set file name ...
			pFile->ReadString(str);
		}
	}

	return true;
}

//	Convert frame to time
DWORD A3DSkinModelActionCore::FrameToTime(float fFrame, int iFrameRate)
{
	if (iFrameRate <= 0)
		return 0;
	else
		return (DWORD)(fFrame * 1000.0f / iFrameRate);
}

//	Set track set name
void A3DSkinModelActionCore::SetTrackSetFileName(const char* szFileName, bool bOwnTckFile)
{
	m_strTrackName	= szFileName;
	m_dwTckNameID	= a_MakeIDFromLowString(szFileName);
	m_bOwnTckFile	= bOwnTckFile;
}

//	Try to bind track set
bool A3DSkinModelActionCore::BindTrackSet()
{
	m_dwRefTime = a_GetTime();

	if (!m_dwTckNameID)
		return false;

	if (!m_bOwnTckFile)
	{
		//	Track set is created when skeleton loaded and is released when
		//	skeleton released, so use GetSkeletonTrackSet() to avoid
		//	changing track set's reference counter
		m_pTrackSet = A3D::g_A3DTrackMan.GetSkeletonTrackSet(m_dwTckNameID, m_strTrackName);
	}
	else
	{
		//	Below 'if (m_pTrackSet)' statement is necessary, because CreateSkeletonTrackSet()
		//	will increase track set's reference counter.
		if (m_pTrackSet)
			return true;

		bool bNewTrackSet;
		m_pTrackSet = A3D::g_A3DTrackMan.CreateSkeletonTrackSet(m_dwTckNameID, 
								m_strTrackName, false, bNewTrackSet);
	}
	
	if (!m_pTrackSet)
	{
	//	ASSERT(0);
		return false;
	}

	return true;
}

//	Unbind track set
void A3DSkinModelActionCore::UnbindTrackSet()
{
	if (!m_bOwnTckFile)
	{
		ASSERT(m_bOwnTckFile);
		return;
	}

	if (m_pTrackSet)
	{
		A3D::g_A3DTrackMan.ReleaseSkeletonTrackSet(m_dwTckNameID, m_strTrackName);
		m_pTrackSet = NULL;
	}
}

//	Get position and orientation of specified time
bool A3DSkinModelActionCore::GetBoneStateOnTrack(int idJoint, int iAbsTime, 
					A3DVECTOR3& vPos, A3DQUATERNION& quRot)
{
	if (!m_pTrackSet)
	{
		ASSERT(m_pTrackSet);
		return false;
	}
	
	A3DSklTrackSet::BONETRACK* pBoneTrack = m_pTrackSet->GetBoneTrack(idJoint);
	if (!pBoneTrack)
	{
		ASSERT(pBoneTrack);
		return false;
	}

	vPos = pBoneTrack->pPosTrack->GetKeyValue(iAbsTime);
	quRot = pBoneTrack->pRotTrack->GetKeyValue(iAbsTime);
	quRot.Normalize();

	return true;
}

bool A3DSkinModelActionCore::GetBoneStateOnAbsTrack(int idJoint, int iAbsTime, 
					A3DVECTOR3& vPos, A3DQUATERNION& quRot)
{
	if (!m_pTrackSet)
	{
		ASSERT(m_pTrackSet);
		return false;
	}
	
	A3DSklTrackSet::BONETRACK* pBoneTrack = m_pTrackSet->GetBoneTrack(idJoint);
	if (!pBoneTrack || !pBoneTrack->pAbsPosTrack || !pBoneTrack->pAbsRotTrack)
	{
		ASSERT(pBoneTrack);
		return false;
	}

	vPos = pBoneTrack->pAbsPosTrack->GetKeyValue(iAbsTime);
	quRot = pBoneTrack->pAbsRotTrack->GetKeyValue(iAbsTime);
	quRot.Normalize();

	return true;
}

//	On start playing
void A3DSkinModelActionCore::OnStartPlaying()
{
	m_iRefCnt++;
}

//	On stop playing
void A3DSkinModelActionCore::OnStopPlaying()
{
	m_iRefCnt--;
	ASSERT(m_iRefCnt >= 0);
}

//	Check whether action can be removed from cache
bool A3DSkinModelActionCore::IsOutOfCacheDate(DWORD dwCurTime)
{
	if (!m_pTrackSet || !m_bOwnTckFile || m_iRefCnt > 0)
		return false;

	static const DWORD dwLifeTime = 300000;
	if (m_dwRefTime + dwLifeTime > dwCurTime)
		return false;

	return true;
}

///////////////////////////////////////////////////////////////////////////
//
//	Implement A3DSkinModelAction
//
