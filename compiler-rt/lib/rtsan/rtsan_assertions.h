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
#include "rtsan/rtsan_diagnostics.h"
#include "rtsan/rtsan_suppressions.h"

#include "sanitizer_common/sanitizer_stacktrace.h"

namespace __rtsan {

class ScopedBypass {
public:
  [[nodiscard]] explicit ScopedBypass(Context &context) : context_(context) {
    context_.BypassPush();
  }

  ~ScopedBypass() { context_.BypassPop(); }

private:
  Context &context_;
};

template <typename OnViolationAction>
void ExpectNotRealtime(Context &context, const DiagnosticsInfo &info,
                       OnViolationAction &&OnViolation) {
  using namespace __sanitizer;

  CHECK(__rtsan_is_initialized());
  if (context.InRealtimeContext() && !context.IsBypassed()) {
    ScopedBypass scoped_bypass{context};

    if (IsInterceptorSuppressed(info.func_name))
      return;

    BufferedStackTrace stack;

    // We use the unwind_on_fatal flag here because of precedent with other
    // sanitizers, this action is not necessarily fatal if halt_on_error=false
    stack.Unwind(info.pc, info.bp, nullptr,
                 common_flags()->fast_unwind_on_fatal);

    if (IsStackTraceSuppressed(stack))
      return;

    OnViolation(stack, info);
  }
}

} // namespace __rtsan
