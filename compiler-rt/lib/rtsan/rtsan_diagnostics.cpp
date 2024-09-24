//===--- rtsan_diagnostics.cpp - Realtime Sanitizer -------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
//===----------------------------------------------------------------------===//

#include "rtsan/rtsan_diagnostics.h"

#include "sanitizer_common/sanitizer_flags.h"
#include "sanitizer_common/sanitizer_stacktrace.h"

using namespace __sanitizer;
using namespace __rtsan;

// We must define our own implementation of this method for our runtime.
// This one is just copied from UBSan.
namespace __sanitizer {
void BufferedStackTrace::UnwindImpl(uptr pc, uptr bp, void *context,
                                    bool request_fast, u32 max_depth) {
  uptr top = 0;
  uptr bottom = 0;
  GetThreadStackTopAndBottom(false, &top, &bottom);
  bool fast = StackTrace::WillUseFastUnwind(request_fast);
  Unwind(max_depth, pc, bp, context, top, bottom, fast);
}
} // namespace __sanitizer

namespace {
} // namespace

__rtsan::InterceptedCallInfo::InterceptedCallInfo(
    const char *intercepted_function_name)
    : intercepted_function_name_(intercepted_function_name) {}

void __rtsan::InterceptedCallInfo::PrintError(Decorator &d) const {
  Report("ERROR: RealtimeSanitizer: unsafe-library-call\n");
}

void __rtsan::InterceptedCallInfo::PrintReason(Decorator &d) const {
  Printf("Intercepted call to real-time unsafe function "
          "`%s%s%s` in real-time context!\n",
          d.FunctionName(),
          intercepted_function_name_, d.Reason());
}

__rtsan::BlockingCallInfo::BlockingCallInfo(const char *blocking_function_name)
    : blocking_function_name_(blocking_function_name) {}

void __rtsan::BlockingCallInfo::PrintError(Decorator &d) const {
  Report("ERROR: RealtimeSanitizer: blocking-call\n");
}

void __rtsan::BlockingCallInfo::PrintReason(Decorator &d) const {
  Printf("Call to blocking function "
          "`%s%s%s` in real-time context!\n",
          d.FunctionName(), blocking_function_name_, d.Reason());
}

void __rtsan::PrintDiagnostics(const DiagnosticsInfo &info) {
  ScopedErrorReportLock l;

  Decorator d;
  info.PrintError(d);
  info.PrintReason(d);
  Printf("%s", d.Default());
}
