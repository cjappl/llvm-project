//===--- rtsan_suppressions.h - Realtime Sanitizer --------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file is a part of the RTSan runtime, providing support for suppressions
//
//===----------------------------------------------------------------------===//

#pragma once

#include "sanitizer_common/sanitizer_common.h"

namespace __rtsan {

enum class ErrorType {
#define RTSAN_CHECK(Name, SummaryKind, FSanitizeFlagName) Name,
#include "rtsan_checks.inc"
#undef RTSAN_CHECK
};

void InitializeSuppressions();
bool IsPCSuppressed(ErrorType ET, __sanitizer::uptr PC);

} // namespace __rtsan
