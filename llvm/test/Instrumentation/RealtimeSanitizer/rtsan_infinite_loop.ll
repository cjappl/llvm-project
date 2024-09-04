; RUN: opt < %s -passes=rtsan -S | FileCheck %s

define void @process() #0 {
entry:
  br label %while.body

while.body:                                       ; preds = %entry, %while.body
  br label %while.body
}

attributes #0 = { sanitize_realtime }

; CHECK: call{{.*}}@__rtsan_expect_not_realtime
; CHECK-NEXT: br label %while.body
