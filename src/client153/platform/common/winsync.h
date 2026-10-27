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

#include <chrono>
#include <condition_variable>
#include <cstring>
#include <mutex>
#include <thread>

namespace pw153::platform {

struct SyncObject {
  enum class Kind { Event, Semaphore, Thread };
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

} // namespace pw153::platform

//	Events: real condition-variable backed flags.
inline HANDLE CreateEventW(void *attributes, BOOL manualReset,
                           BOOL initialState, const char *name) {
  (void)attributes;
  (void)name;
  return new pw153::platform::WinEvent(manualReset != 0, initialState != 0);
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
    unsigned exitCode = state->start(state->argument);
    (void)exitCode;
    std::lock_guard<std::mutex> lock(state->wrapped->mutex);
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
