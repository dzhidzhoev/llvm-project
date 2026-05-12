//===- llvm/MC/MCDXContainerWriter.cpp - DXContainer Writer -----*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "llvm/MC/MCDXContainerWriter.h"
#include "llvm/ADT/bit.h"
#include "llvm/BinaryFormat/DXContainer.h"
#include "llvm/DebugInfo/CodeView/GUID.h"
#include "llvm/DebugInfo/MSF/MSFBuilder.h"
#include "llvm/DebugInfo/PDB/Native/InfoStreamBuilder.h"
#include "llvm/DebugInfo/PDB/Native/PDBFileBuilder.h"
#include "llvm/DebugInfo/PDB/Native/RawConstants.h"
#include "llvm/MC/MCAssembler.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCSection.h"
#include "llvm/MC/MCTargetOptions.h"
#include "llvm/MC/MCValue.h"
#include "llvm/Support/Alignment.h"
#include "llvm/Support/EndianStream.h"
#include <algorithm>

using namespace llvm;

MCDXContainerTargetWriter::~MCDXContainerTargetWriter() = default;

static bool skipSection(const MCAssembler &Asm, const MCSection &Sec) {
  // Skip empty and auxiliary sections.
  return Asm.getSectionAddressSize(Sec) == 0 || Sec.getName() == PdbFileNameSectionName || Sec.getName() == ModuleHashSectionName;
}

void DXContainerObjectWriter::writeObject(support::endian::Writer &W, bool IsDebugContainer) {
  auto &Asm = *this->Asm;
  // Start the file size as the header plus the size of the part offsets.
  // Presently DXContainer files usually contain 7-10 parts. Reserving space for
  // 16 part offsets gives us a little room for growth.
  llvm::SmallVector<uint64_t, 16> PartOffsets;
  uint64_t PartOffset = 0;
  for (const MCSection &Sec : Asm) {
    uint64_t SectionSize = Asm.getSectionAddressSize(Sec);
    if (skipSection(Asm, Sec))
      continue;

    assert(SectionSize < std::numeric_limits<uint32_t>::max() &&
           "Section size too large for DXContainer");

    PartOffsets.push_back(PartOffset);
    PartOffset += sizeof(dxbc::PartHeader) + SectionSize;
    PartOffset = alignTo(PartOffset, Align(4ul));
    // The DXIL part also writes a program header, so we need to include its
    // size when computing the offset for a part after the DXIL part.
    if (Sec.getName() == "DXIL" || Sec.getName() == "ILDB")
      PartOffset += sizeof(dxbc::ProgramHeader);
  }
  assert(PartOffset < std::numeric_limits<uint32_t>::max() &&
         "Part data too large for DXContainer");

  uint64_t PartStart =
      sizeof(dxbc::Header) + (PartOffsets.size() * sizeof(uint32_t));
  uint64_t FileSize = PartStart + PartOffset;
  assert(FileSize < std::numeric_limits<uint32_t>::max() &&
         "File size too large for DXContainer");

  // Write the header.
  W.write<char>({'D', 'X', 'B', 'C'});
  // Write 16-bytes of 0's for the hash.
  W.OS.write_zeros(16);
  // Write 1.0 for file format version.
  W.write<uint16_t>(1u);
  W.write<uint16_t>(0u);
  // Write the file size.
  W.write<uint32_t>(static_cast<uint32_t>(FileSize));
  // Write the number of parts.
  W.write<uint32_t>(static_cast<uint32_t>(PartOffsets.size()));
  // Write the offsets for the part headers for each part.
  for (uint64_t Offset : PartOffsets)
    W.write<uint32_t>(static_cast<uint32_t>(PartStart + Offset));

  for (const MCSection &Sec : Asm) {
    uint64_t SectionSize = Asm.getSectionAddressSize(Sec);
    if (skipSection(Asm, Sec))
      continue;

    unsigned Start = W.OS.tell();
    // Write section header.
    W.write<char>(ArrayRef<char>(Sec.getName().data(), 4));

    uint64_t PartSize = SectionSize;

    if (Sec.getName() == "DXIL" || Sec.getName() == "ILDB")
      PartSize += sizeof(dxbc::ProgramHeader);
    // DXContainer parts should be 4-byte aligned.
    PartSize = alignTo(PartSize, Align(4));
    W.write<uint32_t>(static_cast<uint32_t>(PartSize));
    if (Sec.getName() == "DXIL" || Sec.getName() == "ILDB") {
      dxbc::ProgramHeader Header;
      memset(reinterpret_cast<void *>(&Header), 0, sizeof(dxbc::ProgramHeader));

      const Triple &TT = getContext().getTargetTriple();
      VersionTuple Version = TT.getOSVersion();
      uint8_t MajorVersion = static_cast<uint8_t>(Version.getMajor());
      uint8_t MinorVersion =
          static_cast<uint8_t>(Version.getMinor().value_or(0));
      Header.Version =
          dxbc::ProgramHeader::getVersion(MajorVersion, MinorVersion);
      if (TT.hasEnvironment())
        Header.ShaderKind =
            static_cast<uint16_t>(TT.getEnvironment() - Triple::Pixel);

      // The program header's size field is in 32-bit words.
      Header.Size = (SectionSize + sizeof(dxbc::ProgramHeader) + 3) / 4;
      memcpy(Header.Bitcode.Magic, "DXIL", 4);
      VersionTuple DXILVersion = TT.getDXILVersion();
      Header.Bitcode.MajorVersion = DXILVersion.getMajor();
      Header.Bitcode.MinorVersion = DXILVersion.getMinor().value_or(0);
      Header.Bitcode.Offset = sizeof(dxbc::BitcodeHeader);
      Header.Bitcode.Size = SectionSize;
      if (sys::IsBigEndianHost)
        Header.swapBytes();
      W.write<char>(ArrayRef<char>(reinterpret_cast<char *>(&Header),
                                   sizeof(dxbc::ProgramHeader)));
    }
    Asm.writeSectionData(W.OS, &Sec);
    unsigned Size = W.OS.tell() - Start;
    W.OS.write_zeros(offsetToAlignment(Size, Align(4)));
  }
}

uint64_t DXContainerObjectWriter::writeObject() {
  writeObject(W, false);

  StringRef DebugFileName;
  ArrayRef<char> ModuleHash;
  for (const MCSection &Sec : *Asm) {
    if (Sec.getName() == PdbFileNameSectionName) {
      assert(DebugFileName.empty() && "Duplicate PDBNAME section");
      DebugFileName = Sec.begin().F->getContents().data();
    } else if (Sec.getName() == ModuleHashSectionName) {
      assert(ModuleHash.empty() && "Duplicate PBDHASH section");
      ModuleHash = Sec.begin().F->getContents();
    }
  }

  // PDB file was not requested.
  if (DebugFileName.empty())
    return 0;

  BumpPtrAllocator Allocator;
  pdb::PDBFileBuilder Builder(Allocator);

  // DirectXShaderCompiler uses block size 512.
  if (Error Err = Builder.initialize(512))
    reportFatalInternalError(std::move(Err));

  // Reserved streams that should be empty.
  static_assert(pdb::kSpecialStreamCount == 5 && "First 5 streams should be empty in DirectX PDB file");
  for (uint32_t I = 0; I < pdb::kSpecialStreamCount; ++I)
    if (auto Err = Builder.getMsfBuilder().addStream(0).takeError())
      reportFatalInternalError(std::move(Err));

  // Add DXContainer stream.
  if (auto Err = Builder.getMsfBuilder().addStream(0).takeError())
    reportFatalInternalError(std::move(Err));

  // InfoStream must be filled. Bitcode hash from HASH part is used for PDB GUID.
  codeview::GUID PdbGuid;
  assert(ModuleHash.size() == std::size(PdbGuid.Guid) && "Module hash size must be match GUID size");
  std::copy_n(ModuleHash.begin(), std::size(PdbGuid.Guid), PdbGuid.Guid);

  auto &InfoBuilder = Builder.getInfoBuilder();
  InfoBuilder.setAge(1);
  InfoBuilder.setGuid(PdbGuid);
  InfoBuilder.setSignature(0);
  InfoBuilder.setVersion(pdb::PdbRaw_ImplVer::PdbImplVC70);

  // Write DXContainer.
  raw_svector_ostream DebugContainerStream(*Builder.getDXContainerData());
  support::endian::Writer DebugW(DebugContainerStream, llvm::endianness::little);
  // TODO write only necessary sections
  writeObject(DebugW, true);

  // Write PDB file.
  codeview::GUID IgnoredOutGuid;
  if (Error Err = Builder.commit(DebugFileName, &IgnoredOutGuid))
    getContext().reportError(SMLoc(), toString(std::move(Err)));

  return 0;
}
