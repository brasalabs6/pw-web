var Module = {
  preRun: [function () { pw153MapReadonlyFile(process.env.PW153_FIXTURE, 'fixture.pck'); }],
  onExit: function () {
    if (require('fs').readFileSync(process.env.PW153_FIXTURE, 'utf8') !== 'abcdefgh')
      throw new Error('Read-only source was modified');
  }
};
