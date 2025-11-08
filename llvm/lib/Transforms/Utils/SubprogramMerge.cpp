//===-- ModuleDebugInfoPrinter.cpp - Prints module debug info metadata ----===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This pass decodes the debug info metadata in a module and prints in a
// (sufficiently-prepared-) human-readable form.
//
// For example, run this pass from opt along with the -analyze option, and
// it'll print to standard output.
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

    MapVector<unsigned, Metadata *> Args;
    for (auto *N : Target->getRetainedNodes())
      if (unsigned ArgNum = GetArg(N))
        Args.insert({ArgNum, N});

    SmallVector<Metadata *> MergedRetainedNodes(Target->getRetainedNodes().begin(), Target->getRetainedNodes().end());
    for (DISubprogram *Src : Fs) {
      if (Src == Target)
        continue;
      VM.MD()[Src].reset(Target);

      for (auto *N : Src->getRetainedNodes()) {
        if (unsigned ArgNum = GetArg(N)) {
          auto [I, Inserted] = Args.insert({ArgNum, N});
          if (!Inserted) {
            VM.MD()[N].reset(I->second);
            continue;
          }
        }
        MergedRetainedNodes.push_back(N);
      }
    }
    Target->replaceRetainedNodes(MDNode::get(M.getContext(), MergedRetainedNodes));
  }
  InlinedOutSPMap.clear();

  if (NamedMDNode *CUNodes = M.getNamedMetadata("llvm.dbg.cu")) {
    for (unsigned I = 0, E = CUNodes->getNumOperands(); I != E; ++I) {
      auto *CU = cast<DICompileUnit>(CUNodes->getOperand(I));
      CUNodes->setOperand(I, MapMetadata(CU, VM, RF_IgnoreMissingLocals | RF_ReuseAndMutateDistinctMDs));
    }
  }
  for (Function &F : M) {
    RemapFunction(F, VM, RF_IgnoreMissingLocals | RF_ReuseAndMutateDistinctMDs);
  }

  for (DISubprogram *SP : TargetSPs) {
    SetVector<Metadata *> RetainedNodes(SP->getRetainedNodes().begin(), SP->getRetainedNodes().end());
    SP->replaceRetainedNodes(MDNode::get(M.getContext(), RetainedNodes.getArrayRef()));
  }

  PreservedAnalyses PA;
  PA.preserveSet<CFGAnalyses>();
  return PA;
}
