/*
 * FILE: AFI.cpp
 *
 * DESCRIPTION: interface functions in Angelica File Module;
 *
 * CREATED BY: Hedi, 2001/12/31
 *
 * HISTORY:
 *
 * Copyright (c) 2001 Archosaur Studio, All Rights Reserved.	
 */

#include "AFI.h"
#include "AFPlatform.h"	//	AFI_PATH_SEPARATOR disk path seam.
#include "ALog.h"
#include "AFilePackage.h"
#include "AFilePackMan.h"
#include <unistd.h>	//	getcwd.
#include <cstring>
#include <string>
#include <stdexcept>

///////////////////////////////////////////////////////////////////////////
//
//	Local Types and Variables and Global variables
//
///////////////////////////////////////////////////////////////////////////

ALog*	g_pAFErrLog = NULL;
char	g_szBaseDir[MAX_PATH] = "";

///////////////////////////////////////////////////////////////////////////
//
//	Local functions
//
///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////
//
//	Implement
//
///////////////////////////////////////////////////////////////////////////

bool af_Initialize()
{
	if (g_pAFErrLog)
		af_Finalize();

	// By default, we should set the g_szBaseDir to current directory
	if (!getcwd(g_szBaseDir, MAX_PATH))
		g_szBaseDir[0] = '\0';	//	GetCurrentDirectoryA mapped to POSIX getcwd.
	int iLen = strlen(g_szBaseDir);
	if (g_szBaseDir[iLen-1] == '\\' || g_szBaseDir[iLen-1] == '/')
		g_szBaseDir[iLen-1] = '\0';

	if (!g_pAFErrLog)
	{
		if (!(g_pAFErrLog = new ALog))
			return false;

		g_pAFErrLog->Init("AF.log", "Angelica File Module Error Log");
	}

	return true;
}

bool af_SetBaseDir(const char* szBaseDir)
{
	strncpy(g_szBaseDir, szBaseDir, MAX_PATH);

	// Get rid of last '\\'
	if( g_szBaseDir[0] && g_szBaseDir[strlen(g_szBaseDir) - 1] == '\\' )
		g_szBaseDir[strlen(g_szBaseDir) - 1] = '\0';

	return true;
}

const char* af_GetBaseDir()
{
	if (!g_szBaseDir[0])
		af_Initialize();

	return g_szBaseDir;
}

bool af_Finalize()
{
	if( g_pAFErrLog )
	{
		g_pAFErrLog->Release();
		delete g_pAFErrLog;
		g_pAFErrLog = NULL;
	}

	g_szBaseDir[0] = '\0';
	return true;
}

void af_GetRelativePathNoBase(const char* szFullpath, const char* szParentPath, char* szRelativepath)
{
	const char* p1 = szParentPath;
	const char* p2 = szFullpath;
	
	while( *p1 && *p2 && (*p1 == *p2 || 
		(*p1 >= 'A' && *p1 <= 'Z' && *p1 + 0x20 == *p2) || 
		(*p2 >= 'A' && *p2 <= 'Z' && *p1 == *p2 + 0x20)) )
	{
		p1 ++;
		p2 ++;
	}

	if( *p1 ) // not found;
	{
		const auto length = std::strlen(szFullpath);
		if (length >= MAX_PATH) throw std::length_error("Angelica relative path exceeds MAX_PATH");
		std::memmove(szRelativepath, szFullpath, length + 1);
		return;
	}

	if( *p2 == '\\' )
		p2 ++;

	const auto length = std::strlen(p2);
	if (length >= MAX_PATH) throw std::length_error("Angelica relative path exceeds MAX_PATH");
	std::memmove(szRelativepath, p2, length + 1);
}

void af_GetRelativePathNoBase(const char* szFullpath, const char* szParentPath, AString& strRelativePath)
{
	char szRelativePath[MAX_PATH];
	af_GetRelativePathNoBase(szFullpath, szParentPath, szRelativePath);
	strRelativePath = szRelativePath;
}

void af_GetFullPathNoBase(char* szFullpath, const char* szBaseDir, const char* szFilename)
{
	if (!szFullpath || !szBaseDir || !szFilename)
		throw std::invalid_argument("null path argument");
	szFullpath[0] = '\0';
	const std::string name(szFilename), base(szBaseDir);
	if (name.empty()) return;
	std::string result;
	if ((name.size() > 3 && ((name[1] == ':' && name[2] == '\\') ||
		(name[0] == '\\' && name[1] == '\\'))) || name[0] == AFI_PATH_SEPARATOR)
		result = name;
	else if (name.size() > 1 && name[0] == '\\') return;
	else
	{
		const std::string relative = name.size() > 2 && name[0] == '.' && name[1] == '\\'
			? name.substr(2) : name;
		result = base;
		if (!result.empty() && result.back() != AFI_PATH_SEPARATOR)
			result += AFI_PATH_SEPARATOR;
		result += relative;
	}
	if (result.size() >= MAX_PATH)
		throw std::length_error("Angelica path exceeds MAX_PATH");
	std::memcpy(szFullpath, result.c_str(), result.size() + 1);
}

void af_GetFullPathNoBase(AString& strFullpath, const char* szBaseDir, const char* szFilename)
{
	char szFullPath[MAX_PATH];
	af_GetFullPathNoBase(szFullPath, szBaseDir, szFilename);
	strFullpath = szFullPath;	
}

void af_GetFullPath(char* szFullPath, const char* szFolderName, const char* szFileName)
{
	if (!szFolderName) throw std::invalid_argument("null folder name");
	const std::string szBaseDir = std::string(g_szBaseDir) + AFI_PATH_SEPARATOR_STR + szFolderName;
	if (szBaseDir.size() >= MAX_PATH) throw std::length_error("Angelica base path exceeds MAX_PATH");
	af_GetFullPathNoBase(szFullPath, szBaseDir.c_str(), szFileName);
}

void af_GetFullPath(char* szFullPath, const char* szFileName)
{
	af_GetFullPathNoBase(szFullPath, g_szBaseDir, szFileName);
}

void af_GetFullPath(AString& strFullPath, const char* szFolderName, const char* szFileName)
{
	if (!szFolderName) throw std::invalid_argument("null folder name");
	const std::string szBaseDir = std::string(g_szBaseDir) + AFI_PATH_SEPARATOR_STR + szFolderName;
	if (szBaseDir.size() >= MAX_PATH) throw std::length_error("Angelica base path exceeds MAX_PATH");
	af_GetFullPathNoBase(strFullPath, szBaseDir.c_str(), szFileName);
}

void af_GetFullPath(AString& strFullPath, const char* szFileName)
{
	af_GetFullPathNoBase(strFullPath, g_szBaseDir, szFileName);
}

void af_GetRelativePath(const char* szFullPath, const char* szFolderName, char* szRelativePath)
{
	if (!szFolderName) throw std::invalid_argument("null folder name");
	const std::string szBaseDir = std::string(g_szBaseDir) + AFI_PATH_SEPARATOR_STR + szFolderName;
	if (szBaseDir.size() >= MAX_PATH) throw std::length_error("Angelica base path exceeds MAX_PATH");
	af_GetRelativePathNoBase(szFullPath, szBaseDir.c_str(), szRelativePath);
}

void af_GetRelativePath(const char* szFullPath, char* szRelativePath)
{
	af_GetRelativePathNoBase(szFullPath, g_szBaseDir, szRelativePath);
}

void af_GetRelativePath(const char* szFullPath, const char* szFolderName, AString& strRelativePath)
{
	if (!szFolderName) throw std::invalid_argument("null folder name");
	const std::string szBaseDir = std::string(g_szBaseDir) + AFI_PATH_SEPARATOR_STR + szFolderName;
	if (szBaseDir.size() >= MAX_PATH) throw std::length_error("Angelica base path exceeds MAX_PATH");
	af_GetRelativePathNoBase(szFullPath, szBaseDir.c_str(), strRelativePath);
}

void af_GetRelativePath(const char* szFullPath, AString& strRelativePath)
{
	af_GetRelativePathNoBase(szFullPath, g_szBaseDir, strRelativePath);
}

bool af_GetFileTitle(const char* lpszFile, char* lpszTitle, WORD cbBuf)
{
	if (!lpszFile || !lpszTitle)
		return false;

	lpszTitle[0] = '\0';
	if (lpszFile[0] == '\0')
		return true;

	const char* pszTemp = lpszFile + strlen(lpszFile);

	--pszTemp;
    if ('\\' == (* pszTemp) || '/' == (* pszTemp)) return false;
    while (true) {
        if ('\\' == (* pszTemp) || '/' == (* pszTemp))
		{
            ++pszTemp;
            break;
        }
        if (pszTemp == lpszFile) break;
        --pszTemp;
    }
    //	Original ignored cbBuf (unbounded copy); the bound is honored now.
    std::strncpy(lpszTitle, pszTemp, cbBuf);
    lpszTitle[cbBuf - 1] = '\0';
    return true;
}

bool af_GetFileTitle(const char* lpszFile, AString& strTitle)
{
	char szTitle[MAX_PATH];
	bool bRet = af_GetFileTitle(lpszFile, szTitle, MAX_PATH);
	if (bRet)
		strTitle = szTitle;

	return bRet;
}

bool af_GetFilePath(const char* lpszFile, char* lpszPath, WORD cbBuf)
{
	if (!lpszFile || !lpszPath)
		return false;

	lpszPath[0]= '\0';
	if (lpszFile[0] == '\0')
		return true;

	strncpy(lpszPath, lpszFile, cbBuf);
    char* pszTemp = (char *)lpszPath + strlen(lpszPath);

    --pszTemp;
    while (true) {
        if ('\\' == (* pszTemp) || '/' == (* pszTemp)) 
		{
            break;
        }
        if (pszTemp == lpszPath) break;
        --pszTemp;
    }
    *pszTemp = '\0';
    return true;
}

bool af_GetFilePath(const char* lpszFile, AString& strPath)
{
	char szPath[MAX_PATH];
	bool bRet = af_GetFilePath(lpszFile, szPath, MAX_PATH);
	if (bRet)
		strPath = szPath;

	return bRet;
}

//	Check file extension
bool af_CheckFileExt(const char* szFileName, const char* szExt, int iExtLen/* -1 */, int iFileNameLen/* -1 */)
{
	ASSERT(szFileName && szExt);

	if (iFileNameLen < 0)
		iFileNameLen = strlen(szFileName);

	if (iExtLen < 0)
		iExtLen = strlen(szExt);

	const char* p1 = szFileName + iFileNameLen - 1;
	const char* p2 = szExt + iExtLen - 1;

	bool bMatch = true;

	while (p2 >= szExt && p1 >= szFileName)
	{
		if (*p1 != *p2 && !(*p1 >= 'A' && *p1 <= 'Z' && *p2 == *p1 + 32) &&
			!(*p1 >= 'a' && *p1 <= 'z' && *p2 == *p1 - 32))
		{
			bMatch = false;
			break;
		}

		p1--;
		p2--;
	}

	return bMatch;
}

//	Change file extension
bool af_ChangeFileExt(char* szFileNameBuf, int iBufLen, const char* szNewExt)
{
	char szFile[MAX_PATH];
	strcpy(szFile, szFileNameBuf);

	char* pTemp = strrchr(szFile, '.');
	if (pTemp)
		strcpy(pTemp, szNewExt);
	else
		strcat(szFile, szNewExt);

	int iLen = strlen(szFile);
	if (iLen >= iBufLen)
	{
		ASSERT(iLen < iBufLen);
		return false;
	}

	strcpy(szFileNameBuf, szFile);
	return true;
}

bool af_ChangeFileExt(AString& strFileName, const char* szNewExt)
{
	char szFile[MAX_PATH];
	strcpy(szFile, strFileName);

	char* pTemp = strrchr(szFile, '.');
	if (pTemp)
		strcpy(pTemp, szNewExt);
	else
		strcat(szFile, szNewExt);

	strFileName = szFile;
	return true;
}

// Check if a specified file exist
bool af_IsFileExist(const char * szFileName)
{
	char szRelativePath[MAX_PATH];
	af_GetRelativePath(szFileName, szRelativePath);

	// we must supply a relative path to GetFilePck function
	AFilePackBase * pPackage = g_AFilePackMan.GetFilePck(szRelativePath);
	if (pPackage)
	{
		if (pPackage->IsFileExist(szRelativePath))
			return true;
	}

	// not found in package, so test if exist on the disk, here we must use full path
	char szFullPath[MAX_PATH];
	af_GetFullPath(szFullPath, szFileName);
	//	GetFileAttributesA mapped to a portable read-open probe.
	if (FILE* probe = fopen(szFullPath, "rb"))
	{
		fclose(probe);
		return true;
	}

	return false;
}

