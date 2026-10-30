//	Browser input adapter state table; the adapter writes real key events.
#include "Windows.h"

unsigned char pw153_key_state[256] = {0};

//	The message-loop quit flag the loop adapter consumes.
int pw153_quit_code = 0;

//	The input message queue: the browser bridge posts real Win32 messages
//	(key/mouse events translated by the pre-js) and the game's pump
//	(PeekMessage/DispatchMessage) drains them to the window procedure.
namespace {
constexpr int MESSAGE_QUEUE_CAPACITY = 256;
MSG message_queue[MESSAGE_QUEUE_CAPACITY];
int message_queue_head = 0;
int message_queue_size = 0;
}

extern "C" int pw153_post_message(unsigned message, unsigned wParam, unsigned lParam) {
  if (message_queue_size >= MESSAGE_QUEUE_CAPACITY)
    return 0;	//	full: the event is dropped (the queue drains each frame)
  MSG &slot = message_queue[(message_queue_head + message_queue_size) % MESSAGE_QUEUE_CAPACITY];
  slot.hwnd = nullptr;
  slot.message = message;
  slot.wParam = wParam;
  slot.lParam = lParam;
  slot.time = 0;
  slot.pt = POINT{0, 0};
  ++message_queue_size;
  return 1;
}

extern "C" int pw153_pop_message(unsigned *message, unsigned *wParam, unsigned *lParam) {
  if (message_queue_size <= 0)
    return 0;
  const MSG &slot = message_queue[message_queue_head];
  if (message) *message = slot.message;
  if (wParam) *wParam = static_cast<unsigned>(slot.wParam);
  if (lParam) *lParam = static_cast<unsigned>(slot.lParam);
  message_queue_head = (message_queue_head + 1) % MESSAGE_QUEUE_CAPACITY;
  --message_queue_size;
  return 1;
}

extern "C" void pw153_set_key_state(unsigned key, unsigned down) {
  pw153_key_state[key & 0xFF] = down ? 0x80 : 0;
}

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
