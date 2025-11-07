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
  for (auto &[_, Fs] : InlinedOutSPMap) {
    if (Fs.size() <= 1)
      continue;

    DISubprogram *Target = *Fs.begin();
    auto I = std::find_if(Fs.begin(), Fs.end(), IsFunctionAttached);
    if (I != Fs.end())
      Target = *I;

    SetVector<Metadata *> MergedRetainedNodes(Target->getRetainedNodes().begin(), Target->getRetainedNodes().end());
    for (DISubprogram *Src : Fs) {
      if (Src != Target)
        VM.MD()[Src].reset(Target);
      for (auto *N : Src->getRetainedNodes())
        MergedRetainedNodes.insert(N);
    }
    Target->replaceRetainedNodes(MDNode::get(M.getContext(), MergedRetainedNodes.getArrayRef()));
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

  PreservedAnalyses PA;
  PA.preserveSet<CFGAnalyses>();
  return PA;
}
