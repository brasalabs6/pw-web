/*
 * FILE: winsync.h (platform seam)
 *
 * DESCRIPTION: The Win32 synchronization primitives the client uses, with
 * real implementations: events are condition-variable backed flags with
 * auto/manual reset semantics, semaphores are real counting semaphores,
 * threads are std::threads with a completion event for timed joins, and
 * WaitForSingleObject implements the real wait semantics (WAIT_OBJECT_0 on
 * signal, WAIT_TIMEOUT on timeout). Handles are pointers to these objects;
 * CloseHandle releases them.
 *
 * Copyright (c) 2001 Archosaur Studio, All Rights Reserved.
 */

#ifndef PW153_PLATFORM_WINSYNC_H
#define PW153_PLATFORM_WINSYNC_H

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstring>
#include <functional>
#include <mutex>
#include <thread>

namespace pw153::platform {

struct SyncObject {
  enum class Kind { Event, Semaphore, Thread, Mutex };
  explicit SyncObject(Kind k) : kind(k) {}
  virtual ~SyncObject() = default;
  const Kind kind;
};

struct WinEvent : SyncObject {
  explicit WinEvent(bool manual, bool initial)
      : SyncObject(Kind::Event), manualReset(manual), signaled(initial) {}
  std::mutex mutex;
  std::condition_variable cv;
  const bool manualReset;
  bool signaled;
};

struct WinSemaphore : SyncObject {
  explicit WinSemaphore(LONG initial, LONG maximum)
      : SyncObject(Kind::Semaphore), count(initial), maximum(maximum) {}
  std::mutex mutex;
  std::condition_variable cv;
  LONG count;
  const LONG maximum;
};

struct WinThread : SyncObject {
  explicit WinThread() : SyncObject(Kind::Thread), done(false) {}
  std::thread thread;
  std::mutex mutex;
  std::condition_variable cv;
  bool done;
  long exitCode = 0;
  std::function<void()> pending;	//	CREATE_SUSPENDED threads launch on the first resume.
};

inline void SignalEvent(WinEvent *event) {
  std::lock_guard<std::mutex> lock(event->mutex);
  event->signaled = true;
  event->cv.notify_all();
}

inline void ResetEventState(WinEvent *event) {
  std::lock_guard<std::mutex> lock(event->mutex);
  event->signaled = false;
}

struct WinMutex : SyncObject {
  explicit WinMutex(bool initialOwned)
      : SyncObject(Kind::Mutex), owned(initialOwned) {}
  std::mutex mutex;
  std::condition_variable cv;
  bool owned;
};

//	ExitThread's unwind token: the thread entry wrappers catch it to
//	end the thread at the documented exit code.
struct ThreadExit {
  DWORD code;
};

} // namespace pw153::platform

//	Events: real condition-variable backed flags.
inline HANDLE CreateEventW(void *attributes, BOOL manualReset,
                           BOOL initialState, const char *name) {
  (void)attributes;
  (void)name;
  return new pw153::platform::WinEvent(manualReset != 0, initialState != 0);
}

inline HANDLE CreateEventA(void *attributes, BOOL manualReset,
                           BOOL initialState, const char *name) {
  return CreateEventW(attributes, manualReset, initialState, name);
}

inline HANDLE CreateEvent(void *attributes, BOOL manualReset,
                          BOOL initialState, const char *name) {
  return CreateEventW(attributes, manualReset, initialState, name);
}

inline BOOL SetEvent(HANDLE handle) {
  auto *event = static_cast<pw153::platform::WinEvent *>(handle);
  if (!event)
    return FALSE;
  pw153::platform::SignalEvent(event);
  return TRUE;
}

inline BOOL ResetEvent(HANDLE handle) {
  auto *event = static_cast<pw153::platform::WinEvent *>(handle);
  if (!event)
    return FALSE;
  pw153::platform::ResetEventState(event);
  return TRUE;
}

inline HANDLE CreateMutexW(void *attributes, BOOL initialOwner,
                          const wchar_t *name) {
  (void)name;
  (void)attributes;
  return new pw153::platform::WinMutex(initialOwner != 0);
}

inline HANDLE CreateMutexA(void *attributes, BOOL initialOwner,
                          const char * /*name*/) {
  return CreateMutexW(attributes, initialOwner, nullptr);
}

inline BOOL ReleaseMutex(HANDLE handle) {
  auto *mutex = static_cast<pw153::platform::WinMutex *>(handle);
  if (!mutex)
    return FALSE;
  std::lock_guard<std::mutex> lock(mutex->mutex);
  if (!mutex->owned)
    return FALSE;
  mutex->owned = false;
  mutex->cv.notify_all();
  return TRUE;
}

//	Semaphores: real counting semaphores.
inline HANDLE CreateSemaphoreW(void *attributes, LONG initial, LONG maximum,
                               const char *name) {
  (void)attributes;
  (void)name;
  return new pw153::platform::WinSemaphore(initial, maximum);
}

inline BOOL ReleaseSemaphore(HANDLE handle, LONG count, LONG *previous) {
  auto *semaphore = static_cast<pw153::platform::WinSemaphore *>(handle);
  if (!semaphore)
    return FALSE;
  std::lock_guard<std::mutex> lock(semaphore->mutex);
  if (previous)
    *previous = semaphore->count;
  const LONG target = semaphore->count + count;
  if (target > semaphore->maximum)
    return FALSE;
  semaphore->count = target;
  semaphore->cv.notify_all();
  return TRUE;
}

//	Threads: std::thread with a completion event for the timed-join wait.
#if defined(_MSC_VER)
typedef unsigned(__stdcall *ThreadStart)(void *);
#else
typedef unsigned (*ThreadStart)(void *);
#endif

inline HANDLE _beginthreadex(void *security, unsigned stackSize,
                             ThreadStart start, void *argument,
                             unsigned flags, unsigned *threadId) {
  (void)security;
  (void)stackSize;
  (void)flags;
  auto *wrapped = new pw153::platform::WinThread();
  struct Argument {
    ThreadStart start;
    void *argument;
    pw153::platform::WinThread *wrapped;
  };
  auto *state = new Argument{start, argument, wrapped};
  wrapped->thread = std::thread([state]() {
    long exitCode = 0;
    try {
      exitCode = static_cast<long>(state->start(state->argument));
    } catch (const pw153::platform::ThreadExit &ex) {
      exitCode = static_cast<long>(ex.code);
    }
    std::lock_guard<std::mutex> lock(state->wrapped->mutex);
    state->wrapped->exitCode = exitCode;
    state->wrapped->done = true;
    state->wrapped->cv.notify_all();
    delete state;
  });
  if (threadId)
    *threadId = 0; //\tThread ids are not addressable in this backend.
  return wrapped;
}

//	WaitForSingleObject: real wait semantics over the tagged objects.
inline DWORD WaitForSingleObject(HANDLE handle, DWORD milliseconds) {
  auto *object = static_cast<pw153::platform::SyncObject *>(handle);
  if (!object)
    return WAIT_FAILED;
  const auto timeout = std::chrono::milliseconds(milliseconds);
  if (object->kind == pw153::platform::SyncObject::Kind::Event) {
    auto *event = static_cast<pw153::platform::WinEvent *>(object);
    std::unique_lock<std::mutex> lock(event->mutex);
    if (event->manualReset) {
      if (!event->cv.wait_for(lock, timeout,
                              [&] { return event->signaled; }))
        return WAIT_TIMEOUT;
      return WAIT_OBJECT_0;
    }
    //\tAuto-reset: one waiter consumes the signal.
    if (!event->cv.wait_for(lock, timeout, [&] { return event->signaled; }))
      return WAIT_TIMEOUT;
    event->signaled = false;
    return WAIT_OBJECT_0;
  }
  if (object->kind == pw153::platform::SyncObject::Kind::Semaphore) {
    auto *semaphore = static_cast<pw153::platform::WinSemaphore *>(object);
    std::unique_lock<std::mutex> lock(semaphore->mutex);
    if (!semaphore->cv.wait_for(
            lock, timeout, [&] { return semaphore->count > 0; }))
      return WAIT_TIMEOUT;
    --semaphore->count;
    return WAIT_OBJECT_0;
  }
  if (object->kind == pw153::platform::SyncObject::Kind::Mutex) {
    auto *mutexObject = static_cast<pw153::platform::WinMutex *>(object);
    std::unique_lock<std::mutex> lock(mutexObject->mutex);
    if (!mutexObject->cv.wait_for(lock, timeout, [&] { return !mutexObject->owned; }))
      return WAIT_TIMEOUT;
    mutexObject->owned = true;
    return WAIT_OBJECT_0;
  }
  auto *thread = static_cast<pw153::platform::WinThread *>(object);
  {
    std::unique_lock<std::mutex> lock(thread->mutex);
    if (!thread->cv.wait_for(lock, timeout, [&] { return thread->done; }))
      return WAIT_TIMEOUT;
  }
  if (thread->thread.joinable())
    thread->thread.join();
  return WAIT_OBJECT_0;
}

//	Non-consuming poll over the tagged sync objects: the array wait below
//	uses it so an auto-reset signal is not consumed while other handles in
//	the array are still pending.
inline bool PollSignaled(HANDLE handle) {
  auto *object = static_cast<pw153::platform::SyncObject *>(handle);
  if (!object)
    return false;
  switch (object->kind) {
  case pw153::platform::SyncObject::Kind::Event: {
    auto *event = static_cast<pw153::platform::WinEvent *>(object);
    std::lock_guard<std::mutex> lock(event->mutex);
    return event->signaled;
  }
  case pw153::platform::SyncObject::Kind::Semaphore: {
    auto *semaphore = static_cast<pw153::platform::WinSemaphore *>(object);
    std::lock_guard<std::mutex> lock(semaphore->mutex);
    return semaphore->count > 0;
  }
  case pw153::platform::SyncObject::Kind::Mutex: {
    auto *mutexObject = static_cast<pw153::platform::WinMutex *>(object);
    std::lock_guard<std::mutex> lock(mutexObject->mutex);
    return !mutexObject->owned;
  }
  case pw153::platform::SyncObject::Kind::Thread: {
    auto *thread = static_cast<pw153::platform::WinThread *>(object);
    std::lock_guard<std::mutex> lock(thread->mutex);
    return thread->done;
  }
  }
  return false;
}

//	The multi-object wait: polls the array without consuming, then consumes
//	through the single-object wait on the reported handle. waitAll reports
//	WAIT_OBJECT_0 only when every handle is signaled.
inline DWORD WaitForMultipleObjects(DWORD count, const HANDLE *handles,
                                   BOOL waitAll, DWORD milliseconds) {
  if (!handles || count == 0)
    return WAIT_FAILED;
  const DWORD slice =
      milliseconds == INFINITE ? 100 : (milliseconds / count + 1);
  DWORD waited = 0;
  while (milliseconds == INFINITE || waited < milliseconds) {
    if (waitAll) {
      bool all = true;
      for (DWORD i = 0; i < count; ++i)
        if (!PollSignaled(handles[i])) {
          all = false;
          break;
        }
      if (all) {
        for (DWORD i = 0; i < count; ++i)
          WaitForSingleObject(handles[i], 0);
        return WAIT_OBJECT_0;
      }
    } else {
      for (DWORD i = 0; i < count; ++i)
        if (PollSignaled(handles[i])) {
          WaitForSingleObject(handles[i], 0);
          return WAIT_OBJECT_0 + i;
        }
    }
    Sleep(slice);
    waited += slice;
  }
  return WAIT_TIMEOUT;
}

//	The thread exit code: the wrapper thread records the start routine's
//	return; a live thread reports STILL_ACTIVE (the Win32 contract).
#define STILL_ACTIVE 259

inline BOOL GetExitCodeThread(HANDLE handle, DWORD *code) {
  auto *object = static_cast<pw153::platform::SyncObject *>(handle);
  if (!object || !code ||
      object->kind != pw153::platform::SyncObject::Kind::Thread)
    return FALSE;
  auto *thread = static_cast<pw153::platform::WinThread *>(object);
  std::lock_guard<std::mutex> lock(thread->mutex);
  *code = thread->done ? static_cast<DWORD>(thread->exitCode) : STILL_ACTIVE;
  return TRUE;
}

//	The thread id: a real distinct id per thread (the OS id value is
//	opaque; the port issues stable per-thread ids).
inline DWORD GetCurrentThreadId() {
  static std::atomic<DWORD> issued(0);
  thread_local const DWORD currentId = ++issued;
  return currentId;
}

//	The thread priorities (the documented Win32 values; the browser
//	target has no scheduler priorities).
#define THREAD_PRIORITY_IDLE (-15)
#define THREAD_PRIORITY_LOWEST (-2)
#define THREAD_PRIORITY_BELOW_NORMAL (-1)
#define THREAD_PRIORITY_NORMAL 0
#define THREAD_PRIORITY_ABOVE_NORMAL 1
#define THREAD_PRIORITY_HIGHEST 2
#define THREAD_PRIORITY_TIME_CRITICAL 15

//	The thread suspension: CREATE_SUSPENDED is representable (the thread
//	launches on the first resume); mid-flight suspend is not, so
//	SuspendThread on a running thread fails with (DWORD)-1. The
//	render-loop mapping (m-boot) owns the real pause path.
#define CREATE_SUSPENDED 0x00000004

inline DWORD SuspendThread(HANDLE handle) {
  (void)handle;
  return static_cast<DWORD>(-1);
}
inline DWORD ResumeThread(HANDLE handle) {
  auto *object = static_cast<pw153::platform::SyncObject *>(handle);
  if (!object || object->kind != pw153::platform::SyncObject::Kind::Thread)
    return static_cast<DWORD>(-1);
  auto *thread = static_cast<pw153::platform::WinThread *>(object);
  std::lock_guard<std::mutex> lock(thread->mutex);
  if (thread->pending) {
    auto run = std::move(thread->pending);
    thread->pending = nullptr;
    thread->thread = std::thread(std::move(run));
    return 1;	//	previous suspend count.
  }
  return 0;	//	Win32: resume of a running thread is a no-op returning 0.
}

//	The thread creation: the real std::thread-backed object; the
//	CREATE_SUSPENDED flag defers the launch to the first ResumeThread.
//	The start routine is a template parameter so the game's thread
//	functions (declared both `DWORD WINAPI` and `unsigned long WINAPI`)
//	all bind without a return-type mismatch.
template <class Start>
inline HANDLE CreateThread(void *security, unsigned stackSize, Start start,
                           void *argument, DWORD creationFlags,
                           DWORD *threadId) {
  (void)security;
  (void)stackSize;
  auto *wrapped = new pw153::platform::WinThread();
  struct Argument {
    Start start;
    void *argument;
    pw153::platform::WinThread *wrapped;
  };
  auto *state = new Argument{std::move(start), argument, wrapped};
  auto body = [state]() {
    long exitCode = 0;
    try {
      exitCode = static_cast<long>(state->start(state->argument));
    } catch (const pw153::platform::ThreadExit &ex) {
      exitCode = static_cast<long>(ex.code);
    }
    std::lock_guard<std::mutex> lock(state->wrapped->mutex);
    state->wrapped->exitCode = exitCode;
    state->wrapped->done = true;
    state->wrapped->cv.notify_all();
    delete state;
  };
  if (creationFlags & CREATE_SUSPENDED) {
    std::lock_guard<std::mutex> lock(wrapped->mutex);
    wrapped->pending = std::move(body);
  } else {
    wrapped->thread = std::thread(std::move(body));
  }
  if (threadId)
    *threadId = 0;
  return wrapped;
}

//	ExitThread: terminates the calling thread by unwinding to the entry
//	wrapper (which catches the token). A surrounding catch(...) in the
//	game would swallow it; the browser-port paths that call ExitThread
//	are not wrapped that way.
[[noreturn]] inline void ExitThread(DWORD exitCode) {
  throw pw153::platform::ThreadExit{exitCode};
}

//	The thread priority: no scheduler priorities exist on the browser
//	target; the call fails explicitly.
inline BOOL SetThreadPriority(HANDLE handle, int priority) {
  (void)handle;
  (void)priority;
  SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
  return FALSE;
}

inline BOOL CloseHandle(HANDLE handle) {
  auto *object = static_cast<pw153::platform::SyncObject *>(handle);
  if (!object)
    return FALSE;
  switch (object->kind) {
  case pw153::platform::SyncObject::Kind::Event:
    delete static_cast<pw153::platform::WinEvent *>(object);
    return TRUE;
  case pw153::platform::SyncObject::Kind::Semaphore:
    delete static_cast<pw153::platform::WinSemaphore *>(object);
    return TRUE;
  case pw153::platform::SyncObject::Kind::Mutex:
    delete static_cast<pw153::platform::WinMutex *>(object);
    return TRUE;
  case pw153::platform::SyncObject::Kind::Thread: {
    auto *thread = static_cast<pw153::platform::WinThread *>(object);
    if (thread->thread.joinable()) {
      //\tClosing a running thread handle detaches it (Win32 semantics).
      thread->thread.detach();
    }
    delete thread;
    return TRUE;
  }
  }
  return FALSE;
}

#endif // PW153_PLATFORM_WINSYNC_H

#ifndef CreateMutex
#define CreateMutex(attributes, initialOwner, name) \
  CreateMutexW(attributes, initialOwner, name)
#endif

//	The legacy thread entry: the same real thread object as _beginthreadex
//	with the void-returning start signature.
inline HANDLE _beginthread(void (*start)(void *), unsigned stackSize,
                           void *argument) {
  (void)stackSize;
  auto *wrapped = new pw153::platform::WinThread();
  struct Argument { void (*start)(void *); void *argument; pw153::platform::WinThread *wrapped; };
  auto *state = new Argument{start, argument, wrapped};
  wrapped->thread = std::thread([state]() {
    long exitCode = 0;
    try {
      state->start(state->argument);
    } catch (const pw153::platform::ThreadExit &ex) {
      exitCode = static_cast<long>(ex.code);
    }
    std::lock_guard<std::mutex> lock(state->wrapped->mutex);
    state->wrapped->exitCode = exitCode;
    state->wrapped->done = true;
    state->wrapped->cv.notify_all();
    delete state;
  });
  return wrapped;
}
