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
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicsDirectX.h"
#include "llvm/IR/Module.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

#include <map>

#define DEBUG_TYPE "dx-debug-info"

using namespace llvm;

// llvm.dbg.value has an additional "offset" operand in DXIL. Transform
// llvm.dbg.value it to llvm.dx.dbg.value and add zero offset.
static void replaceDbgVariableIntr(DbgVariableIntrinsic *DVI, Function *NewF) {
  if (DVI->getIntrinsicID() != Intrinsic::dbg_value) {
    return;
  }

  Type *Int64Ty = Type::getInt64Ty(DVI->getContext());
  Constant *ZeroOffset = ConstantInt::get(Int64Ty, 0);

  Value *NewOps[] = {
      DVI->getOperand(0),
      ZeroOffset,
      DVI->getOperand(1),
      DVI->getOperand(2),
  };

  CallInst *NewI = CallInst::Create(NewF->getFunctionType(), NewF, NewOps);
  ReplaceInstWithInst(DVI, NewI);
}

static void replaceDbgValue(Module &M) {
  Function *F = getDeclarationIfExists(&M, Intrinsic::dbg_value);
  if (!F)
    return;

  if (F->getNumUses() == 0) {
    F->eraseFromParent();
    return;
  }

  Function *NewF = getOrInsertDeclaration(&M, Intrinsic::dx_dbg_value);

  for (User *U : make_early_inc_range(F->users())) {
    auto *DVI = cast<DbgVariableIntrinsic>(U);
    replaceDbgVariableIntr(DVI, NewF);
  }

  // Remove conflicting declarations.
  F->eraseFromParent();
  NewF->setName("llvm.dbg.value");
}

DebugInfoMap DebugInfoPass::run(Module &M) {
  DebugInfoMap Res;
  DebugInfoFinder DIF;
  DIF.processModule(M);

  // Replace llvm.dbg.value with equivalent DXIL intrinsics.
  replaceDbgValue(M);

  std::multimap<const DICompileUnit *, const Metadata *> CUSubprograms;

  for (const Function &F : M) {
    if (const DISubprogram *SP = F.getSubprogram()) {
      auto *FunctionMD = ConstantAsMetadata::get(const_cast<Function *>(&F));
      Res.VEExtra.insert({SP, FunctionMD});
    }
  }

  for (const DISubprogram *SP : DIF.subprograms())
    if (SP->getUnit())
      CUSubprograms.insert({SP->getUnit(), SP});

  for (auto It = CUSubprograms.begin(), End = CUSubprograms.end();
       It != End;) {
    auto *CU = It->first;
    auto CUEnd = CUSubprograms.upper_bound(CU);
    SmallVector<Metadata *, 16> Subprograms;
    do
      Subprograms.push_back(const_cast<Metadata *>(It->second));
    while (++It != CUEnd);
    auto *SubprogramMD = MDTuple::get(M.getContext(), Subprograms);
    Res.VEExtra.insert({CU, SubprogramMD});
  }

  for (const GlobalVariable &GV : M.globals()) {
    SmallVector<DIGlobalVariableExpression *, 4> GVEs;
    GV.getDebugInfo(GVEs);
    for (auto *GVE : GVEs) {
      if (GVE->getExpression()->getNumElements())
        continue;
      auto [It, Inserted] = Res.VEExtra.insert(
          {GVE->getVariable(),
           ValueAsMetadata::get(const_cast<GlobalVariable *>(&GV))});
      if (!Inserted)
        It->second = nullptr;
    }
  }

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
