/*
 * FILE: winsync.h (platform seam)
 *
 * DESCRIPTION: The Win32 synchronization primitives the client uses, with
 * real implementations: events are condition-variable backed flags with
 * auto/manual reset semantics, semaphores are real counting semaphores,
 * mutexes track the owning thread with recursive acquisition depth (the
 * Win32 contract), threads are std::threads whose runtime state is owned
 * jointly by the handle and the running entry (so closing a running
 * thread's handle cannot destroy the state the thread still uses), and
 * WaitForSingleObject implements the real wait semantics (WAIT_OBJECT_0 on
 * signal, WAIT_TIMEOUT on timeout, true infinite waits for INFINITE).
 * Handles are pointers to these objects; CloseHandle releases them.
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
#include <memory>
#include <mutex>
#include <thread>

//	The thread id: a real distinct id per thread (the OS id value is
//	opaque; the port issues stable per-thread ids). Defined before the
//	sync objects: the mutex tracks its owner by it.
inline DWORD GetCurrentThreadId() {
  static std::atomic<DWORD> issued(0);
  thread_local const DWORD currentId = ++issued;
  return currentId;
}

namespace pw153::platform {

struct SyncObject {
  enum class Kind { Event, Semaphore, Thread, Mutex };
  explicit SyncObject(Kind k) : kind(k) {}
  virtual ~SyncObject() = default;
  const Kind kind;
};

//	The thread's runtime state: owned jointly by the handle object and
//	the running entry lambda (shared_ptr), so CloseHandle of a running
//	thread detaches the std::thread and frees the handle while the
//	entry's state survives until the thread completes (PW23-001).
struct ThreadState {
  std::mutex mutex;
  std::condition_variable cv;
  bool done = false;
  bool joined = false;	//	one waiter performs the std::thread join.
  long exitCode = 0;
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
  explicit WinThread() : SyncObject(Kind::Thread), state(new ThreadState()) {}
  std::shared_ptr<ThreadState> state;
  std::thread thread;
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

//	The mutex tracks its owning thread and the recursive acquisition
//	depth (the Win32 contract: the owner reacquires freely, a foreign
//	release fails) (PW23-002).
struct WinMutex : SyncObject {
  explicit WinMutex(bool initialOwned)
      : SyncObject(Kind::Mutex), owned(false), depth(0), owner(0) {
    if (initialOwned) {
      owned = true;
      depth = 1;
      owner = ::GetCurrentThreadId();
    }
  }
  std::mutex mutex;
  std::condition_variable cv;
  bool owned;
  unsigned depth;
  DWORD owner;
};

//	ExitThread's unwind token: the thread entry wrappers catch it to
//	end the thread at the documented exit code.
struct ThreadExit {
  DWORD code;
};

//	The thread entry body shared by every creation form: runs the start
//	routine, records the exit code, marks the state done. The state is
//	owned via the shared_ptr, independent of the handle's lifetime.
template <class Run>
std::function<void()> MakeThreadBody(const std::shared_ptr<ThreadState> &state,
                                     Run &&run) {
  return [state, run = std::forward<Run>(run)]() {
    long exitCode = 0;
    try {
      exitCode = static_cast<long>(run());
    } catch (const ThreadExit &ex) {
      exitCode = static_cast<long>(ex.code);
    }
    std::lock_guard<std::mutex> lock(state->mutex);
    state->exitCode = exitCode;
    state->done = true;
    state->cv.notify_all();
  };
}

} // namespace pw153::platform

//	Events: real condition-variable backed flags. The name is not
//	addressable across processes in a browser target, so it is accepted
//	only for call-site compatibility; the UNICODE build binds the neutral
//	name to the wide form (the original game is a UNICODE build).
inline HANDLE CreateEventW(void *attributes, BOOL manualReset,
                           BOOL initialState, const wchar_t *name) {
  (void)attributes;
  (void)name;
  return new pw153::platform::WinEvent(manualReset != 0, initialState != 0);
}

inline HANDLE CreateEventA(void *attributes, BOOL manualReset,
                           BOOL initialState, const char *name) {
  (void)name;
  return CreateEventW(attributes, manualReset, initialState, nullptr);
}

inline HANDLE CreateEvent(void *attributes, BOOL manualReset,
                          BOOL initialState, const wchar_t *name) {
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

//	ReleaseMutex: only the owning thread releases; a recursive owner
//	releases one depth level and the last level wakes the waiters (the
//	Win32 contract).
inline BOOL ReleaseMutex(HANDLE handle) {
  auto *object = static_cast<pw153::platform::SyncObject *>(handle);
  if (!object || object->kind != pw153::platform::SyncObject::Kind::Mutex)
    return FALSE;
  auto *mutex = static_cast<pw153::platform::WinMutex *>(object);
  std::lock_guard<std::mutex> lock(mutex->mutex);
  if (!mutex->owned || mutex->owner != GetCurrentThreadId())
    return FALSE;
  if (--mutex->depth == 0) {
    mutex->owned = false;
    mutex->cv.notify_all();
  }
  return TRUE;
}

//	Semaphores: real counting semaphores.
inline HANDLE CreateSemaphoreW(void *attributes, LONG initial, LONG maximum,
                               const char *name) {
  (void)attributes;
  (void)name;
  return new pw153::platform::WinSemaphore(initial, maximum);
}

inline HANDLE CreateSemaphore(void *attributes, LONG initial, LONG maximum,
                              const char *name) {
  return CreateSemaphoreW(attributes, initial, maximum, name);
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

//	Threads: std::thread with a completion state for the timed-join wait.
#if defined(_MSC_VER)
typedef unsigned(__stdcall *ThreadStart)(void *);
#else
typedef unsigned (*ThreadStart)(void *);
#endif

#define CREATE_SUSPENDED 0x00000004

inline HANDLE _beginthreadex(void *security, unsigned stackSize,
                             ThreadStart start, void *argument,
                             unsigned flags, unsigned *threadId) {
  (void)security;
  (void)stackSize;
  auto *wrapped = new pw153::platform::WinThread();
  auto body = pw153::platform::MakeThreadBody(
      wrapped->state, [start, argument]() -> unsigned {
        return start(argument);
      });
  if (flags & CREATE_SUSPENDED) {
    std::lock_guard<std::mutex> lock(wrapped->state->mutex);
    wrapped->pending = std::move(body);
  } else {
    wrapped->thread = std::thread(std::move(body));
  }
  if (threadId)
    *threadId = 0; //\tThread ids are not addressable in this backend.
  return wrapped;
}

//	WaitForSingleObject: real wait semantics over the tagged objects;
//	INFINITE is a true unbounded wait (not a huge timeout).
inline DWORD WaitForSingleObject(HANDLE handle, DWORD milliseconds) {
  auto *object = static_cast<pw153::platform::SyncObject *>(handle);
  if (!object)
    return WAIT_FAILED;
  const bool infinite = milliseconds == INFINITE;
  const auto timeout = std::chrono::milliseconds(milliseconds);
  if (object->kind == pw153::platform::SyncObject::Kind::Event) {
    auto *event = static_cast<pw153::platform::WinEvent *>(object);
    std::unique_lock<std::mutex> lock(event->mutex);
    if (event->manualReset) {
      if (infinite) {
        event->cv.wait(lock, [&] { return event->signaled; });
      } else if (!event->cv.wait_for(lock, timeout,
                                     [&] { return event->signaled; })) {
        return WAIT_TIMEOUT;
      }
      return WAIT_OBJECT_0;
    }
    //\tAuto-reset: one waiter consumes the signal.
    if (infinite) {
      event->cv.wait(lock, [&] { return event->signaled; });
    } else if (!event->cv.wait_for(lock, timeout,
                                   [&] { return event->signaled; })) {
      return WAIT_TIMEOUT;
    }
    event->signaled = false;
    return WAIT_OBJECT_0;
  }
  if (object->kind == pw153::platform::SyncObject::Kind::Semaphore) {
    auto *semaphore = static_cast<pw153::platform::WinSemaphore *>(object);
    std::unique_lock<std::mutex> lock(semaphore->mutex);
    if (infinite) {
      semaphore->cv.wait(lock, [&] { return semaphore->count > 0; });
    } else if (!semaphore->cv.wait_for(
                   lock, timeout, [&] { return semaphore->count > 0; })) {
      return WAIT_TIMEOUT;
    }
    --semaphore->count;
    return WAIT_OBJECT_0;
  }
  if (object->kind == pw153::platform::SyncObject::Kind::Mutex) {
    auto *mutexObject = static_cast<pw153::platform::WinMutex *>(object);
    std::unique_lock<std::mutex> lock(mutexObject->mutex);
    const DWORD self = GetCurrentThreadId();
    //\tRecursive acquisition by the owner (the Win32 contract).
    if (mutexObject->owned && mutexObject->owner == self) {
      ++mutexObject->depth;
      return WAIT_OBJECT_0;
    }
    if (infinite) {
      mutexObject->cv.wait(lock, [&] { return !mutexObject->owned; });
    } else if (!mutexObject->cv.wait_for(
                   lock, timeout, [&] { return !mutexObject->owned; })) {
      return WAIT_TIMEOUT;
    }
    mutexObject->owned = true;
    mutexObject->owner = self;
    mutexObject->depth = 1;
    return WAIT_OBJECT_0;
  }
  auto *thread = static_cast<pw153::platform::WinThread *>(object);
  {
    std::unique_lock<std::mutex> lock(thread->state->mutex);
    if (infinite) {
      thread->state->cv.wait(lock, [&] { return thread->state->done; });
    } else if (!thread->state->cv.wait_for(
                   lock, timeout, [&] { return thread->state->done; })) {
      return WAIT_TIMEOUT;
    }
  }
  //	Exactly one waiter joins the std::thread (multiple waiters on the
  //	same handle all return WAIT_OBJECT_0; a concurrent join is UB).
  bool joinNeeded = false;
  {
    std::lock_guard<std::mutex> lock(thread->state->mutex);
    joinNeeded = !thread->state->joined;
    thread->state->joined = true;
  }
  if (joinNeeded && thread->thread.joinable())
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
    //	Acquirable by this thread: free, or owned by it (the recursive
    //	acquisition the consume path performs).
    return !mutexObject->owned ||
           mutexObject->owner == GetCurrentThreadId();
  }
  case pw153::platform::SyncObject::Kind::Thread: {
    auto *thread = static_cast<pw153::platform::WinThread *>(object);
    std::lock_guard<std::mutex> lock(thread->state->mutex);
    return thread->state->done;
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

//	SignalObjectAndWait: signals one object, then waits on another. The
//	signal completes (state change plus wake) before the wait begins, so
//	a rendezvous partner observes it even though the two steps are not
//	one atomic scheduler operation. Signaling covers the three signalable
//	kinds (event, semaphore, mutex); the wait side is the single-object
//	wait above. Alertable waits (APCs) do not exist on this target, so
//	bAlertable is accepted and ignored.
inline DWORD SignalObjectAndWait(HANDLE hObjectToSignal,
                                 HANDLE hObjectToWaitOn,
                                 DWORD dwMilliseconds, BOOL bAlertable) {
  (void)bAlertable;
  auto *signal = static_cast<pw153::platform::SyncObject *>(hObjectToSignal);
  if (!signal)
    return WAIT_FAILED;
  switch (signal->kind) {
  case pw153::platform::SyncObject::Kind::Event:
    if (!SetEvent(hObjectToSignal))
      return WAIT_FAILED;
    break;
  case pw153::platform::SyncObject::Kind::Semaphore:
    if (!ReleaseSemaphore(hObjectToSignal, 1, nullptr))
      return WAIT_FAILED;
    break;
  case pw153::platform::SyncObject::Kind::Mutex:
    if (!ReleaseMutex(hObjectToSignal))
      return WAIT_FAILED;
    break;
  default:
    //\tThreads are not signalable (the Win32 contract).
    return WAIT_FAILED;
  }
  return WaitForSingleObject(hObjectToWaitOn, dwMilliseconds);
}

//	The thread exit code: the entry records the start routine's return;
//	a live thread reports STILL_ACTIVE (the Win32 contract).
#define STILL_ACTIVE 259

inline BOOL GetExitCodeThread(HANDLE handle, DWORD *code) {
  auto *object = static_cast<pw153::platform::SyncObject *>(handle);
  if (!object || !code ||
      object->kind != pw153::platform::SyncObject::Kind::Thread)
    return FALSE;
  auto *thread = static_cast<pw153::platform::WinThread *>(object);
  std::lock_guard<std::mutex> lock(thread->state->mutex);
  *code = thread->state->done
              ? static_cast<DWORD>(thread->state->exitCode)
              : STILL_ACTIVE;
  return TRUE;
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
inline DWORD SuspendThread(HANDLE handle) {
  (void)handle;
  return static_cast<DWORD>(-1);
}
inline DWORD ResumeThread(HANDLE handle) {
  auto *object = static_cast<pw153::platform::SyncObject *>(handle);
  if (!object || object->kind != pw153::platform::SyncObject::Kind::Thread)
    return static_cast<DWORD>(-1);
  auto *thread = static_cast<pw153::platform::WinThread *>(object);
  std::lock_guard<std::mutex> lock(thread->state->mutex);
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
  auto body = pw153::platform::MakeThreadBody(
      wrapped->state,
      [start = std::move(start), argument]() -> unsigned long {
        return start(argument);
      });
  if (creationFlags & CREATE_SUSPENDED) {
    std::lock_guard<std::mutex> lock(wrapped->state->mutex);
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

//	The priority query rides on the same missing scheduler; it reports
//	the documented normal priority (the callers compare against it).
inline int GetThreadPriority(HANDLE handle) {
  (void)handle;
  return THREAD_PRIORITY_NORMAL;
}

//	The priority boost toggle rides on the scheduler priorities that do
//	not exist on this target; it fails like SetThreadPriority above.
inline BOOL SetThreadPriorityBoost(HANDLE handle, BOOL disable) {
  (void)handle;
  (void)disable;
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
      //\tClosing a running thread handle detaches it (Win32 semantics);
      //\tthe entry's shared state outlives the handle until the thread
      //\tcompletes (PW23-001).
      thread->thread.detach();
    }
    delete thread;
    return TRUE;
  }
  }
  return FALSE;
}

#ifndef CreateMutex
#define CreateMutex(attributes, initialOwner, name) \
  CreateMutexW(attributes, initialOwner, name)
#endif

//	The APC queue: the browser target has no Win32 APC queues (the
//	hosted threads are std::thread); the queue request fails and the
//	caller's error path handles it (the browser wrapper's deferred
//	window move/destroy stay unqueued).
typedef VOID(WINAPI *PAPCFUNC)(ULONG_PTR parameter);

inline BOOL QueueUserAPC(PAPCFUNC function, HANDLE thread,
                         ULONG_PTR parameter) {
  (void)function;
  (void)thread;
  (void)parameter;
  return FALSE;
}

//	The legacy thread entry: the same real thread object as _beginthreadex
//	with the void-returning start signature.
inline HANDLE _beginthread(void (*start)(void *), unsigned stackSize,
                           void *argument) {
  (void)stackSize;
  auto *wrapped = new pw153::platform::WinThread();
  auto body = pw153::platform::MakeThreadBody(
      wrapped->state, [start, argument]() -> unsigned {
        start(argument);
        return 0;
      });
  wrapped->thread = std::thread(std::move(body));
  return wrapped;
}

#endif // PW153_PLATFORM_WINSYNC_H
