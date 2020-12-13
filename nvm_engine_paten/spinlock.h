#ifndef TAIR_CONTEST_KV_CONTEST_SPINLOCK_H_
#define TAIR_CONTEST_KV_CONTEST_SPINLOCK_H_

#include <atomic>
#include <mutex>

class SpinLock {
  public:
    void Lock() {
      while(lck.test_and_set(std::memory_order_acquire)){}
    }
    
    void Unlock() {
      lck.clear(std::memory_order_release);
    }
             
  private:
    std::atomic_flag lck = ATOMIC_FLAG_INIT;
};

#endif // TAIR_CONTEST_KV_CONTEST_SPINLOCK_H_