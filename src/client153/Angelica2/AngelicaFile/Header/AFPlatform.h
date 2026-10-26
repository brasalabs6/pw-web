/*
 * FILE: AFPlatform.h
 *
 * DESCRIPTION: header for a platform related headers list
 *
 * CREATED BY: Hedi, 2001/12/3
 *
 * HISTORY:
 *
 * Copyright (c) 2001 Archosaur Studio, All Rights Reserved.
 *
 * Adapted from original AFPlatform.h, revision and hashes in source-manifest.json.
 * Windows umbrella replaced by direct includes; CRITICAL_SECTION comes from the
 * portable platform seam in platform/common/criticalsection.h.
 */

#ifndef _AFPLATFORM_H_
#define _AFPLATFORM_H_

#include <cassert>
#include <cstdio>
#include <cstdlib>

#include "criticalsection.h"

#ifndef MAX_PATH
#define MAX_PATH 260
#endif

//	Disk path separator. Windows keeps the original backslash; POSIX targets
//	join with '/'. Package-internal names keep the format's backslashes.
#ifdef _WIN32
#define AFI_PATH_SEPARATOR '\\'
#define AFI_PATH_SEPARATOR_STR "\\"
#else
#define AFI_PATH_SEPARATOR '/'
#define AFI_PATH_SEPARATOR_STR "/"
#endif

#endif
