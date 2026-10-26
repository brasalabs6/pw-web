// Browser/native C++ platform backend. No Win32 emulation beyond this seam:
// CRITICAL_SECTION maps to a real std::mutex with the Win32 API surface the
// original sources call (Initialize/Delete/Enter/Leave). The lock is real;
// single-threaded runtimes serialize trivially.
#ifndef PW153_PLATFORM_CRITICALSECTION_H
#define PW153_PLATFORM_CRITICALSECTION_H

#include <mutex>

struct CRITICAL_SECTION {
  std::mutex mutex;
};

inline void InitializeCriticalSection(CRITICAL_SECTION *pcs) {
  //	std::mutex is default-constructed ready; nothing to do.
  (void)pcs;
}

inline void DeleteCriticalSection(CRITICAL_SECTION *pcs) {
  //	std::mutex releases on destruction; nothing to do.
  (void)pcs;
}

inline void EnterCriticalSection(CRITICAL_SECTION *pcs) { pcs->mutex.lock(); }

inline void LeaveCriticalSection(CRITICAL_SECTION *pcs) { pcs->mutex.unlock(); }

#endif // PW153_PLATFORM_CRITICALSECTION_H
