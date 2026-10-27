//	Browser input adapter state table; the adapter writes real key events.
#include "Windows.h"

unsigned char pw153_key_state[256] = {0};

//	The message-loop quit flag the loop adapter consumes.
int pw153_quit_code = 0;

namespace {
POINT cursor_position{};
bool cursor_position_known = false;
POINT canvas_origin{};
bool canvas_origin_known = false;
}

extern "C" void pw153_set_cursor_position(LONG x, LONG y) {
  cursor_position = {x, y};
  cursor_position_known = true;
}

BOOL GetCursorPos(POINT *point) {
  if (!point || !cursor_position_known) return FALSE;
  *point = cursor_position;
  return TRUE;
}

extern "C" void pw153_set_canvas_origin(LONG x, LONG y) {
  canvas_origin = {x, y};
  canvas_origin_known = true;
}

BOOL ScreenToClient(HWND, POINT *point) {
  if (!point || !canvas_origin_known) return FALSE;
  point->x -= canvas_origin.x;
  point->y -= canvas_origin.y;
  return TRUE;
}

BOOL ClientToScreen(HWND, POINT *point) {
  if (!point || !canvas_origin_known) return FALSE;
  point->x += canvas_origin.x;
  point->y += canvas_origin.y;
  return TRUE;
}
