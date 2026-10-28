/*
 * Defence (anti-cheat) browser-build boundary (explicit failing seam).
 *
 * The original client embeds a Windows anti-cheat stack under
 * Element/CElementClient/defence/: native-code stack walking (MS inline
 * assembly), DLL loading, process/window enumeration and raw-socket
 * fingerprinting, all feeding the gacd server-side protocol. None of it can
 * exist in a browser sandbox, and the defining TUs cannot compile for
 * wasm32 (MS-style inline assembly is rejected outright).
 *
 * This translation unit defines exactly the defence API surface the linked
 * game objects reference (the full-closure link worklist): the CheaterKiller,
 * StackChecker and NetDLL entry points that ElementClient.cpp and
 * EC_GameRun.cpp call. Every entry fails explicitly: data collection and
 * the anti-cheat thread are absent (logged once), DLL registration and
 * parsing fail, and the gacd callbacks produce no data.
 *
 * Declared limitation (never silent): the browser build carries no
 * client-side anti-cheat; server-side validation is the enforcement
 * boundary for the browser target.
 *
 * This file is new code by the browser port (not an upstream adaptation);
 * it compiles against the unmodified vendored defence headers.
 */

#include "cheaterkiller.h"
#include "stackchecker.h"

//	APILoader.h references Marshal::OctetsStream unqualified; the original
//	translation units saw the GNET namespace through the project's
//	precompiled-header context.
#include "gnmarshal.h"
using namespace GNET;

#include "APILoader.h"

#include <cstdio>

namespace
{

bool defence_limitation_logged()
{
	static bool logged = false;
	if (!logged)
	{
		logged = true;
		std::fprintf(stderr,
			"pw-browser: the client-side anti-cheat (defence) is not "
			"available in the browser build (Windows native-code "
			"defence cannot run in a browser sandbox); the defence "
			"entry points fail explicitly.\n");
	}
	return false;
}

}  // namespace

namespace CheaterKiller
{

void WINAPI AppendCheatInfo(LPVOID buf)
{
	(void)buf;
	defence_limitation_logged();
}

void WINAPI TryKillCheaters()
{
	defence_limitation_logged();
}

}  // namespace CheaterKiller

namespace StackChecker
{

void ACStackCheckInit()
{
	defence_limitation_logged();
}

void ACTrace(int id)
{
	(void)id;
	defence_limitation_logged();
}

VOID WINAPI GetStackCheckData(void* data)
{
	(void)data;
	defence_limitation_logged();
}

VOID WINAPI UpdateStackCheckCode(void* code)
{
	(void)code;
	defence_limitation_logged();
}

}  // namespace StackChecker

namespace NetDLL
{

void WINAPI Register(WORD id, LPVOID entry)
{
	(void)id;
	(void)entry;
	defence_limitation_logged();
}

const char* WINAPI GetString(WORD id)
{
	(void)id;
	defence_limitation_logged();
	return NULL;
}

bool WINAPI Parse(LPVOID buffer, DWORD size)
{
	(void)buffer;
	(void)size;
	defence_limitation_logged();
	return false;
}

void WINAPI AppendSpyInfo(DWORD id, LPVOID buf, size_t size)
{
	(void)id;
	(void)buf;
	(void)size;
	defence_limitation_logged();
}

void WINAPI DumpResult(Marshal::OctetsStream& os)
{
	(void)os;
	defence_limitation_logged();
}

bool WINAPI DumpResultEx(Marshal::OctetsStream& os)
{
	(void)os;
	defence_limitation_logged();
	return false;
}

}  // namespace NetDLL
