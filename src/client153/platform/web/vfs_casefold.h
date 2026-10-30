/*
 * FILE: vfs_casefold.h (platform seam)
 *
 * DESCRIPTION: Installs the case-insensitive VFS lookup the original
 * client ran on (the Windows file system semantic; the browser MEMFS is
 * case-sensitive). See vfs_casefold.cpp.
 */

#ifndef PW153_PLATFORM_VFS_CASEFOLD_H
#define PW153_PLATFORM_VFS_CASEFOLD_H

#ifdef __EMSCRIPTEN__
extern "C" void pw153_vfs_install_casefold(void);
#else
inline void pw153_vfs_install_casefold(void) {}
#endif

#endif // PW153_PLATFORM_VFS_CASEFOLD_H
