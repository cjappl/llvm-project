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
#include "rtsan/rtsan_flags.h"
#include "rtsan/rtsan_stats.h"

#include "sanitizer_common/sanitizer_stackdepot.h"
#include "sanitizer_common/sanitizer_stacktrace.h"

namespace __rtsan {

template <typename OnViolationAction>
void ExpectNotRealtime(Context &context, OnViolationAction &&OnViolation,
                       __sanitizer::uptr caller_pc,
                       __sanitizer::uptr caller_bp) {
  CHECK(__rtsan_is_initialized());
  if (context.InRealtimeContext() && !context.IsBypassed()) {
    context.BypassPush();

    if (flags().print_stats_on_exit)
      IncrementTotalErrorCount();

    __sanitizer::BufferedStackTrace stack;
    stack.Unwind(caller_pc, caller_bp, nullptr,
                 /*request_fast*/ true);

    __sanitizer::StackDepotHandle handle = StackDepotPut_WithHandle(stack);

    const bool is_stack_novel = handle.use_count() == 0;
    if (UNLIKELY(is_stack_novel)) {
      OnViolation(stack);
      handle.inc_use_count_unsafe();
    }

    context.BypassPop();
  }
}

} // namespace __rtsan
