// Node-only diagnostic. Source packages are opened read-only and read on demand.
var Module = {
  onRuntimeInitialized: function () {
    const path = require('path');
    const assets = JSON.parse(process.env.PW153_PROBE_FILES || '[]');
    for (const asset of assets) {
      const filename = typeof asset === 'string' ? asset : asset.source;
      const name = typeof asset === 'string' ? path.basename(filename) : asset.virtualName;
      if (typeof name !== 'string' || !/^[a-z0-9_-]+(?:\\[a-z0-9_-]+)*\.(pck|pkx)$/i.test(name))
        throw new Error('Invalid package name');
      pw153MapReadonlyFile(filename, name);
    }
    console.error('PW_ENTRY_PROBE_BEGIN');
    const empty = Module._malloc(2);
    HEAPU16[empty >> 1] = 0;
    try {
      const result = Module.__Z8wWinMainPvS_Pwi(0, 0, empty, 0);
      console.error('PW_ENTRY_PROBE_RETURN ' + result);
      process.exitCode = result === 0 ? 0 : 1;
      // Startup failure may leave the event-loop poll timer alive. Let queued
      // diagnostics flush, then terminate this Node-only probe with failure.
      if (result !== 0) setTimeout(() => process.exit(1), 50);
    } catch (error) {
      console.error('PW_ENTRY_PROBE_EXCEPTION ' + (error && error.stack || error));
      process.exitCode = 1;
    }
  }
};
