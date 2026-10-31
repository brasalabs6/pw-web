/*
 * FILE: ABaseDef.h
 *
 * DESCRIPTION: Base definition
 *
 * CREATED BY: duyuxin, 2003/6/6
 *
 * HISTORY:
 *
 * Copyright (c) 2001 Archosaur Studio, All Rights Reserved.
 */

#ifndef _ABASEDEF_H_
#define _ABASEDEF_H_

#include <cstdint>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef MAX_PATH
#define MAX_PATH 260	//	Windows path limit kept for original buffers.
#endif

///////////////////////////////////////////////////////////////////////////
//
//	Define and Macro
//
///////////////////////////////////////////////////////////////////////////

#ifndef ASSERT
#include <assert.h>
#define ASSERT assert
#endif //	ASSERT

///////////////////////////////////////////////////////////////////////////
//
//	Types and Global variables
//
///////////////////////////////////////////////////////////////////////////

//	unsigned value
typedef std::uint32_t DWORD; //	32-bit;
typedef std::uint64_t DWORD64; //	64-bit;
typedef unsigned short WORD; //	16-bit;
typedef unsigned char BYTE;  //	8-bit;

typedef int BOOL;
#define TRUE 1
#define FALSE 0

#ifdef UNICODE
// The whole port compiles with -fshort-wchar: wchar_t keeps the original
// MSVC 16-bit width, so client strings and serialized protocol fields stay
// UTF-16 code units exactly like the original build.
typedef wchar_t ACHAR;
#define _AL(str) L##str
#else
typedef char ACHAR;
#define _AL(str) str
#endif

//	typedef int				INT
//	commone integer ---- int, but the size undetermined, so not defined, use
//carefully;

//	signed value
typedef std::int32_t LONG; //	32-bit;
typedef short SHORT;       //	16-bit;
typedef char CHAR;         //	8-bit;
typedef float FLOAT;       //	float
typedef double DOUBLE;     //	double

///////////////////////////////////////////////////////////////////////////
//
//	Declare of Global functions
//
///////////////////////////////////////////////////////////////////////////

#endif //	_ABASEDEF_H_
