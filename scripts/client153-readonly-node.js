// Map local files as seekable, read-only MEMFS nodes without copying their contents.
// Used only by the Node diagnostic; browser asset streaming has a separate owner.
function pw153MapReadonlyFile(source, destination) {
  const hostFs = require('fs');
  const size = hostFs.statSync(source).size;
  if (!Number.isSafeInteger(size) || size < 0) throw new Error('Invalid source size');
  FS.createDataFile('/', destination, null, true, false);
  const node = FS.lookupPath('/' + destination).node;
  node.usedBytes = size;
  node.stream_ops = {
    open(stream) {
      // POSIX access mode and creation/truncation/append are all disallowed.
      if (stream.flags & (3 | 64 | 512 | 1024)) throw new FS.ErrnoError(69);
      stream.pw153fd = hostFs.openSync(source, 'r');
    },
    close(stream) { hostFs.closeSync(stream.pw153fd); },
    read(stream, buffer, offset, length, position) {
      try { return hostFs.readSync(stream.pw153fd, buffer, offset, length, position); }
      catch (_) { throw new FS.ErrnoError(29); }
    },
    write() { throw new FS.ErrnoError(69); },
    llseek(stream, offset, whence) {
      const position = offset + (whence === 1 ? stream.position : whence === 2 ? size : 0);
      if (whence < 0 || whence > 2 || position < 0 || !Number.isSafeInteger(position))
        throw new FS.ErrnoError(28);
      return position;
    }
  };
}
