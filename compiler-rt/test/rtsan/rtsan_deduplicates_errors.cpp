// RUN: %clangxx -fsanitize=realtime %s -o %t
// RUN: env RTSAN_OPTIONS="halt_on_error=false,print_stats_on_exit=true" %run %t 2>&1 | FileCheck %s
// RUN: env RTSAN_OPTIONS="halt_on_error=false" %run %t 2>&1 | grep "unsafe function " | wc -l | awk '{exit $1 != 3}'
// UNSUPPORTED: ios

// Intent: Ensure that there is no duplication of reports when not halt_on_error

#include <stdlib.h>

void *MallocViolation() { return malloc(10); }

void *MallocViolation2() { return malloc(10); }

void FreeViolation(void *Ptr) { free(Ptr); }

void process() [[clang::nonblocking]] {
  void *ptr;
  for (int i = 0; i < 10; i++) {
    ptr = MallocViolation();
  }
  FreeViolation(ptr);
  ptr = MallocViolation2();
}

static void process_another() [[clang::nonblocking]] { process(); }

static void process_elsewhere() [[clang::nonblocking]] {
  process();
  process_another();
}

int main() {
  process();
  process_elsewhere();
  process_another();
  return 0;
}

// All of these, if successfully deduplicated should only result in 3 stack
// being printed out

// CHECK-LABEL: {{.*real-time unsafe function `malloc`.*}}
// CHECK-NEXT:  {{.*MallocViolation.*}}

// CHECK-LABEL: {{.*real-time unsafe function `free`.*}}
// CHECK-NEXT:  {{.*FreeViolation.*}}

// CHECK-LABEL: {{.*real-time unsafe function `malloc`.*}}
// CHECK-NEXT:  {{.*MallocViolation2.*}}

// CHECK-LABEL: {{.*RealtimeSanitizer exit stats:.*}}
