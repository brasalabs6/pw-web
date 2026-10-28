/*
 * Xiph.Org libvorbisfile browser-build boundary (explicit failing seam).
 *
 * The pinned 1.5.3 tree ships the ogg/vorbis SDK as headers + prebuilt
 * Windows libraries only (3rdsdk/oggvorbis holds the vcproj files without
 * the codec source; 3rdsdk/lib holds vorbis_static.lib). The libvorbisfile
 * entry points AMOggFile.cpp references are therefore defined here with
 * explicit failure semantics: ov_open_callbacks reports OV_EIMPL, so
 * AMOggFile::Open fails and the game logs the error and continues without
 * Ogg Vorbis audio.
 *
 * Declared limitation (never silent): the browser build cannot decode
 * Ogg Vorbis until a real codec (e.g. an Emscripten libvorbis build)
 * replaces this translation unit.
 *
 * This file is new code by the browser port (not an upstream adaptation).
 */

#include <vorbis/vorbisfile.h>

namespace
{

bool vorbis_limitation_logged()
{
	static bool logged = false;
	if (!logged)
	{
		logged = true;
		std::fprintf(stderr,
			"pw-browser: Ogg Vorbis decoding is not available in the "
			"browser build (the pinned tree ships the codec as a "
			"prebuilt Windows library only); Ogg audio open fails "
			"with OV_EIMPL.\n");
	}
	return false;
}

}  // namespace

int ov_open_callbacks(void *datasource, OggVorbis_File *vf,
                      char *initial, long ibytes, ov_callbacks callbacks)
{
	(void)datasource;
	(void)vf;
	(void)initial;
	(void)ibytes;
	(void)callbacks;
	vorbis_limitation_logged();
	return OV_EIMPL;
}

int ov_clear(OggVorbis_File *vf)
{
	(void)vf;
	vorbis_limitation_logged();
	return OV_EIMPL;
}

vorbis_info *ov_info(OggVorbis_File *vf, int link)
{
	(void)vf;
	(void)link;
	vorbis_limitation_logged();
	return NULL;
}

int ov_pcm_seek(OggVorbis_File *vf, ogg_int64_t pos)
{
	(void)vf;
	(void)pos;
	vorbis_limitation_logged();
	return OV_EIMPL;
}

ogg_int64_t ov_pcm_total(OggVorbis_File *vf, int i)
{
	(void)vf;
	(void)i;
	vorbis_limitation_logged();
	return OV_EIMPL;
}

long ov_read(OggVorbis_File *vf, char *buffer, int length,
             int bigendianp, int word, int sgned, int *bitstream)
{
	(void)vf;
	(void)buffer;
	(void)length;
	(void)bigendianp;
	(void)word;
	(void)sgned;
	(void)bitstream;
	vorbis_limitation_logged();
	return OV_EIMPL;
}
