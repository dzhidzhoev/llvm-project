//===- llvm/MC/DXContainerSourceInfo.h - DXContainer SourceInfo -*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_MC_DXCONTAINERSOURCEINFO_H
#define LLVM_MC_DXCONTAINERSOURCEINFO_H

#include "llvm/ADT/SmallString.h"
#include "llvm/BinaryFormat/DXContainer.h"
#include "llvm/Object/DXContainer.h"

namespace llvm {

class raw_ostream;

namespace mcdxbc {

/// This data structure is a helper for writing SourceInfo data.
/// It is implemented in the MC library so that it can be used by both
/// the MC layer and Object tools.
struct SourceInfo {
  bool IsFinalized = false;
  object::DirectX::SourceInfo BaseData;
  SmallString<128> CompressedContents;

  void setCompressionType(dxbc::SourceInfo::Contents::CompressionType Type) {
    CompressionType = Type;
  }

  void addFile(StringRef Name, StringRef Content) {
    Override = true;
    FileNamesAndContents.emplace_back(Name, Content);
  }
  void addArg(StringRef Arg) {
    Override = true;
    Args.push_back(Arg);
  }

  void write(raw_ostream &OS) const;
  void finalize();

private:
  bool Override = false;
  std::optional<dxbc::SourceInfo::Contents::CompressionType> CompressionType;
  SmallVector<std::pair<StringRef, StringRef>> FileNamesAndContents;
  SmallVector<StringRef> Args;

  void compute();
};

struct DebugName {
  object::DXContainer::ILDNData BaseData;

  DebugName() { BaseData.first.Flags = 0; }

  void setFileName(StringRef FileName);
  void write(raw_ostream &OS) const;
};

} // namespace mcdxbc
} // namespace llvm

#endif // LLVM_MC_DXCONTAINERSOURCEINFO_H
