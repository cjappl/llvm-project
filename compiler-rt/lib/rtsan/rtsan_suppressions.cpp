//===--- rtsan_suppressions.cpp - Realtime Sanitizer ------------*- C++ -*-===//
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

#include "rtsan/rtsan_suppressions.h"

#include "rtsan/rtsan_flags.h"

#include "sanitizer_common/sanitizer_common.h"
#include "sanitizer_common/sanitizer_internal_defs.h"
#include "sanitizer_common/sanitizer_suppressions.h"
#include "sanitizer_common/sanitizer_symbolizer.h"

#include <new>

using namespace __sanitizer;
using namespace __rtsan;

alignas(64) static char suppression_placeholder[sizeof(SuppressionContext)];
static SuppressionContext *suppression_ctx = nullptr;
static const char *kSuppressionTypes[] = {
#define RTSAN_CHECK(Name, SummaryKind, FSanitizeFlagName) FSanitizeFlagName,
#include "rtsan_checks.inc"
#undef RTSAN_CHECK
};

static const char *ConvertTypeToFlagName(ErrorType Type) {
  switch (Type) {
#define RTSAN_CHECK(Name, SummaryKind, FSanitizeFlagName)                      \
  case ErrorType::Name:                                                        \
    return FSanitizeFlagName;
#include "rtsan_checks.inc"
#undef RTSAN_CHECK
  }
  UNREACHABLE("unknown ErrorType!");
}

void __rtsan::InitializeSuppressions() {
  CHECK_EQ(nullptr, suppression_ctx);
  suppression_ctx = new (suppression_placeholder)
      SuppressionContext(kSuppressionTypes, ARRAY_SIZE(kSuppressionTypes));
  suppression_ctx->ParseFromFile(flags().suppressions);
}

bool __rtsan::IsPCSuppressed(ErrorType error_type, uptr pc) {
  CHECK(suppression_ctx);
  const char *SuppType = ConvertTypeToFlagName(error_type);
  // Fast path: don't symbolize pc if there is no suppressions for given error
  // type.
  if (!suppression_ctx->HasSuppressionType(SuppType))
    return false;

  Suppression *s = nullptr;
  if (const char *Module = Symbolizer::GetOrInit()->GetModuleNameForPc(pc)) {
    if (suppression_ctx->Match(Module, SuppType, &s))
      return true;
  }

  SymbolizedStackHolder symbolized_stack(
      Symbolizer::GetOrInit()->SymbolizePC(pc));
  const SymbolizedStack *frames = symbolized_stack.get();
  CHECK(frames);
  for (const SymbolizedStack *cur = frames; cur != nullptr; cur = cur->next) {
    const char *function_name = cur->info.function;
    if (!function_name) {
      continue;
    }
    if (suppression_ctx->Match(function_name, ConvertTypeToFlagName(error_type),
                               &s)) {
      return true;
    }
  }

  // Suppress by function or source file name from debug info.
  const AddressInfo &address_info = symbolized_stack.get()->info;
  return suppression_ctx->Match(address_info.function, SuppType, &s) ||
         suppression_ctx->Match(address_info.file, SuppType, &s);
}
