//===- RealtimeSanitizer.cpp - RealtimeSanitizer instrumentation *- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file is a part of the RealtimeSanitizer, an LLVM transformation for
// detecting and reporting realtime safety violations.
//
// See also: llvm-project/compiler-rt/lib/rtsan/
//
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Demangle/Demangle.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/raw_ostream.h"

#include "llvm/Transforms/Instrumentation/RealtimeSanitizer.h"

using namespace llvm;

static void insertCallBeforeInstruction(Function &CallingFn,
                                        IRBuilder<> &Builder,
                                        const char *FunctionName,
                                        ArrayRef<Value *> FunctionArgs) {
  std::vector<Type *> FunctionArgTypes;
  FunctionArgTypes.reserve(FunctionArgs.size());
  for (Value *Arg : FunctionArgs)
    FunctionArgTypes.push_back(Arg->getType());

  FunctionType *FuncType = FunctionType::get(
      Type::getVoidTy(CallingFn.getContext()), FunctionArgTypes, false);
  FunctionCallee Func =
      CallingFn.getParent()->getOrInsertFunction(FunctionName, FuncType);
  Builder.CreateCall(Func, FunctionArgs);
}

static void insertCallAtFunctionEntryPoint(Function &Fn,
                                           const char *InsertFnName) {

  IRBuilder<> Builder{&Fn.front().front()};
  insertCallBeforeInstruction(Fn, Builder, InsertFnName, std::nullopt);
}

static void insertCallAtAllFunctionExitPoints(Function &Fn,
                                              const char *InsertFnName) {
  for (auto &BB : Fn)
    for (auto &I : BB)
      if (isa<ReturnInst>(&I)) {
        IRBuilder<> Builder{&I};
        insertCallBeforeInstruction(Fn, Builder, InsertFnName, std::nullopt);
      }
}

RealtimeSanitizerPass::RealtimeSanitizerPass(
    const RealtimeSanitizerOptions &Options) {}

PreservedAnalyses RealtimeSanitizerPass::run(Function &F,
                                             AnalysisManager<Function> &AM) {
  PreservedAnalyses PA = PreservedAnalyses::all();
  if (F.hasFnAttribute(Attribute::SanitizeRealtime)) {
    insertCallAtFunctionEntryPoint(F, "__rtsan_realtime_enter");
    insertCallAtAllFunctionExitPoints(F, "__rtsan_realtime_exit");
    PA = PreservedAnalyses::none();
    PA.preserveSet<CFGAnalyses>();
  }

  const LoopInfo &LI = AM.getResult<LoopAnalysis>(F);
  ScalarEvolution &SE = AM.getResult<ScalarEvolutionAnalysis>(F);
  for (Loop *L : LI) {
      BasicBlock *Context =
          L->getLoopPreheader() ? L->getLoopPreheader() : L->getHeader();
      assert(Context && "Loop has no preheader or header block");

    const bool HasNoExits = L->hasNoExitBlocks();
    const bool CannotPredictLoopCount =
        isa<SCEVCouldNotCompute>(SE.getConstantMaxBackedgeTakenCount(L)) ||
        isa<SCEVCouldNotCompute>(SE.getBackedgeTakenCount(L));
    const bool LoopIsPotentiallyUnbound = HasNoExits || CannotPredictLoopCount;

    if (LoopIsPotentiallyUnbound) {
      IRBuilder<> Builder{&Context->back()};


    std::string ReasonStr =
        demangle(F.getName().str()) + " contains a possibly unbounded loop ";

      if (HasNoExits)
        ReasonStr += "(reason: no exit blocks).";
      else if (CannotPredictLoopCount)
        ReasonStr += "(reason: backedge taken count cannot be computed).";
      else
        assert(false);

      Value *Reason = Builder.CreateGlobalStringPtr(ReasonStr);
      insertCallBeforeInstruction(F, Builder, "__rtsan_expect_not_realtime",
                                  {Reason});

      // TODO: What is preserved here??
      PA = PreservedAnalyses::none();
    }
  }

  return PA;
}
