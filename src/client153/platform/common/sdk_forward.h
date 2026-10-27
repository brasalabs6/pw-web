/*
 * FILE: sdk_forward.h (platform seam)
 *
 * DESCRIPTION: Forward declarations the prebuilt SDK headers use without
 * including their owning headers (the original build supplied them through
 * precompiled headers). Pointer-only usages need just the declaration.
 *
 * Copyright (c) 2001 Archosaur Studio, All Rights Reserved.
 */

#ifndef PW153_PLATFORM_SDK_FORWARD_H
#define PW153_PLATFORM_SDK_FORWARD_H

#include <algorithm>	//	std::min/std::max users rely on the PCH-supplied include.

class A2DSpriteItem;
class A3DAdditionalView;
class A3DTerrain2Render;
class A3DLitModel;

#endif // PW153_PLATFORM_SDK_FORWARD_H
