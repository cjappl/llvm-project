//===-- rtsan_malloc_linux.cpp---------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file is a part of RealtimeSanitizer
//
// Linux-specific malloc interception.
//===----------------------------------------------------------------------===//

#include "sanitizer_common/sanitizer_platform.h"

#if SANITIZER_INTERCEPT_MEMALIGN || SANITIZER_INTERCEPT_PVALLOC
#include <malloc.h>
#endif

#if !SANITIZER_APPLE && !SANITIZER_WINDOWS

#include "interception/interception.h"
#include "sanitizer_common/sanitizer_allocator_dlsym.h"
#include "sanitizer_common/sanitizer_common.h"
#include "sanitizer_common/sanitizer_platform_interceptors.h"

#include "rtsan/rtsan.h"
#include "rtsan/rtsan_interceptors.h"
#include "rtsan/rtsan_internal.h"

using namespace __sanitizer;
using namespace __rtsan;

namespace {
struct DlsymAlloc : public DlSymAllocator<DlsymAlloc> {
  static bool UseImpl() { return !__rtsan_is_initialized(); }
};
} // namespace

INTERCEPTOR(void *, calloc, SIZE_T num, SIZE_T size) {
  if (DlsymAlloc::Use())
    return DlsymAlloc::Callocate(num, size);

  RTSAN_EXPECT_NOT_REALTIME("calloc");
  return REAL(calloc)(num, size);
}

INTERCEPTOR(void, free, void *ptr) {
  if (DlsymAlloc::PointerIsMine(ptr))
    return DlsymAlloc::Free(ptr);

  if (ptr != nullptr)
    RTSAN_EXPECT_NOT_REALTIME("free");

  return REAL(free)(ptr);
}

INTERCEPTOR(void *, malloc, SIZE_T size) {
  if (DlsymAlloc::Use())
    return DlsymAlloc::Allocate(size);

  RTSAN_EXPECT_NOT_REALTIME("malloc");
  return REAL(malloc)(size);
}

INTERCEPTOR(void *, realloc, void *ptr, SIZE_T size) {
  if (DlsymAlloc::Use() || DlsymAlloc::PointerIsMine(ptr))
    return DlsymAlloc::Realloc(ptr, size);

  RTSAN_EXPECT_NOT_REALTIME("realloc");
  return REAL(realloc)(ptr, size);
}

INTERCEPTOR(void *, reallocf, void *ptr, SIZE_T size) {
  RTSAN_EXPECT_NOT_REALTIME("reallocf");
  return REAL(reallocf)(ptr, size);
}

INTERCEPTOR(void *, valloc, SIZE_T size) {
  RTSAN_EXPECT_NOT_REALTIME("valloc");
  return REAL(valloc)(size);
}

INTERCEPTOR(void *, aligned_alloc, SIZE_T alignment, SIZE_T size) {
  RTSAN_EXPECT_NOT_REALTIME("aligned_alloc");
  return REAL(aligned_alloc)(alignment, size);
}

INTERCEPTOR(int, posix_memalign, void **memptr, SIZE_T alignment, SIZE_T size) {
  RTSAN_EXPECT_NOT_REALTIME("posix_memalign");
  return REAL(posix_memalign)(memptr, alignment, size);
}

#if SANITIZER_INTERCEPT_MEMALIGN
INTERCEPTOR(void *, memalign, SIZE_T alignment, SIZE_T size) {
  RTSAN_EXPECT_NOT_REALTIME("memalign");
  return REAL(memalign)(alignment, size);
}
#endif // SANITIZER_INTERCEPT_MEMALIGN

#if SANITIZER_INTERCEPT_PVALLOC
INTERCEPTOR(void *, pvalloc, SIZE_T size) {
  RTSAN_EXPECT_NOT_REALTIME("pvalloc");
  return REAL(pvalloc)(size);
}
#endif

void __rtsan::InitializeMallocInterceptors() {
  INTERCEPT_FUNCTION(calloc);
  INTERCEPT_FUNCTION(free);
  INTERCEPT_FUNCTION(malloc);
  INTERCEPT_FUNCTION(posix_memalign);
  INTERCEPT_FUNCTION(realloc);
  INTERCEPT_FUNCTION(valloc);
  INTERCEPT_FUNCTION(aligned_alloc);
  INTERCEPT_FUNCTION(reallocf);

#if SANITIZER_INTERCEPT_MEMALIGN
  INTERCEPT_FUNCTION(memalign);
#endif

#if SANITIZER_INTERCEPT_PVALLOC
  INTERCEPT_FUNCTION(pvalloc);
#endif
}

#endif // !SANITIZER_APPLE && !SANITIZER_WINDOWS
