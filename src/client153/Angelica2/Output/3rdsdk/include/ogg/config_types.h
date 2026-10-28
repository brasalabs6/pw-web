/*
 * FILE: config_types.h (ogg SDK seam)
 *
 * DESCRIPTION: The per-platform integer types libogg's build generates
 * into this header (config_types.h.in). The pinned 1.5.3 tree ships the
 * ogg/vorbis SDK as headers + prebuilt Windows libraries only (see
 * 3rdsdk/oggvorbis — vcproj files without source), and the generated
 * header was never checked in; os_types.h includes it on every non-Win32
 * target, which the browser build is. The contents are the canonical
 * LP64 little-endian instantiation of that template (stdint types).
 *
 * Declared limitation (never silent): the ogg/vorbis decode entry points
 * remain unimplemented in the browser build — see
 * thirdparty/xiph/vorbis_stub.cpp.
 */

#ifndef __CONFIG_TYPES_H__
#define __CONFIG_TYPES_H__

/* include this first to avoid later warnings */
#include <stdint.h>

typedef int16_t ogg_int16_t;
typedef uint16_t ogg_uint16_t;
typedef int32_t ogg_int32_t;
typedef uint32_t ogg_uint32_t;
typedef int64_t ogg_int64_t;
typedef uint64_t ogg_uint64_t;

#endif
