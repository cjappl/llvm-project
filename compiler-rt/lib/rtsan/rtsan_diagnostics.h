//===--- rtsan_diagnostics.h - Realtime Sanitizer ---------------*- C++ -*-===//
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

#include "sanitizer_common/sanitizer_common.h"
#include "sanitizer_common/sanitizer_internal_defs.h"
#include "sanitizer_common/sanitizer_report_decorator.h"

namespace __rtsan {

class Decorator : public __sanitizer::SanitizerCommonDecorator {
public:
  Decorator() : SanitizerCommonDecorator() {}
  const char *FunctionName() const { return Green(); }
  const char *Reason() const { return Blue(); }
};

class DiagnosticsInfo {
public:
  virtual ~DiagnosticsInfo() = default;
  virtual void PrintError(Decorator& d) const = 0;
  virtual void PrintReason(Decorator& d) const = 0;
};

class InterceptedCallInfo : public DiagnosticsInfo {
public:
  explicit InterceptedCallInfo(const char *intercepted_function_name);
  void PrintError(Decorator& d) const override;
  void PrintReason(Decorator& d) const override;
private:
  const char *intercepted_function_name_;
};

struct BlockingCallInfo : public DiagnosticsInfo {
public:
  explicit BlockingCallInfo(const char *blocking_function_name);
  void PrintError(Decorator& d) const override;
  void PrintReason(Decorator& d) const override;
private:
  const char *blocking_function_name_;
};

void PrintDiagnostics(const DiagnosticsInfo &info);
} // namespace __rtsan
