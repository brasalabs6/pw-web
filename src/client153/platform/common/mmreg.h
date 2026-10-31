/*
 * FILE: mmreg.h (platform seam)
 *
 * DESCRIPTION: Multimedia format registry header; the client reaches it
 * through mmsystem.h. The format structures the audio cycle needs land
 * with the audio adapter; the include resolves today through the seam.
 *
 * Copyright (c) 2001 Archosaur Studio, All Rights Reserved.
 */
#ifndef PW153_PLATFORM_MMREG_H
#define PW153_PLATFORM_MMREG_H
#include "mmsystem.h"

//	The RIFF fourcc (the documented mmreg.h constant; the wave-file
//	loader checks the file type with it).
#define FOURCC_RIFF mmioFOURCC('R', 'I', 'F', 'F')
#endif
