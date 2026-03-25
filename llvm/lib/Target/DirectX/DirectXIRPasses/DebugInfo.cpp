//===--- DebugInfo.cpp - analysis and lowering for Debug info -*- C++ -*- ---=//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "DebugInfo.h"
#include "llvm/BinaryFormat/Dwarf.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/Module.h"

#define DEBUG_TYPE "dx-debug-info"

using namespace llvm;

DebugInfoMap DebugInfoPass::run(Module &M) {
  DebugInfoMap Res;
  DebugInfoFinder DIF;
  DIF.processModule(M);

  for (auto *GVE : DIF.global_variables())
    Res.VEReplace.insert({GVE, GVE->getVariable()});

  for (auto *S : DIF.scopes()) {
    if (auto *CB = dyn_cast<DICommonBlock>(S))
      Res.VEReplace.insert({CB, CB->getScope()});
  }

  for (auto *T : DIF.types()) {
    if (auto *SR = dyn_cast<DISubrangeType>(T)) {
      auto *BT = SR->getBaseType();
      if (!BT)
        BT = DIBasicType::get(
            SR->getContext(), dwarf::DW_TAG_base_type, SR->getName(),
            SR->getSizeInBits(), SR->getAlignInBits(), dwarf::DW_ATE_unsigned,
            SR->getNumExtraInhabitants(), /*DataSizeInBits=*/0, SR->getFlags());
      Res.VEReplace.insert({T, BT});
    }
  }

  return Res;
}
