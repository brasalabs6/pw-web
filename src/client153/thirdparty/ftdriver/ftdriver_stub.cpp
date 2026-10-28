/*
 * FTDriver browser-build boundary (explicit failing seam).
 *
 * The original client consumes the FreeType font driver as the prebuilt
 * Windows DLL FTDriver.dll (Common/FTDriver/FTDriverDLL.NET2005 ships
 * FTDriver.dll + FTDriver.lib; the game links the import library and
 * loads the DLL at runtime). The wrapper's own sources exist in the
 * pinned tree, but they bind FreeType 2.1.10 and its removed cache API
 * (freetype/ftcache.h, FTC_Manager_New/FTC_CMapCache_New), so closing the
 * boundary for real is a standalone library milestone: build FreeType
 * 2.1.10 for wasm32 plus the FTDriver wrapper TUs (FTFont, WTFont,
 * MyFTCache, MyRaster, FTGlyphCache, CriticalSession).
 *
 * This translation unit defines exactly the DLL entry point the linked
 * game objects reference (the full-closure link worklist):
 * CreateFTManager. It fails explicitly - the documented contract is a
 * NULL return on failure, and A3DFTFontMan::Create handles NULL by
 * returning false, so the FTDriver font path degrades to the engine's
 * existing non-FreeType font rendering.
 *
 * Declared limitation (never silent): the browser build cannot render
 * fonts through FTDriver until the FreeType 2.1.10 + wrapper chain is
 * ported (or a browser font adapter implements the IFTManager contract).
 *
 * This file is new code by the browser port (not an upstream adaptation);
 * it compiles against the unmodified vendored wrapper header.
 */

#include "FTInterface.h"

#include <cstdio>

namespace
{

bool ftdriver_limitation_logged()
{
	static bool logged = false;
	if (!logged)
	{
		logged = true;
		std::fprintf(stderr,
			"pw-browser: the FTDriver font DLL (FreeType 2.1.10 cache API) "
			"is not available in the browser build; CreateFTManager fails "
			"and the engine falls back to its non-FreeType font path\n");
	}
	return logged;
}

} // namespace

//	The documented DLL entry point (FTInterface.h); the real FTDriver.dll
//	returns an IFTManager over FreeType - the browser build reports the
//	failure the documented contract defines for it.
IFTManager * CreateFTManager(int nMaxFontFamilyCountInCache,
                             int nMaxFontSizeInCache, int nMaxCacheSizeBytes)
{
	(void)nMaxFontFamilyCountInCache;
	(void)nMaxFontSizeInCache;
	(void)nMaxCacheSizeBytes;
	ftdriver_limitation_logged();
	return NULL;
}
