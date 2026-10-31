/*
 * FMOD event-system browser-build boundary (explicit failing seam).
 *
 * The AudioEngine::Event/EventInstance/EventSystem surface is the game's
 * wrapper around Firelight Technologies' FMOD Event System. The pinned
 * upstream tree ships the wrapper headers (Common/Output/include/aaudio)
 * and the prebuilt Windows libraries only — FMOD is commercial middleware
 * with no source in the authorized tree, so the browser build cannot
 * contain the real implementation.
 *
 * This translation unit defines exactly the three wrapper symbols the
 * original client references (the full-closure link worklist) with
 * explicit failure semantics: instance creation and event lookup fail,
 * playback start fails, and the limitation is logged once. AUIManager's
 * UI-sound path and FX_BINDING's AUDIOEVENT_BINDING take their existing
 * failure paths.
 *
 * Declared limitation (never silent): the browser build plays no
 * FMOD-based sounds until a WebAudio adapter implements this wrapper's
 * contract. The rest of the wrapper classes are left undefined on
 * purpose — the client does not call them.
 *
 * This file is new code by the browser port (not an upstream adaptation);
 * it compiles against the unmodified vendored wrapper headers.
 */

#include "FEvent.h"
#include "FEventInstance.h"
#include "FEventSystem.h"

#include <cstdio>

namespace
{

bool fmod_limitation_logged()
{
	static bool logged = false;
	if (!logged)
	{
		logged = true;
		std::fprintf(stderr,
			"pw-browser: the FMOD event system is not available in the "
			"browser build (commercial middleware, no source in the "
			"pinned tree); FMOD-based sound playback is disabled.\n");
	}
	return false;
}

}  // namespace

namespace AudioEngine
{

EventInstance* Event::CreateInstance(bool bAutoDestroy)
{
	(void)bAutoDestroy;
	fmod_limitation_logged();
	return NULL;
}

bool EventInstance::Start(LAYER_INDEX LayerIndex)
{
	(void)LayerIndex;
	fmod_limitation_logged();
	return false;
}

Event* EventSystem::GetEvent(const char* szGuid)
{
	(void)szGuid;
	fmod_limitation_logged();
	return NULL;
}

}  // namespace AudioEngine
