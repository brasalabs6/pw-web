// Node-only diagnostic. Assets are copied into MEMFS; source files are never mounted.
var Module = {
  onRuntimeInitialized: function () {
    const fs = require('fs');
    const path = require('path');
    const assets = JSON.parse(process.env.PW153_PROBE_FILES || '[]');
    for (const filename of assets) {
      const name = path.basename(filename);
      if (!/^[a-z0-9_-]+\.(pck|pkx)$/i.test(name)) throw new Error('Invalid package name');
      FS.writeFile('/' + name, fs.readFileSync(filename));
    }
    console.error('PW_ENTRY_PROBE_BEGIN');
    const empty = Module._malloc(2);
    HEAPU16[empty >> 1] = 0;
    try {
      const result = Module.__Z8wWinMainPvS_Pwi(0, 0, empty, 0);
      console.error('PW_ENTRY_PROBE_RETURN ' + result);
      process.exitCode = result === 0 ? 0 : 1;
    } catch (error) {
      console.error('PW_ENTRY_PROBE_EXCEPTION ' + (error && error.stack || error));
      process.exitCode = 1;
    }
  }
};
