//	wingdi.h — the GDI surface the original CFWEditorLib glyph/tess headers
//	include (<WinGDI.h>/<wingdi.h>; case-variant includes fixed to this
//	name). A browser page has no GDI: the font-object creation fails with
//	NULL, so the FW text/outline glyph paths bail the same way they do on
//	a machine without the requested font. The font-enumeration and
//	LOGFONT surfaces live in Windows.h (force-included first); only the
//	types the glyph headers reference are defined here.

#ifndef PW153_WINGDI_H
#define PW153_WINGDI_H

//	The fixed-point number and the transform matrix (the documented
//	wingdi layouts; GetGlyphOutline's MAT2 argument).
typedef struct _FIXED {
  WORD fract;
  short value;
} FIXED;

typedef struct _MAT2 {
  FIXED eM11;
  FIXED eM12;
  FIXED eM21;
  FIXED eM22;
} MAT2, *LPMAT2;

//	The glyph metrics record (the documented wingdi layout; CCharOutline
//	keeps one per character and reads gmCellIncX for the advance).
typedef struct _GLYPHMETRICS {
  UINT gmBlackBoxX;
  UINT gmBlackBoxY;
  POINT gmptGlyphOrigin;
  short gmCellIncX;
  short gmCellIncY;
} GLYPHMETRICS, *LPGLYPHMETRICS;

//	The font-object creation: no GDI fonts exist in a browser page, so
//	every request fails (NULL = documented failure; the callers check the
//	handle before selecting it).
inline HFONT CreateFontIndirectW(const LOGFONTW *logfont) {
  (void)logfont;
  return NULL;
}
inline HFONT CreateFontIndirectA(const LOGFONTA *logfont) {
  (void)logfont;
  return NULL;
}
inline HFONT CreateFontIndirect(const LOGFONTW *logfont) {
  return CreateFontIndirectW(logfont);
}

//	The region API (the documented wingdi forms; the FW text glyph hit-test
//	uses a polygon region). A page has no GDI regions, so creation fails
//	(NULL) and the point-in-region test reports false - the glyph falls
//	back to its rectangle hit-test.
#define ALTERNATE 1
#define WINDING 2
typedef POINT *LPPOINT;
inline HRGN CreatePolygonRgn(const POINT *points, int count, int mode) {
  (void)points; (void)count; (void)mode;
  return NULL;
}
inline BOOL PtInRegion(HRGN region, int x, int y) {
  (void)region; (void)x; (void)y;
  return FALSE;
}

#endif  // PW153_WINGDI_H
