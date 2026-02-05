//===-- ModuleDebugInfoPrinter.cpp - Prints module debug info metadata ----===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Utils/SubprogramMerge.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Pass.h"
#include "llvm/Transforms/Utils/ValueMapper.h"

using namespace llvm;

static unsigned GetArg(const Metadata *N) {
  if (auto *DV = dyn_cast_or_null<DILocalVariable>(N))
    return DV->getArg();
  return 0;
}

PreservedAnalyses SubprogramMergePass::run(Module &M,
                                                  ModuleAnalysisManager &AM) {
  LexicalScopes LScopes;
  auto IsFunctionAttached = [&LScopes](const DISubprogram *SP) {
    return !!LScopes.getFunction(SP);
  };

  DenseMap<PointerIntPair<MDString *, 1, bool>, SetVector<DISubprogram *>> InlinedOutSPMap;
  {
    DebugInfoFinder Finder;

    // TODO should we process only functions to reduce memory usage?
    Finder.processModule(M);
    LScopes.initialize(M);

    for (DISubprogram *SP : Finder.subprograms())
      if (SP->isDefinition() && SP->getRawLinkageName() && !LScopes.getFunction(SP))
        InlinedOutSPMap[{SP->getRawLinkageName(), SP->getKeyInstructionsEnabled()}].insert(SP);
  }

  ValueToValueMapTy VM;
  SmallVector<DISubprogram *> TargetSPs;
  for (auto &[_, Fs] : InlinedOutSPMap) {
    if (Fs.size() <= 1)
      continue;

    DISubprogram *Target = *Fs.begin();
    auto I = std::find_if(Fs.begin(), Fs.end(), IsFunctionAttached);
    if (I != Fs.end())
      Target = *I;
    TargetSPs.push_back(Target);

    for (DISubprogram *Src : Fs) {
      if (Src == Target)
        continue;
      VM.MD()[Src].reset(Target);

      // for (auto *N : Src->getRetainedNodes()) {
      //   if (unsigned ArgNum = GetArg(N)) {
      //     auto [I, Inserted] = Args.insert({ArgNum, N});
      //     if (!Inserted) {
      //       VM.MD()[N].reset(I->second);
      //       continue;
      //     }
      //   }
      // }
    }
  }
  InlinedOutSPMap.clear();

  if (NamedMDNode *CUNodes = M.getNamedMetadata("llvm.dbg.cu")) {
    for (unsigned I = 0, E = CUNodes->getNumOperands(); I != E; ++I) {
      auto *CU = cast<DICompileUnit>(CUNodes->getOperand(I));
      CUNodes->setOperand(I, MapMetadata(CU, VM, RF_IgnoreMissingLocals | RF_ReuseAndMutateDistinctMDs));
    }
  }

  DebugInfoFinder Finder(true);
  DenseMap<unsigned, DILocalVariable *> Args;

  for (Function &F : M) {
    Finder.processFunction(F);
    // llvm::errs() << "\nRemapping " << F.getName() << "\n";

    for (auto &[SP, LVs] : Finder.local_variables()) {
      DISubprogram *TargetSP = cast_or_null<DISubprogram>(VM.MD().lookup(SP));
      if (!TargetSP)
        continue;
      // llvm::errs() << "Remapping local vars ";
      // SP->dump();

      for (auto *N : TargetSP->getRetainedNodes())
        if (auto *LV = dyn_cast<DILocalVariable>(N))
          if (unsigned ArgNum = GetArg(LV))
            Args[ArgNum] = LV;

      for (DILocalVariable *LV : LVs)
        if (unsigned ArgNum = GetArg(LV)) {
          // llvm::errs() << "Checking " << LV << "\n";
          auto Entry = Args.insert({ArgNum, LV});
          if (!Entry.second && Entry.first->second != LV) {
            // llvm::errs() << "Will remap " << LV << " to " << Entry.first->second << "\n";
            VM.MD()[LV].reset(Entry.first->second);
          }
        }
    }
    Finder.reset();

    RemapFunction(F, VM, RF_IgnoreMissingLocals | RF_ReuseAndMutateDistinctMDs);

    Args.clear();
  }

  PreservedAnalyses PA;
  PA.preserveSet<CFGAnalyses>();
  return PA;
}
