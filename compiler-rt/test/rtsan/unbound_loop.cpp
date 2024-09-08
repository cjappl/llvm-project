// RUN: %clangxx -fsanitize=realtime %s -o %t -O3
// RUN: not %run %t 2>&1 | FileCheck %s --allow-empty
// RUN: %clangxx -fsanitize=realtime %s -o %t -O2
// RUN: not %run %t 2>&1 | FileCheck %s --allow-empty
// RUN: %clangxx -fsanitize=realtime %s -o %t -O1
// RUN: not %run %t 2>&1 | FileCheck %s --allow-empty

// RUN: %clangxx -fsanitize=realtime %s -o %t -O0
// RUN: not %run %t 2>&1 | FileCheck %s --allow-empty

// UNSUPPORTED: ios

// Intent: Ensure basic bound audio loops don't trigger rtsan.

#include <stdio.h>
#include <stdlib.h>

#include <assert.h>


void BadScalarEvolution(int* buffer, int sample_count, int channel_count) [[clang::nonblocking]] {

  int sample = 0;
  while (sample < sample_count) {
    int channel = 0;
    while (channel < channel_count) {
      buffer[sample * channel_count + channel] = sample;
      channel++;
    }
    sample++;

    // NOTE! Here is the "bug" that causes the loop to be unbounded.
    sample_count++;
  }
}

int main() {
  const int sample_count = 10;
  const int channel_count = 2;
  int buffer[channel_count * sample_count];

  BadScalarEvolution(buffer, sample_count, channel_count);

  return 0;
}

// CHECK: {{.*Real-time violation.*}}
// CHECK-NEXT {{.*BadScalarEvolution.*}}
