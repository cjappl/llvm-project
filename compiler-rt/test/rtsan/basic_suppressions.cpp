// RUN: %clangxx -fsanitize=realtime %s -o %t
// RUN: %env_rtsan_opts=suppressions='%s.supp' %run %t 2>&1 | FileCheck %s
// UNSUPPORTED: ios

// Intent: Ensure that suppressions work as intended

#include <stdio.h>
#include <stdlib.h>

void *MallocViolation() { return malloc(10); }

void process() [[clang::nonblocking]] { void *ptr = MallocViolation(); }

int main() {
  printf("Starting...\n");
  process();
  return 0;
}

// CHECK-NOT: failed to open suppressions file
// CHECK-NOT: intercepted call to real-time unsafe
