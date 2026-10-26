/*
 * FILE: AMemory.h
 *
 * DESCRIPTION: Routines for memory allocating and freeing
 *
 * CREATED BY: duyuxin, 2003/6/4
 *
 * HISTORY:
 *
 * Copyright (c) 2001 Archosaur Studio, All Rights Reserved.
 *
 * Adapted from original AMemory.h, revision and hashes in source-manifest.json.
 * Keeps the original release-mode behavior: A_DEBUG_NEW maps to plain new and
 * a_malloc/a_realloc/a_free map to malloc/realloc/free, exactly like the
 * original _A_FORBID_MALLOC path. The custom allocator (AMemory.cpp,
 * AMemoryDbg.cpp) and its global operator new/delete overrides are omitted
 * until a port consumer needs them; the owning-container copy guard in
 * AContainers.cpp prevents the double-free class of bugs the allocator tracked.
 */

#ifndef _AMEMORY_H_
#define _AMEMORY_H_

#include "ABaseDef.h"

#pragma once

///////////////////////////////////////////////////////////////////////////
//
//	Define and Macro
//
///////////////////////////////////////////////////////////////////////////

//	Enable below line to forbid new and delete operators
//	#define	_A_FORBID_NEWDELETE
//	Enable below line ot forbid a_malloc and a_free functions
//	#define _A_FORBID_MALLOC

#if (defined (_DEBUG) && !defined (_A_FORBID_NEWDELETE))
//	#define A_DEBUG_NEW new (__FILE__, __LINE__)
#define A_DEBUG_NEW new
#else
#define A_DEBUG_NEW new
#endif

#define a_malloc(size) malloc(size)
#define a_realloc(pMem, size) realloc(pMem, size)
#define a_free(p) free(p)

#define a_malloctemp(size) malloc(size)
#define a_freetemp(p) free(p)

///////////////////////////////////////////////////////////////////////////
//
//	Types and Global variables
//
///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////
//
//	Declare of Global functions
//
///////////////////////////////////////////////////////////////////////////

//	The original custom allocator entry points (_a_New, _a_Delete,
//	_a_NewAlign, _a_Realloc, _a_MallocTemp and the debug variants) are omitted
//	with this adaptation; use new/delete and the a_malloc/a_free macros above.

///////////////////////////////////////////////////////////////////////////
//
//	Inline functions
//
///////////////////////////////////////////////////////////////////////////

#endif	//	_AMEMORY_H_
