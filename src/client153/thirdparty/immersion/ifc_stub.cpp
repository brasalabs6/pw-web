/*
 * Immersion TouchSense (IFC) browser-build boundary (explicit failing seam).
 *
 * The Immersion Foundation Classes are proprietary middleware (headers and
 * prebuilt Windows library only — neither is in the pinned tree), and force
 * feedback cannot exist in a browser sandbox. This translation unit defines
 * exactly the CImmDevice/CImmProject surface AMImmWrapper.cpp references,
 * failing explicitly: device creation returns NULL so the wrapper's own
 * Init error path runs and the game continues without force feedback.
 *
 * Declared limitation (never silent): the browser build has no force
 * feedback until a browser gamepad-vibration adapter implements the
 * wrapper's contract.
 *
 * This file is new code by the browser port (not an upstream adaptation).
 */

#include <cstdio>

#include "ifc.h"

namespace
{

bool immersion_limitation_logged()
{
	static bool logged = false;
	if (!logged)
	{
		logged = true;
		std::fprintf(stderr,
			"pw-browser: force feedback (Immersion TouchSense) is not "
			"available in the browser build (proprietary SDK, no source "
			"in the pinned tree); the force-feedback device fails to "
			"create.\n");
	}
	return false;
}

}  // namespace

CImmDevice *CImmDevice::CreateDevice(HMODULE module, HWND window)
{
	(void)module;
	(void)window;
	immersion_limitation_logged();
	return NULL;
}

CImmCompoundEffect::~CImmCompoundEffect()
{
}

CImmProject::CImmProject()
{
}

CImmProject::~CImmProject()
{
}

BOOL CImmProject::OpenFile(const char *projectName, CImmDevice *device)
{
	(void)projectName;
	(void)device;
	immersion_limitation_logged();
	return FALSE;
}

void CImmProject::Close()
{
}

CImmCompoundEffect *CImmProject::CreateEffect(const char *effectName,
                                              CImmDevice *device)
{
	(void)effectName;
	(void)device;
	return NULL;
}

void CImmProject::DestroyEffect(CImmCompoundEffect *effect)
{
	(void)effect;
}

BOOL CImmProject::Start(const char *effectName)
{
	(void)effectName;
	immersion_limitation_logged();
	return FALSE;
}
