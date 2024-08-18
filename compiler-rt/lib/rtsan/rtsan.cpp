//===--- rtsan.cpp - Realtime Sanitizer -------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
//===----------------------------------------------------------------------===//

#include <rtsan/rtsan.h>

#include <rtsan/rtsan_context.h>
#include <rtsan/rtsan_flags.h>
#include <rtsan/rtsan_interceptors.h>
#include <rtsan/rtsan_internal.h>
#include <rtsan/rtsan_stack.h>

#include "sanitizer_common/sanitizer_atomic.h"
#include "sanitizer_common/sanitizer_common.h"
#include "sanitizer_common/sanitizer_mutex.h"

using namespace __rtsan;
using namespace __sanitizer;

static StaticSpinMutex rtsan_inited_mutex;
static atomic_uint8_t rtsan_initialized = {0};

static const unsigned rtsan_buggy_pc_pool_size = 25;
static atomic_uintptr_t rtsan_buggy_pc_pool[rtsan_buggy_pc_pool_size];

static atomic_uint32_t rtsan_total_error_count{0};
static atomic_uint32_t rtsan_unique_error_count{0};

static void IncrementTotalErrorCount() {
  atomic_fetch_add(&rtsan_total_error_count, 1, memory_order_relaxed);
}

static u32 GetTotalErrorCount() {
  return atomic_load(&rtsan_total_error_count, memory_order_relaxed);
}

static void IncrementUniqueErrorCount() {
  atomic_fetch_add(&rtsan_unique_error_count, 1, memory_order_relaxed);
}

static u32 GetUniqueErrorCount() {
  return atomic_load(&rtsan_unique_error_count, memory_order_relaxed);
}

static void SetInitialized() {
  atomic_store(&rtsan_initialized, 1, memory_order_release);
}

static void PrintDiagnostics(const char *intercepted_function_name, uptr pc,
                             uptr bp) {
  ScopedErrorReportLock l;

  Report("Real-time violation: intercepted call to real-time unsafe function "
         "`%s` in real-time context! Stack trace:\n",
         intercepted_function_name);
  PrintStackTrace(pc, bp);
}

static bool SuppressErrorReport(uptr pc) {
  for (unsigned i = 0; i < rtsan_buggy_pc_pool_size; i++) {
    uptr cmp = atomic_load_relaxed(&rtsan_buggy_pc_pool[i]);
    if (cmp == 0 &&
        atomic_compare_exchange_strong(&rtsan_buggy_pc_pool[i], &cmp, pc,
                                       memory_order_relaxed))
      return false;
    if (cmp == pc)
      return true;
  }
  Die();
}

void __rtsan::InternalExpectNotRealtime(Context &context,
                                        const char *intercepted_function_name,
                                        uptr pc, uptr bc) {
  __rtsan_ensure_initialized();

  if (context.InRealtimeContext() && !context.IsBypassed()) {
    context.BypassPush();
    IncrementTotalErrorCount();

    if (!SuppressErrorReport(pc)) {
      IncrementUniqueErrorCount();

      PrintDiagnostics(intercepted_function_name, pc, bc);

      if (flags().halt_on_error)
        Die();
    }

    context.BypassPop();
  }
}

static void rtsan_atexit() {
  ScopedErrorReportLock l;
  Printf("RealtimeSanitizer exit stats:\n");
  Printf("    Total error count: %u\n", GetTotalErrorCount());
  Printf("    Unique error count: %u\n", GetUniqueErrorCount());
}

extern "C" {

SANITIZER_INTERFACE_ATTRIBUTE void __rtsan_init() {
  CHECK(!__rtsan_is_initialized());

  SanitizerToolName = "RealtimeSanitizer";
  InitializeFlags();
  InitializeInterceptors();

  if (flags().print_stats_on_exit)
    Atexit(rtsan_atexit);

  SetInitialized();
}

SANITIZER_INTERFACE_ATTRIBUTE void __rtsan_ensure_initialized() {
  if (LIKELY(__rtsan_is_initialized()))
    return;

  SpinMutexLock lock(&rtsan_inited_mutex);

  // Someone may have initialized us while we were waiting for the lock
  if (__rtsan_is_initialized())
    return;

  __rtsan_init();
}

SANITIZER_INTERFACE_ATTRIBUTE bool __rtsan_is_initialized() {
  return atomic_load(&rtsan_initialized, memory_order_acquire) == 1;
}

SANITIZER_INTERFACE_ATTRIBUTE void __rtsan_realtime_enter() {
  __rtsan::GetContextForThisThread().RealtimePush();
}

SANITIZER_INTERFACE_ATTRIBUTE void __rtsan_realtime_exit() {
  __rtsan::GetContextForThisThread().RealtimePop();
}

SANITIZER_INTERFACE_ATTRIBUTE void __rtsan_disable() {
  __rtsan::GetContextForThisThread().BypassPush();
}

SANITIZER_INTERFACE_ATTRIBUTE void __rtsan_enable() {
  __rtsan::GetContextForThisThread().BypassPop();
}

SANITIZER_INTERFACE_ATTRIBUTE void
__rtsan_expect_not_realtime(const char *intercepted_function_name) {
  __rtsan_ensure_initialized();
  ExpectNotRealtime(GetContextForThisThread(), intercepted_function_name);
}

} // extern "C"
