/*
 * DirectSound factory seam (explicit failing implementation).
 *
 * dsound.h declares DirectSoundCreate8 for the AngelicaMedia sound engine;
 * the real device creation belongs to the browser audio adapter, which does
 * not exist yet. Until it does, the factory fails explicitly: the out
 * parameter is nulled and DSERR_GENERIC is returned, so AMSoundEngine::Init
 * takes its documented failure path (no DirectSound-based audio in the
 * browser build).
 */

#include "dsound.h"
#include "Windows.h"

#include <cstdio>

HRESULT DirectSoundCreate8(const void *pcGuidBuffer, IDirectSound8 **ppDS8,
                           void *pUnkOuter)
{
	(void)pcGuidBuffer;
	(void)pUnkOuter;
	if (ppDS8)
		*ppDS8 = NULL;
	static bool logged = false;
	if (!logged)
	{
		logged = true;
		std::fprintf(stderr,
			"pw-browser: DirectSound is not available in the browser build "
			"(the Web Audio adapter owns the real device later); "
			"DirectSoundCreate8 fails.\n");
	}
	SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
	return DSERR_GENERIC;
}
