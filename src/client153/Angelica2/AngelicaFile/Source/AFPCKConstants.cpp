/*
 * FILE: AFPCKConstants.cpp
 *
 * DESCRIPTION: Package format constants for the Angelica File Lib.
 *
 * The original 1.5.3 snapshot declares AFPCK_GUARDBYTE0, AFPCK_GUARDBYTE1,
 * AFPCK_MASKDWORD and AFPCK_CHECKMASK as extern ints defined in the Element
 * client project, outside the Angelica library. This port defines them with
 * the values proven against the authorized v145 packages:
 * tools/assets/pck153.py (validated reader, 15 packages / 304,636 files) and
 * a direct byte read of element/facedata.pck (2026-10-26) both show
 * guard0 0xFDFDFEEE, guard1 0xF00DBEEF, entry-offset mask 0xA8937462 and
 * check mask 0x59374231. The stale 0xabcdefab / 0xffeeffee comments in
 * AFilePackage.h predate the v145 format.
 *
 * Copyright (c) 2001 Archosaur Studio, All Rights Reserved.
 */

#include "ABaseDef.h"

int AFPCK_GUARDBYTE0 = 0xFDFDFEEE;
int AFPCK_GUARDBYTE1 = 0xF00DBEEF;
int AFPCK_MASKDWORD = 0xA8937462;
int AFPCK_CHECKMASK = 0x59374231;
