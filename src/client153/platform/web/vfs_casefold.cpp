/*
 * FILE: vfs_casefold.cpp (platform seam)
 *
 * DESCRIPTION: Installs the case-insensitive VFS lookup the original
 * client ran on. The Windows file system the game was built for resolves
 * "Maps\Login\BSData\1.dat" and "maps\login\bsdata\1.dat" to the same
 * file; the browser MEMFS is case-sensitive and the shipped asset tree's
 * on-disk casing does not always match the game's path constants (the
 * login scene's BSData directory). The hook retries a failed lookup with
 * a case-folded walk of the MEMFS tree, so every file open, stat and
 * access through Emscripten's FS keeps the original semantic.
 *
 * Copyright (c) 2001 Archosaur Studio, All Rights Reserved.
 */

#include <emscripten.h>

extern "C" void pw153_vfs_install_casefold(void) {
	// clang-format off
	EM_ASM({
		if (FS.__pw153Casefold) return;
		FS.__pw153Casefold = true;
		const origLookup = FS.lookupPath;
		// Walks the MEMFS tree case-insensitively. A missing LAST
		// component is tolerated only for parent lookups (the caller
		// wants the directory that will hold it), keeping the caller's
		// original spelling for the leaf; every other miss fails the
		// fold so the original error propagates.
		const fold = (path, opts) => {
			const parts = path.split('/');
			let node = FS.root;
			let built = '';
			for (let i = 0; i < parts.length; ++i) {
				const part = parts[i];
				if (part === '' || part === '.') continue;
				if (part === '..') {
					node = node.parent;
					const cut = built.lastIndexOf('/');
					built = cut > 0 ? built.slice(0, cut) : '';
					continue;
				}
				const isLast = i === parts.length - 1;
				if (!node.isFolder) return null;
				let hit = null;
				for (const name in node.contents) {
					if (name === part) { hit = name; break; }
				}
				if (hit === null) {
					const lower = part.toLowerCase();
					for (const name in node.contents) {
						if (name.toLowerCase() === lower) { hit = name; break; }
					}
				}
				if (hit === null) {
					if (isLast && opts && opts.parent) {
						// The parent exists case-folded; the leaf is the
						// name the caller will create.
						return built + '/' + part;
					}
					return null;
				}
				node = node.contents[hit];
				built += '/' + hit;
			}
			return built || '/';
		};
		const retryFolded = (path, opts) => {
			const folded = fold(path, opts);
			if (folded !== null && folded !== path) {
				console.log('[pw153] vfs casefold: ' + path + ' -> ' + folded);
				// The caller's lookup semantics (follow, parent,
				// noent_okay) apply to the folded path unchanged.
				return origLookup.call(FS, folded, opts);
			}
			return undefined;
		};
		FS.lookupPath = function(path, opts) {
			try {
				const result = origLookup.call(FS, path, opts);
				// The noent_okay callers get a node-less result for a
				// last-component miss instead of a throw.
				if (result && result.node === undefined && typeof path === 'string') {
					const folded = retryFolded(path, opts);
					if (folded !== undefined) return folded;
				}
				return result;
			} catch (e) {
				// This glue's ErrnoError carries the numeric errno (44 =
				// ENOENT); a mid-path miss throws even with noent_okay.
				if (e && (e.errno === 44 || e.code === 'ENOENT') &&
					typeof path === 'string') {
					const folded = retryFolded(path, opts);
					if (folded !== undefined) return folded;
				}
				throw e;
			}
		};
	});
	// clang-format on
}
