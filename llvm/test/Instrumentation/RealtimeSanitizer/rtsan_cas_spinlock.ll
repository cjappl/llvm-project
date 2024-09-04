; RUN: opt < %s -passes=rtsan -S | FileCheck %s

%class.SpinLockTestAndSet = type { %"struct.std::__1::atomic_flag" }
%"struct.std::__1::atomic_flag" = type { %"struct.std::__1::__cxx_atomic_impl" }
%"struct.std::__1::__cxx_atomic_impl" = type { %"struct.std::__1::__cxx_atomic_base_impl" }
%"struct.std::__1::__cxx_atomic_base_impl" = type { i8 }

define noundef i32 @main() local_unnamed_addr #0 {
entry:
  %spinlock = alloca %class.SpinLockTestAndSet, align 1
  call void @llvm.lifetime.start.p0(i64 1, ptr nonnull %spinlock)
  store i8 0, ptr %spinlock, align 1
  br label %while.cond.i

while.cond.i:                                     ; preds = %while.cond.i, %entry
  %0 = atomicrmw xchg ptr %spinlock, i8 1 acquire, align 1
  %extract.t2.i.i = trunc i8 %0 to i1
  br i1 %extract.t2.i.i, label %while.cond.i, label %SpinlockTestAndSet.exit

SpinlockTestAndSet.exit:              ; preds = %while.cond.i
  store atomic i8 0, ptr %spinlock release, align 1
  call void @llvm.lifetime.end.p0(i64 1, ptr nonnull %spinlock)
  ret i32 0
}

attributes #0 = { sanitize_realtime }

; CHECK: call{{.*}}@__rtsan_expect_not_realtime
