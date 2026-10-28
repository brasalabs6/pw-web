// The Win32 synchronization seam's contracts (winsync.h), per the review
// issues PW23-001 (thread handle close vs running thread state) and
// PW23-002 (mutex ownership, recursion and INFINITE).
#include "Windows.h"

#include <atomic>
#include <cstdio>
#include <stdexcept>
#include <thread>
#include <vector>

#define CHECK(condition) do { if (!(condition)) throw std::runtime_error(#condition); } while (false)

// PW23-002: the mutex tracks the owning thread, allows recursive
// acquisition, rejects foreign release, and honors 0/positive/INFINITE
// timeouts.
static void TestMutexOwnership() {
  HANDLE mutex = CreateMutex(nullptr, FALSE, nullptr);
  CHECK(mutex != nullptr);
  CHECK(WaitForSingleObject(mutex, INFINITE) == WAIT_OBJECT_0);
  // Recursive acquisition by the owner succeeds immediately.
  CHECK(WaitForSingleObject(mutex, 0) == WAIT_OBJECT_0);
  CHECK(WaitForSingleObject(mutex, 0) == WAIT_OBJECT_0);
  // A foreign thread cannot release the mutex.
  std::atomic<BOOL> foreignRelease{FALSE};
  std::thread foreign([&] { foreignRelease = ReleaseMutex(mutex); });
  foreign.join();
  CHECK(foreignRelease.load() == FALSE);
  // Partial release: the owner still holds it after two of three levels.
  CHECK(ReleaseMutex(mutex));
  CHECK(ReleaseMutex(mutex));
  std::atomic<DWORD> foreignWait{WAIT_FAILED};
  std::thread waiter([&] {
    foreignWait = WaitForSingleObject(mutex, 0);
  });
  waiter.join();
  CHECK(foreignWait.load() == WAIT_TIMEOUT);
  // The last level releases and wakes the waiters.
  CHECK(ReleaseMutex(mutex));
  CHECK(ReleaseMutex(mutex) == FALSE);  // not owned anymore
  CHECK(CloseHandle(mutex));
}

static void TestMutexInitialOwner() {
  // CreateMutex(initialOwner=TRUE): the creating thread owns it.
  HANDLE mutex = CreateMutex(nullptr, TRUE, nullptr);
  CHECK(mutex != nullptr);
  CHECK(WaitForSingleObject(mutex, 0) == WAIT_OBJECT_0);  // recursive
  std::atomic<DWORD> other{WAIT_FAILED};
  std::thread otherThread([&] { other = WaitForSingleObject(mutex, 0); });
  otherThread.join();
  CHECK(other.load() == WAIT_TIMEOUT);
  CHECK(ReleaseMutex(mutex));
  CHECK(CloseHandle(mutex));
}

static void TestMutexInfiniteWait() {
  // A held mutex released by the owner unblocks an INFINITE waiter.
  HANDLE mutex = CreateMutex(nullptr, FALSE, nullptr);
  CHECK(WaitForSingleObject(mutex, INFINITE) == WAIT_OBJECT_0);
  std::atomic<bool> acquired{false};
  std::thread waiter([&] {
    if (WaitForSingleObject(mutex, INFINITE) == WAIT_OBJECT_0)
      acquired = true;
  });
  std::this_thread::sleep_for(std::chrono::milliseconds(50));
  CHECK(ReleaseMutex(mutex));
  waiter.join();
  CHECK(acquired.load());
  CHECK(CloseHandle(mutex));
}

// PW23-001: closing a running thread's handle must not destroy the state
// the thread still uses; the thread completes cleanly (ASan/UBSan would
// report any use-after-free).
static void TestThreadCloseWhileRunning() {
  std::atomic<bool> completed{false};
  HANDLE thread = CreateThread(
      nullptr, 0,
      [](void *argument) -> unsigned long {
        auto *flag = static_cast<std::atomic<bool> *>(argument);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        flag->store(true);  // written after the handle is closed
        return 7;
      },
      &completed, 0, nullptr);
  CHECK(thread != nullptr);
  CHECK(CloseHandle(thread));  // close while the thread runs
  std::this_thread::sleep_for(std::chrono::milliseconds(300));
  CHECK(completed.load());
}

static void TestThreadWaitAndExitCode() {
  std::atomic<int> value{0};
  HANDLE thread = CreateThread(
      nullptr, 0,
      [](void *argument) -> unsigned long {
        auto *out = static_cast<std::atomic<int> *>(argument);
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        out->store(42);
        return 42;
      },
      &value, 0, nullptr);
  DWORD code = 0;
  CHECK(GetExitCodeThread(thread, &code));
  CHECK(code == STILL_ACTIVE);
  // Multiple waiters on the same handle all observe completion.
  std::vector<std::thread> waiters;
  std::atomic<int> waitResults{0};
  for (int i = 0; i < 4; ++i)
    waiters.emplace_back([&] {
      if (WaitForSingleObject(thread, INFINITE) == WAIT_OBJECT_0)
        waitResults.fetch_add(1);
    });
  for (std::thread &waiter : waiters)
    waiter.join();
  CHECK(waitResults.load() == 4);
  CHECK(value.load() == 42);
  CHECK(GetExitCodeThread(thread, &code));
  CHECK(code == 42);
  CHECK(CloseHandle(thread));  // close after completion
}

static void TestThreadSuspendedAndExitThread() {
  // CREATE_SUSPENDED defers the launch to the first ResumeThread.
  std::atomic<bool> ran{false};
  HANDLE thread = CreateThread(
      nullptr, 0,
      [](void *argument) -> unsigned long {
        static_cast<std::atomic<bool> *>(argument)->store(true);
        ExitThread(42);
      },
      &ran, CREATE_SUSPENDED, nullptr);
  CHECK(thread != nullptr);
  DWORD code = 0;
  CHECK(GetExitCodeThread(thread, &code));
  CHECK(code == STILL_ACTIVE);  // not launched yet
  CHECK(ResumeThread(thread) == 1);
  CHECK(WaitForSingleObject(thread, INFINITE) == WAIT_OBJECT_0);
  CHECK(ran.load());
  CHECK(GetExitCodeThread(thread, &code));
  CHECK(code == 42);  // ExitThread's code
  CHECK(CloseHandle(thread));
}

static void TestBeginThreadExSuspended() {
  std::atomic<bool> ran{false};
  auto start = +[](void *arg) -> unsigned {
    static_cast<std::atomic<bool> *>(arg)->store(true);
    return 17;
  };
  HANDLE thread = _beginthreadex(nullptr, 0, start, &ran, CREATE_SUSPENDED, nullptr);
  CHECK(thread != nullptr);
  CHECK(WaitForSingleObject(thread, 100) == WAIT_TIMEOUT);
  CHECK(!ran.load());
  CHECK(ResumeThread(thread) == 1);
  CHECK(WaitForSingleObject(thread, INFINITE) == WAIT_OBJECT_0);
  CHECK(ran.load());
  DWORD code = 0;
  CHECK(GetExitCodeThread(thread, &code) && code == 17);
  CHECK(CloseHandle(thread));
}

static void TestEventAndSemaphoreTimeouts() {
  HANDLE event = CreateEvent(nullptr, TRUE, FALSE, nullptr);
  CHECK(WaitForSingleObject(event, 0) == WAIT_TIMEOUT);
  CHECK(WaitForSingleObject(event, 30) == WAIT_TIMEOUT);
  CHECK(SetEvent(event));
  CHECK(WaitForSingleObject(event, 0) == WAIT_OBJECT_0);
  CHECK(CloseHandle(event));
  HANDLE semaphore = CreateSemaphore(nullptr, 1, 2, nullptr);
  CHECK(WaitForSingleObject(semaphore, 0) == WAIT_OBJECT_0);
  CHECK(WaitForSingleObject(semaphore, 0) == WAIT_TIMEOUT);
  CHECK(ReleaseSemaphore(semaphore, 1, nullptr));
  CHECK(WaitForSingleObject(semaphore, INFINITE) == WAIT_OBJECT_0);
  CHECK(CloseHandle(semaphore));
}

int main() {
  TestMutexOwnership();
  TestMutexInitialOwner();
  TestMutexInfiniteWait();
  TestThreadCloseWhileRunning();
  TestThreadWaitAndExitCode();
  TestThreadSuspendedAndExitThread();
  TestBeginThreadExSuspended();
  TestEventAndSemaphoreTimeouts();
  std::printf("winsync_semantics: all checks passed\n");
  return 0;
}
