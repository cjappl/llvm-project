// RUN: %clangxx -fsanitize=realtime %s -o %t -O3
// RUN: %run %t 2>&1 | FileCheck %s --allow-empty
// RUN: %clangxx -fsanitize=realtime %s -o %t -O2
// RUN: %run %t 2>&1 | FileCheck %s --allow-empty
// RUN: %clangxx -fsanitize=realtime %s -o %t -O1
// RUN: %run %t 2>&1 | FileCheck %s --allow-empty

// RUN: %clangxx -fsanitize=realtime %s -o %t -O0
// RUN: %run %t 2>&1 | FileCheck %s --allow-empty

// UNSUPPORTED: ios

// Intent: Ensure basic bound audio loops don't trigger rtsan.

#include <stdio.h>
#include <stdlib.h>

#include <assert.h>


void FillBufferInterleaved(int* buffer, int sample_count, int channel_count) [[clang::nonblocking]] {
  for (int sample = 0; sample < sample_count; sample++)
    for (int channel = 0; channel < channel_count; channel++)
      buffer[sample * channel_count + channel] = sample;
}

void Deinterleave(int* buffer, int sample_count, int channel_count, int* scratch_buffer) [[clang::nonblocking]] {
  for (int channel = 0; channel < channel_count; channel++)
    for (int sample = 0; sample < sample_count; sample++) {
          int interleaved_index = sample * channel_count + channel;
          int deinterleaved_index = channel * sample_count + sample;
          scratch_buffer[deinterleaved_index] = buffer[interleaved_index];
      }

  for (int i = 0; i < sample_count * channel_count; i++)
      buffer[i] = scratch_buffer[i];
}

int main() {
  const int sample_count = 10;
  const int channel_count = 2;
  int buffer[channel_count * sample_count];

  FillBufferInterleaved(buffer, sample_count, channel_count);

  assert(buffer[0] == 0);
  assert(buffer[1] == 0);
  assert(buffer[2] == 1);
  assert(buffer[3] == 1);

  assert(buffer[18] == 9);
  assert(buffer[19] == 9);

  int scratch_buffer[channel_count * sample_count];

  Deinterleave(buffer, sample_count, channel_count, scratch_buffer);

  assert(buffer[0] == 0);
  assert(buffer[1] == 1);
  assert(buffer[8] == 8);
  assert(buffer[9] == 9);

  assert(buffer[10] == 0);
  assert(buffer[11] == 1);
  assert(buffer[18] == 8);
  assert(buffer[19] == 9);

  return 0;
}

// CHECK-NOT: {{.*Real-time violation.*}}
