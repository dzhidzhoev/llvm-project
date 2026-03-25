//===----- DebugInfo.h - analysis and lowering for Debug info -*- C++ -*- -===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// \file Analyze and downgrade debug info metadata to match DXIL (LLVM 3.7).
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_DIRECTX_DEBUGINFO_H
#define LLVM_LIB_TARGET_DIRECTX_DEBUGINFO_H

#include "llvm/ADT/DenseMap.h"

namespace llvm {

class Module;
class Metadata;

class DebugInfoMap {
public:
  using MDMap = DenseMap<const Metadata *, const Metadata *>;

  /// Enumerate extra metadata when Key is encountered in ValueEnumerator.
  MDMap VEExtra;

  /// Completely replace one metadata with another in ValueEnumerator.
  MDMap VEReplace;
};

namespace DebugInfoPass {

DebugInfoMap run(Module &M);

} // namespace DebugInfoPass
} // namespace llvm

#endif // LLVM_LIB_TARGET_DIRECTX_DEBUGINFO_H
