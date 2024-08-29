; RUN: opt < %s -passes='rtsan' -S | FileCheck %s


define void @procces(ptr noundef %buffer, i32 noundef %size) #0 {
entry:
  %buffer.addr = alloca ptr, align 8
  %size.addr = alloca i32, align 4
  %i = alloca i32, align 4
  store ptr %buffer, ptr %buffer.addr, align 8
  store i32 %size, ptr %size.addr, align 4
  store i32 0, ptr %i, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %0 = load i32, ptr %i, align 4
  %1 = load i32, ptr %size.addr, align 4
  %cmp = icmp slt i32 %0, %1
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %2 = load i32, ptr %i, align 4
  %conv = sitofp i32 %2 to float
  %3 = load ptr, ptr %buffer.addr, align 8
  %4 = load i32, ptr %i, align 4
  %idxprom = sext i32 %4 to i64
  %arrayidx = getelementptr inbounds float, ptr %3, i64 %idxprom
  store float %conv, ptr %arrayidx, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %5 = load i32, ptr %i, align 4
  %inc = add nsw i32 %5, 1
  store i32 %inc, ptr %i, align 4
  br label %for.cond

for.end:                                          ; preds = %for.cond
  ret void
}

attributes #0 = { sanitize_realtime }

; In this simple loop, we should not insert rtsan_expect_not_realtime
; CHECK: call{{.*}}@__rtsan_realtime_enter

; TODO: This test fails when it shouldn't!!
; XXXXX--CHECK-NOT: call{{.*}}@__rtsan_expect_not_realtime

; CHECK: call{{.*}}@__rtsan_realtime_exit
