/*
 * FILE: AChar.h
 *
 * DESCRIPTION: ACHAR routines
 *
 * CREATED BY: duyuxin, 2004/2/10
 *
 * HISTORY:
 *
 * Copyright (c) 2001 Archosaur Studio, All Rights Reserved.
 *
 * Adapted from original AChar.h, revision and hashes in source-manifest.json.
 * The UNICODE branch maps to the port's 16-bit wide seam: the system wchar_t
 * CRT reads 32-bit units under the native ABI and is wrong for the port's
 * 16-bit strings, so the string/scan/convert macros route through
 * platform/common/widestr. The console and wide-file variants have no
 * browser equivalent yet and stay on the standard library names, which the
 * runtime reports as pending behavior, not fake success.
 */

#ifndef _ACHAR_H_
#define _ACHAR_H_

#include <wchar.h>
#include <stdlib.h>
#include <math.h>

#include "widestr.h"

///////////////////////////////////////////////////////////////////////////
//
//	Define and Macro
//
///////////////////////////////////////////////////////////////////////////

#ifdef UNICODE

	#define a_strcpy	wcscpy
	#define a_strncpy	wcsncpy
	#define a_strlen		pw153::platform::wide_strlen
	#define	a_strcmp		pw153::platform::wide_strcmp
	#define a_stricmp	pw153::platform::wide_casecmp
	#define a_strcat		wcscat
	#define a_strrchr	pw153::platform::wide_strrchr
	#define a_strstr		pw153::platform::wide_strstr
	#define a_strcspn	pw153::platform::wide_strcspn
	#define a_strupr		pw153::platform::wide_strupr
	#define a_strlwr		pw153::platform::wide_strlwr

	#define a_scanf		wscanf
	#define a_sscanf	pw153::platform::wide_sscanf
	#define a_printf	wprintf
	#define a_sprintf	pw153::platform::wide_sprintf
	#define a_vsprintf	pw153::platform::wide_vsprintf
	#define a_atoi		pw153::platform::wide_toi
	#define a_getc		getwc
	#define a_putc		putwc
	#define a_getchar	getwchar
	#define a_putchar	putwchar
	#define a_gets		_getws
	#define a_puts		_putws

	#define a_fputc		fputwc
	#define	a_fgetc		fgetwc
	#define	a_fgets		fgetws
	#define	a_fputs		fputws
	#define a_fscanf	fwscanf
	#define a_fprintf	fwprintf

#else

	#define a_strcpy	strcpy
	#define a_strncpy	strncpy
	#define a_strlen	strlen
	#define a_strcmp	strcmp
	#define a_stricmp	strcasecmp
	#define a_strcat	strcat
	#define a_strrchr	strrchr
	#define a_strstr	strstr
	#define a_strcspn	strcspn
	#define a_strupr	pw153::platform::narrow_strupr
	#define a_strlwr	pw153::platform::narrow_strlwr

	#define a_scanf		scanf
	#define a_sscanf	sscanf
	#define a_printf	printf
	#define a_sprintf	sprintf
	#define a_vsprintf	vsprintf
	#define a_atoi		atoi
	#define a_getc		getc
	#define a_putc		putc
	#define a_getchar	getchar
	#define a_putchar	putchar
	#define a_gets		gets
	#define a_puts		puts

	#define a_fputc		fputc
	#define	a_fgetc		fgetc
	#define	a_fgets		fgets
	#define	a_fputs		fputs
	#define a_fscanf	fscanf
	#define a_fprintf	fprintf

#endif	//	UNICODE

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


#endif	//	_ACHAR_H_
