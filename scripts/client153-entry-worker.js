// The browser entry probe worker: receives the offscreen canvas from the
// page, exposes it to the module (Module.canvas via globalThis.pwCanvas),
// then loads the entry module whose pre-js maps the packages and drives
// the original wWinMain. All client output flows back to the page through
// the postMessage channel the pre-js set up.
self.onmessage = function (event) {
  if (event.data instanceof OffscreenCanvas) {
    globalThis.pwCanvas = event.data;
    importScripts('/entry.js');
  }
};
