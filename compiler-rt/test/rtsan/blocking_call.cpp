// RUN: %clangxx -fsanitize=realtime %s -o %t
// RUN: %env_rtsan_opts="symbolize=true" not %run %t 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-SYMBOLIZE
// RUN: %env_rtsan_opts="symbolize=false" not %run %t 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-NOSYMBOLIZE
// UNSUPPORTED: ios

// Intent: Check that a function marked with [[clang::nonblocking]] cannot call a function that is blocking.

#include <stdio.h>
#include <stdlib.h>

// TODO: Remove when [[blocking]] is implemented.
extern "C" void __rtsan_notify_blocking_call();

void custom_blocking_function() {
  // TODO: When [[blocking]] is implemented, don't call this directly.
  __rtsan_notify_blocking_call();
}

void safe_call() {
  // TODO: When [[blocking]] is implemented, don't call this directly.
  __rtsan_notify_blocking_call();
}

void process() [[clang::nonblocking]] { custom_blocking_function(); }

int main() {
  safe_call(); // This shouldn't die, because it isn't in nonblocking context.
  process();
  return 0;
  // CHECK-NOT: {{.*safe_call*}}

  // CHECK-SYMBOLIZE: ==ERROR: RealtimeSanitizer: blocking-call
  // CHECK-SYMBOLIZE-NEXT: Real-time unsafe call to blocking function `custom_blocking_function()`
  // CHECK-SYMBOLIZE-NEXT: #0 {{.*custom_blocking_function*}}
  // CHECK-SYMBOLIZE-NEXT: #1 {{.*process*}}

  // nosymbolize will instead print out "... to blocking function `(/abs/path/to/blocking_call.cpp.tmp:arm64+0x100003ee0)`"
  // CHECK-NOSYMBOLIZE: ==ERROR: RealtimeSanitizer: blocking-call
  // CHECK-NOSYMBOLIZE-NEXT: Real-time unsafe call to blocking function `({{.*}}+0x{{.*}})`

  // And of course have no additional symbols
  // CHECK-NOSYMBOLIZE-NEXT: #0 0x{{.*}}
  // CHECK-NOSYMBOLIZE-NEXT: #1 0x{{.*}}
}
