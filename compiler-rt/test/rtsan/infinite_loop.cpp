// RUN: %clangxx -fsanitize=realtime %s -o %t
// RUN: not %run %t 2>&1 | FileCheck %s
// UNSUPPORTED: ios

// Intent: Ensure we detect infinite loops

#include <stdio.h>
#include <stdlib.h>

void infinity() [[clang::nonblocking]] {
  while (true)
    ;
}

int main() {
  infinity();
  return 0;
  // CHECK: {{.*Real-time violation.*}}
  // CHECK: {{.*infinity*}}
}
