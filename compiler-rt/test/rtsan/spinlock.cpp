// RUN: %clangxx -DCAS_SPINLOCK -fsanitize=realtime %s -o %t
// RUN: not %run %t 2>&1 | FileCheck %s --check-prefix=CHECK-ALL --check-prefix=CHECK-CAS
// RUN: %clangxx -DTEST_AND_SET_SPINLOCK -fsanitize=realtime %s -o %t
// RUN: not %run %t 2>&1 | FileCheck %s --check-prefix=CHECK-ALL --check-prefix=CHECK-TEST-AND-SET
// UNSUPPORTED: ios

#include <atomic>

#include <atomic>

class SpinLockTestAndSet {
public:
  void lock() {
    while (lock_flag.test_and_set(std::memory_order_acquire)) {
      // Busy-wait (spin) until the lock is acquired
    }
  }

  void unlock() { lock_flag.clear(std::memory_order_release); }

private:
  std::atomic_flag lock_flag = ATOMIC_FLAG_INIT;
};

class SpinLockCompareExchange {
public:
  void lock() {
    bool expected = false;
    while (!lock_flag.compare_exchange_weak(
        expected, true, std::memory_order_acquire, std::memory_order_relaxed)) {
    }
  }

  void unlock() { lock_flag.store(false, std::memory_order_release); }

private:
  std::atomic<bool> lock_flag{false};
};

int lock_violation() [[clang::nonblocking]] {
#if defined(TEST_AND_SET_SPINLOCK)
  SpinLockTestAndSet lock;
#elif defined(CAS_SPINLOCK)
  SpinLockCompareExchange lock;
#else
#  error "No spinlock defined"
#endif
  lock.lock();
  return 0;
}

int main() [[clang::nonblocking]] { lock_violation(); }

// CHECK-ALL: {{.*Real-time violation.*}}
// CHECK-CAS: {{.*SpinLockCompareExchange::lock.*}}
// CHECK-TEST-AND-SET: {{.*SpinLockTestAndSet::lock.*}}
