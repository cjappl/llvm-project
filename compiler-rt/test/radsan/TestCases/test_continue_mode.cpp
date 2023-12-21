// RUN: %clangxx -fsanitize=realtime %s -o %t
// RUN: env RADSAN_OPTIONS="error_mode=continue" not %run %t 2>&1 | FileCheck %s
// UNSUPPORTED: ios

// Intent: Ensure that Continue mode does not exit on the first violation.

#include <stdlib.h>

void* mallocViolation() {
  return malloc(10);
}

void freeViolation(void* Ptr) {
  free(Ptr);
}

[[clang::realtime]] void process() {
  void* Ptr = mallocViolation();
  freeViolation(Ptr);
}

int main() {
  process();
  return 0;
  // CHECK: {{.*Real-time violation.*}}
  // CHECK: {{.*malloc*}}
  // CHECK: {{.*free*}}
  // CHECK: {{.*2 warnings reported.*}}
}
