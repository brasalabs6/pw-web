/*
 * FILE: WindowsX.h (platform seam)
 *
 * DESCRIPTION: The message-cracking helpers the original uses, with real
 * implementations over the seam types.
 *
 * Copyright (c) 2001 Archosaur Studio, All Rights Reserved.
 */

#ifndef PW153_PLATFORM_WINDOWSX_H
#define PW153_PLATFORM_WINDOWSX_H

#include "Windows.h"

inline int GET_X_LPARAM(LPARAM lParam) { return static_cast<int>(static_cast<std::int16_t>(LOWORD(lParam))); }
inline int GET_Y_LPARAM(LPARAM lParam) { return static_cast<int>(static_cast<std::int16_t>(HIWORD(lParam))); }

#endif // PW153_PLATFORM_WINDOWSX_H
