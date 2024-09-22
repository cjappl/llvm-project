//===--- rtsan_assertions.h - Realtime Sanitizer ----------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Part of the RealtimeSanitizer runtime library
//
//===----------------------------------------------------------------------===//

#pragma once

#include "rtsan/rtsan.h"
#include "rtsan/rtsan_context.h"
#include "rtsan/rtsan_stats.h"

#include "sanitizer_common/sanitizer_stackdepot.h"
#include "sanitizer_common/sanitizer_stacktrace.h"

namespace __rtsan {

#define GET_STACK_TRACE                                                        \
  BufferedStackTrace stack;                                                    \
  GET_CALLER_PC_BP;                                                            \
  stack.Unwind(pc, bp, nullptr, true)

template <typename OnViolationAction>
void ExpectNotRealtime(Context &context, OnViolationAction &&OnViolation) {
  CHECK(__rtsan_is_initialized());
  if (context.InRealtimeContext() && !context.IsBypassed()) {
    context.BypassPush();

    IncrementTotalErrorCount();

    using namespace __sanitizer;
    GET_STACK_TRACE;

    StackDepotHandle handle = StackDepotPut_WithHandle(stack);

    const bool never_before_seen_stack = handle.use_count() == 0;

    if (never_before_seen_stack)
      OnViolation();

    handle.inc_use_count_unsafe();

    context.BypassPop();
  }
}

} // namespace __rtsan
