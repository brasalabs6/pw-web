#include "winsock2.h"
#include "gnthread.h"
#include <thread>

int main() {
  Thread::Mutex mutex;
  Thread::Condition condition;
  mutex.Lock();
  condition.Wait(mutex);
  condition.NotifyOne();
  condition.Wait(mutex);
  mutex.UNLock();
  Thread::Semaphore semaphore;
  std::thread notifier([&semaphore] { semaphore.NotifyOne(); });
  semaphore.Wait(mutex);
  notifier.join();
}
