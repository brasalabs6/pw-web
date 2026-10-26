/*
 * FILE: AFPI.h
 *
 * DESCRIPTION: private interface functions for Angelica File Lib;
 *
 * CREATED BY: Hedi, 2001/12/31
 *
 * HISTORY:
 *
 * Copyright (c) 2001 Archosaur Studio, All Rights Reserved.	
 */

#ifndef _AFPI_H_
#define _AFPI_H_

#include "AFPlatform.h"
#include "ALog.h"
#include <zlib.h>	//	Original vendored ZLib; system zlib has the same API.

extern ALog*	g_pAFErrLog;
extern char		g_szBaseDir[MAX_PATH];

// You must use a () to include the fmt string;
// For example AFERRLOG(("Error Occurs at %d", nval))

#define AFERRLOG(fmt) {if(g_pAFErrLog) g_pAFErrLog->Log fmt;}

#endif

