#include <radsan/radsan_interceptors.h>

#include <sanitizer_common/sanitizer_platform.h>

#include <interception/interception.h>
#include <radsan/radsan_context.h>

#if !SANITIZER_LINUX && !SANITIZER_APPLE
#error Sorry, radsan does not yet support this platform
#endif

#if SANITIZER_APPLE
#include <libkern/OSAtomic.h>
#include <os/lock.h>
#endif

#include <fcntl.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

using namespace __sanitizer;

namespace radsan {
void exitIfRealtime(const char *intercepted_function_name) {
  getContextForThisThread().exitIfRealtime(intercepted_function_name);
}
} // namespace radsan

/*
    Filesystem
*/

INTERCEPTOR(int, open, const char *path, int oflag, ...) {
  // TODO Establish whether we should intercept here if the flag contains
  // O_NONBLOCK
  radsan::exitIfRealtime("open");
  va_list args;
  va_start(args, oflag);
  auto result = REAL(open)(path, oflag, args);
  va_end(args);
  return result;
}

INTERCEPTOR(int, openat, int fd, const char *path, int oflag, ...) {
  // TODO Establish whether we should intercept here if the flag contains
  // O_NONBLOCK
  radsan::exitIfRealtime("openat");
  va_list args;
  va_start(args, oflag);
  auto result = REAL(openat)(fd, path, oflag, args);
  va_end(args);
  return result;
}

INTERCEPTOR(int, creat, const char *path, mode_t mode) {
  // TODO Establish whether we should intercept here if the flag contains
  // O_NONBLOCK
  radsan::exitIfRealtime("creat");
  auto result = REAL(creat)(path, mode);
  return result;
}

INTERCEPTOR(int, fcntl, int filedes, int cmd, ...) {
  radsan::exitIfRealtime("fcntl");
  va_list args;
  va_start(args, cmd);
  auto result = REAL(fcntl)(filedes, cmd, args[0]);
  va_end(args);
  return result;
}

INTERCEPTOR(int, close, int filedes) {
  radsan::exitIfRealtime("close");
  return REAL(close)(filedes);
}

INTERCEPTOR(FILE *, fopen, const char *path, const char * mode) {
  radsan::exitIfRealtime("fopen");
  return REAL(fopen)(path, mode);
}

INTERCEPTOR(int, fclose, FILE *stream) {
  radsan::exitIfRealtime("fclose");
  return REAL(fclose)(stream);
}

/*
    Concurrency
*/

#if SANITIZER_APPLE
INTERCEPTOR(void, OSSpinLockLock, volatile OSSpinLock *lock) {
  radsan::exitIfRealtime("OSSpinLockLock");
  return REAL(OSSpinLockLock)(lock);
}

INTERCEPTOR(void, os_unfair_lock_lock, os_unfair_lock_t lock) {
  radsan::exitIfRealtime("os_unfair_lock_lock");
  return REAL(os_unfair_lock_lock)(lock);
}
#elif SANITIZER_LINUX
INTERCEPTOR(int, pthread_spin_lock, pthread_spinlock_t *spinlock) {
  radsan::exitIfRealtime("pthread_spin_lock");
  return REAL(pthread_spin_lock)(spinlock);
}
#endif

INTERCEPTOR(int, pthread_create, pthread_t *thread, const pthread_attr_t *attr,
            void *(*start_routine)(void *), void *arg) {
  radsan::exitIfRealtime("pthread_create");
  return REAL(pthread_create)(thread, attr, start_routine, arg);
}

INTERCEPTOR(int, pthread_mutex_lock, pthread_mutex_t *mutex) {
  radsan::exitIfRealtime("pthread_mutex_lock");
  return REAL(pthread_mutex_lock)(mutex);
}

INTERCEPTOR(int, pthread_mutex_unlock, pthread_mutex_t *mutex) {
  radsan::exitIfRealtime("pthread_mutex_unlock");
  return REAL(pthread_mutex_unlock)(mutex);
}

INTERCEPTOR(int, pthread_join, pthread_t thread, void **value_ptr) {
  radsan::exitIfRealtime("pthread_join");
  return REAL(pthread_join)(thread, value_ptr);
}

INTERCEPTOR(int, pthread_cond_signal, pthread_cond_t *cond) {
  radsan::exitIfRealtime("pthread_cond_signal");
  return REAL(pthread_cond_signal)(cond);
}

INTERCEPTOR(int, pthread_cond_broadcast, pthread_cond_t *cond) {
  radsan::exitIfRealtime("pthread_cond_broadcast");
  return REAL(pthread_cond_broadcast)(cond);
}

INTERCEPTOR(int, pthread_cond_wait, pthread_cond_t *cond,
            pthread_mutex_t *mutex) {
  radsan::exitIfRealtime("pthread_cond_wait");
  return REAL(pthread_cond_wait)(cond, mutex);
}

INTERCEPTOR(int, pthread_cond_timedwait, pthread_cond_t *cond,
            pthread_mutex_t *mutex, const timespec *ts) {
  radsan::exitIfRealtime("pthread_cond_timedwait");
  return REAL(pthread_cond_timedwait)(cond, mutex, ts);
}

INTERCEPTOR(int, pthread_rwlock_rdlock, pthread_rwlock_t *lock) {
  radsan::exitIfRealtime("pthread_rwlock_rdlock");
  return REAL(pthread_rwlock_rdlock)(lock);
}

INTERCEPTOR(int, pthread_rwlock_unlock, pthread_rwlock_t *lock) {
  radsan::exitIfRealtime("pthread_rwlock_unlock");
  return REAL(pthread_rwlock_unlock)(lock);
}

INTERCEPTOR(int, pthread_rwlock_wrlock, pthread_rwlock_t *lock) {
  radsan::exitIfRealtime("pthread_rwlock_wrlock");
  return REAL(pthread_rwlock_wrlock)(lock);
}

/*
    Sleeping
*/

INTERCEPTOR(unsigned int, sleep, unsigned int s) {
  radsan::exitIfRealtime("sleep");
  return REAL(sleep)(s);
}

INTERCEPTOR(int, usleep, useconds_t u) {
  radsan::exitIfRealtime("usleep");
  return REAL(usleep)(u);
}

INTERCEPTOR(int, nanosleep, const struct timespec *rqtp,
            struct timespec *rmtp) {
  radsan::exitIfRealtime("nanosleep");
  return REAL(nanosleep)(rqtp, rmtp);
}

/*
    Memory
*/

INTERCEPTOR(void *, calloc, SIZE_T num, SIZE_T size) {
  radsan::exitIfRealtime("calloc");
  return REAL(calloc)(num, size);
}

INTERCEPTOR(void, free, void *ptr) {
  if (ptr != NULL) {
    radsan::exitIfRealtime("free");
  }
  return REAL(free)(ptr);
}

INTERCEPTOR(void *, malloc, SIZE_T size) {
  radsan::exitIfRealtime("malloc");
  return REAL(malloc)(size);
}

INTERCEPTOR(void *, realloc, void *ptr, SIZE_T size) {
  radsan::exitIfRealtime("realloc");
  return REAL(realloc)(ptr, size);
}

INTERCEPTOR(void *, reallocf, void *ptr, SIZE_T size) {
  radsan::exitIfRealtime("reallocf");
  return REAL(reallocf)(ptr, size);
}

INTERCEPTOR(void *, valloc, SIZE_T size) {
  radsan::exitIfRealtime("valloc");
  return REAL(valloc)(size);
}

INTERCEPTOR(void *, aligned_alloc, SIZE_T alignment, SIZE_T size) {
  radsan::exitIfRealtime("aligned_alloc");
  return REAL(aligned_alloc)(alignment, size);
}

/*
    Preinit
*/

namespace radsan {
void initialiseInterceptors() {
  INTERCEPT_FUNCTION(open);
  INTERCEPT_FUNCTION(openat);
  INTERCEPT_FUNCTION(close);
  INTERCEPT_FUNCTION(fclose);
  INTERCEPT_FUNCTION(fcntl);
  INTERCEPT_FUNCTION(creat);

#if SANITIZER_APPLE
  INTERCEPT_FUNCTION(OSSpinLockLock);
  INTERCEPT_FUNCTION(os_unfair_lock_lock);
#elif SANITIZER_LINUX
  INTERCEPT_FUNCTION(pthread_spin_lock);
#endif

  INTERCEPT_FUNCTION(pthread_create);
  INTERCEPT_FUNCTION(pthread_mutex_lock);
  INTERCEPT_FUNCTION(pthread_mutex_unlock);
  INTERCEPT_FUNCTION(pthread_join);
  INTERCEPT_FUNCTION(pthread_cond_signal);
  INTERCEPT_FUNCTION(pthread_cond_broadcast);
  INTERCEPT_FUNCTION(pthread_cond_wait);
  INTERCEPT_FUNCTION(pthread_cond_timedwait);
  INTERCEPT_FUNCTION(pthread_rwlock_rdlock);
  INTERCEPT_FUNCTION(pthread_rwlock_unlock);
  INTERCEPT_FUNCTION(pthread_rwlock_wrlock);

  INTERCEPT_FUNCTION(sleep);
  INTERCEPT_FUNCTION(usleep);
  INTERCEPT_FUNCTION(nanosleep);

  INTERCEPT_FUNCTION(calloc);
  INTERCEPT_FUNCTION(free);
  INTERCEPT_FUNCTION(malloc);
  INTERCEPT_FUNCTION(realloc);
  INTERCEPT_FUNCTION(reallocf);
  INTERCEPT_FUNCTION(valloc);
  INTERCEPT_FUNCTION(aligned_alloc);
}
} // namespace radsan
