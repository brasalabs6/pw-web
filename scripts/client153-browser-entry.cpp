// The browser entry probe harness (m-boot): creates the WebGL2 context on
// the offscreen canvas the host page transferred to the worker, makes it
// current, then drives the original client entry point. The reachable
// closure of pw153_browser_probe (which includes wWinMain) is what the
// --entry-symbol link retains; the probe page reports the result via the
// worker postMessage channel the pre-js sets up.
//
// Design by the Codex m-link wave (2026-10-28, session
// 01a17447-64cb-76a3-b35f-e00206a5ba66, draft at /tmp/pw153-browser-entry);
// landed and wired into scripts/link-client153-full.py by Grok.

#include <emscripten/html5.h>

#include <cstdio>
#include <cstdint>

extern int wWinMain(void *, void *, wchar_t *, int);
extern "C" bool pw153_window_attach(void *host);
extern "C" void pw153_window_detach();

extern "C" int pw153_browser_probe() {
  EmscriptenWebGLContextAttributes attributes;
  emscripten_webgl_init_context_attributes(&attributes);
  attributes.majorVersion = 2;
  // WebGL cannot blit a single-sample offscreen framebuffer into a
  // multisampled default framebuffer. The D3D backbuffer is single-sample.
  attributes.antialias = false;
  auto context = emscripten_webgl_create_context("#canvas", &attributes);
  printf("pw153_browser_probe: context handle = %d\n", (int)context);
  if (context <= 0)
    return -100;
  auto current = emscripten_webgl_make_context_current(context);
  printf("pw153_browser_probe: make_current = %d\n", (int)current);
  if (current != EMSCRIPTEN_RESULT_SUCCESS)
    return -101;
  if (!pw153_window_attach(reinterpret_cast<void *>(static_cast<intptr_t>(context))))
    return -102;
  wchar_t command[] = L"";
  const int result = wWinMain(nullptr, nullptr, command, 0);
  pw153_window_detach();
  return result;
}
