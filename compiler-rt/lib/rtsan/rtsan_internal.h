//===--- rtsan_internal.h - Realtime Sanitizer ------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file is a part of the RTSan Runtime Library.
//
// It declares various internal functions and data structures used by the RTSan
//
//===----------------------------------------------------------------------===//

#pragma once

#include "rtsan/rtsan_context.h"

#include "sanitizer_common/sanitizer_internal_defs.h"
#include "sanitizer_common/sanitizer_stacktrace.h"

// This macro is important because it allows us to collect the caller
// PC and BP as high up in the stack frame as possible.
// This means we get the cleanest stack traces possible, and can check
// For duplicate PC to not log the same issue twice.
//
// Any internal interceptor should use this macro
#define RTSAN_EXPECT_NOT_REALTIME(intercepted_function_name)                   \
  do {                                                                         \
    GET_CALLER_PC_BP;                                                          \
    Context &context = GetContextForThisThread();                              \
    InternalExpectNotRealtime(context, intercepted_function_name, pc, bp);     \
  } while (0)

namespace __rtsan {
void InternalExpectNotRealtime(__rtsan::Context &context,
                               const char *intercepted_function_name,
                               __sanitizer::uptr pc, __sanitizer::uptr bp);
} // namespace __rtsan
