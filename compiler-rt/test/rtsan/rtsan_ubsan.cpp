// RUN: %clangxx -fsanitize=undefined,realtime %s -o %t
// RUN: %env_rtsan_opts="halt_on_error=0" env UBSAN_OPTIONS="halt_on_error=0" %run %t 2>&1 | FileCheck %s
// UNSUPPORTED: ios

// Intent: Ensure that RealtimeSanitizer and UBSan can both report errors from
//         the same binary when compiled with -fsanitize=realtime,undefined.

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

__attribute__((noinline)) int TriggerUbsan() {
  volatile int max = INT_MAX;
  return max + 1;
}

void TriggerRtsan() [[clang::nonblocking]] {
  int overflowed = TriggerUbsan();
  printf("overflowed=%d\n", overflowed);
}

int main() {
  TriggerRtsan();
  return 0;
}

// CHECK: runtime error: signed integer overflow
// CHECK: Intercepted call to real-time unsafe function `malloc` in real-time context!
// CHECK: SUMMARY: RealtimeSanitizer: unsafe-library-call
