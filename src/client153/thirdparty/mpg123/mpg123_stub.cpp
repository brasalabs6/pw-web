/*
 * mpg123 browser-build boundary (explicit failing seam).
 *
 * The pinned 1.5.3 tree ships the MP3 decoder as Mp3Lib.h + the prebuilt
 * Windows library mpg123lib.lib only (no source in 3rdsdk). The CMpg123
 * members AMMp3File.cpp references are therefore defined here with
 * explicit failure semantics: mpg123_open reports failure (0), so
 * AMMp3File::Open fails and the game logs the error and continues without
 * MP3 audio.
 *
 * Declared limitation (never silent): the browser build cannot decode
 * MP3 until a real codec (e.g. an Emscripten mpg123 build) replaces this
 * translation unit.
 *
 * This file is new code by the browser port (not an upstream adaptation).
 */

#include <cstdio>

#include "Mp3Lib.h"

namespace
{

bool mp3_limitation_logged()
{
	static bool logged = false;
	if (!logged)
	{
		logged = true;
		std::fprintf(stderr,
			"pw-browser: MP3 decoding is not available in the browser "
			"build (the pinned tree ships the codec as a prebuilt "
			"Windows library only); MP3 audio open fails.\n");
	}
	return false;
}

}  // namespace

CMpg123::CMpg123()
{
	m_decoder = NULL;
	m_readers = NULL;
	stereo = 0;
	frequency = 0;
	pcm_sample = NULL;
	pcm_point = 0;
}

CMpg123::~CMpg123()
{
}

int CMpg123::mpg123_open(char *szfilename)
{
	(void)szfilename;
	mp3_limitation_logged();
	return 0;
}

int CMpg123::mpg123_close()
{
	mp3_limitation_logged();
	return 0;
}

int CMpg123::mpg123_seek(int nSample)
{
	(void)nSample;
	mp3_limitation_logged();
	return 0;
}

int CMpg123::mpg123_read(char *buffer, long size, long *preadsize,
                         int *reachend)
{
	(void)buffer;
	(void)size;
	if (preadsize)
		*preadsize = 0;
	if (reachend)
		*reachend = 1;
	mp3_limitation_logged();
	return 0;
}

int CMpg123::mpg123_get_total_sample()
{
	mp3_limitation_logged();
	return 0;
}

int CMpg123::mpg123_getinfo(long *pFrequency, int *pStereo)
{
	if (pFrequency)
		*pFrequency = 0;
	if (pStereo)
		*pStereo = 0;
	mp3_limitation_logged();
	return 0;
}
