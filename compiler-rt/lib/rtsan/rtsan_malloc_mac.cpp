//===-- rtsan_malloc_mac.cpp-----------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file is a part of RealtimeSanitizer
//
// Mac-specific malloc interception.
//===----------------------------------------------------------------------===//

#include "sanitizer_common/sanitizer_platform.h"
#if SANITIZER_APPLE

#include "sanitizer_common/sanitizer_errno_codes.h"
#include "sanitizer_common/sanitizer_internal_defs.h"

#include "rtsan/rtsan.h"
#include "rtsan/rtsan_interceptors.h"

#include <cstdlib>

void __rtsan::InitializeMallocInterceptors() {}

using namespace __rtsan;
using namespace __sanitizer;

#define COMMON_MALLOC_ZONE_NAME "rtsan"
#define COMMON_MALLOC_ENTER()                                                  \
  do {                                                                         \
    __rtsan_ensure_initialized();                                              \
  } while (false)
#define COMMON_MALLOC_SANITIZER_INITIALIZED __rtsan_is_initialized()
#define COMMON_MALLOC_FORCE_LOCK()
#define COMMON_MALLOC_FORCE_UNLOCK()
#define COMMON_MALLOC_FREE(ptr)                                                \
  __rtsan_expect_not_realtime("free");                                         \
  return REAL(malloc_zone_free)(malloc_default_zone(), ptr);
#define COMMON_MALLOC_MALLOC(size)                                             \
  __rtsan_expect_not_realtime("malloc");                                       \
  void *p = REAL(malloc_zone_malloc)(malloc_default_zone(), size);
#define COMMON_MALLOC_REALLOC(ptr, size)                                       \
  __rtsan_expect_not_realtime("realloc");                                      \
  void *p = REAL(malloc_zone_realloc)(malloc_default_zone(), ptr, size);
#define COMMON_MALLOC_CALLOC(count, size)                                      \
  __rtsan_expect_not_realtime("calloc");                                       \
  void *p = REAL(malloc_zone_calloc)(malloc_default_zone(), count, size);
#define COMMON_MALLOC_POSIX_MEMALIGN(memptr, alignment, size)                  \
  __rtsan_expect_not_realtime("posix_memalign");                               \
  int res = 0;                                                                 \
  if (UNLIKELY(!IsPowerOfTwo(alignment))) {                                    \
    memptr = nullptr;                                                          \
    res = errno_EINVAL;                                                        \
  }                                                                            \
                                                                               \
  if (res == 0) {                                                              \
    void *ptr =                                                                \
        REAL(malloc_zone_memalign)(malloc_default_zone(), alignment, size);    \
    if (UNLIKELY(!ptr))                                                        \
      res = errno_ENOMEM;                                                      \
    else                                                                       \
      *memptr = ptr;                                                           \
  }
#define COMMON_MALLOC_VALLOC(size)                                             \
  __rtsan_expect_not_realtime("valloc");                                       \
  void *p = REAL(malloc_zone_valloc)(malloc_default_zone(), size);
#define COMMON_MALLOC_MALLOC_ZONE_ENUMERATOR(zone_name, zone_ptr)
#define COMMON_MALLOC_SIZE(ptr) uptr size = REAL(malloc_size)(ptr);
#define COMMON_MALLOC_FILL_STATS(zone, stats)                                  \
  REAL(malloc_zone_statistics)(zone, stats);
#define COMMON_MALLOC_REPORT_UNKNOWN_REALLOC(ptr, zone_ptr, zone_name)         \
  __rtsan_expect_not_realtime(__func__);                                       \
  Report("mz_realloc(%p) -- attempting to realloc unallocated memory. Zone "   \
         "name: %s\n",                                                         \
         ptr, zone_name);
#define COMMON_MALLOC_MEMALIGN(alignment, size)                                \
  __rtsan_expect_not_realtime(__func__);                                       \
  void *p = REAL(malloc_zone_memalign)(malloc_default_zone(), alignment, size);
#define COMMON_MALLOC_NAMESPACE __rtsan
#define COMMON_MALLOC_HAS_ZONE_ENUMERATOR 0
#define COMMON_MALLOC_HAS_EXTRA_INTROSPECTION_INIT 0

#include "sanitizer_common/sanitizer_malloc_mac.inc"

#endif // SANITIZER_APPLE
