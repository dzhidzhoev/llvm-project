//===- llvm/MC/DXContainerInfo.cpp - DXContainer Info -----*- C++ -------*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "llvm/ADT/SmallString.h"
#include "llvm/BinaryFormat/DXContainer.h"
#include "llvm/MC/DXContainerSourceInfo.h"
#include "llvm/Object/DXContainer.h"
#include "llvm/Support/Compression.h"
#include "llvm/Support/EndianStream.h"
#include "llvm/Support/FormatVariadic.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Support/SwapByteOrder.h"
#include <type_traits>

using namespace llvm;
using namespace llvm::mcdxbc;

static uint64_t align(uint64_t Size) {
  return alignTo(Size, dxbc::DXCONTAINER_STRUCT_ALIGNMENT);
}

template <typename StructT>
static void writeStruct(raw_ostream &OS, StructT S) {
  static_assert(std::is_class<StructT>() &&
                "This method must be used for writing structure types.");
  if (sys::IsBigEndianHost)
    S.swapBytes();
  OS.write(reinterpret_cast<const char *>(&S), sizeof(StructT));
}

static void writeString(raw_ostream &OS, StringRef S) {
  OS.write(S.data(), S.size());
  // Write null terminator.
  OS.write_zeros(1);
}

static void writePadding(raw_ostream &OS, uint64_t Prev) {
  uint64_t UnpaddedSize = OS.tell() - Prev;
  uint64_t Padding = align(UnpaddedSize) - UnpaddedSize;
  if (Padding)
    OS.write_zeros(Padding);
}

void SourceInfo::compute() {
  assert(CompressionType && "Compression type must be set.");

  BaseData.Parameters.SectionCount = 3;
  BaseData.Parameters.Flags = 0;

  BaseData.Names.GenericHeader.Flags = 0;
  BaseData.Names.GenericHeader.Type =
      dxbc::SourceInfo::SectionType::SourceNames;
  BaseData.Names.Parameters.Flags = 0;
  BaseData.Names.Parameters.Count = FileNamesAndContents.size();
  BaseData.Names.Entries.clear();
  BaseData.Names.Entries.reserve(BaseData.Names.Parameters.Count);

  BaseData.Contents.GenericHeader.Flags = 0;
  BaseData.Contents.GenericHeader.Type =
      dxbc::SourceInfo::SectionType::SourceContents;
  BaseData.Contents.Parameters.Flags = 0;
  BaseData.Contents.Parameters.Type = *CompressionType;
  BaseData.Contents.Parameters.Count = FileNamesAndContents.size();
  BaseData.Contents.Entries.clear();
  BaseData.Contents.Entries.reserve(BaseData.Contents.Parameters.Count);

  size_t NameEntriesSizeInBytes = 0;
  size_t ContentEntriesSizeInBytes = 0;
  for (const auto &NameContent : FileNamesAndContents) {
    object::DirectX::SourceInfo::SourceNames::Entry NameEntry;
    NameEntry.FileName = NameContent.first;
    NameEntry.Parameters.ContentSizeInBytes = NameContent.second.size() + 1;
    NameEntry.Parameters.NameSizeInBytes = NameEntry.FileName.size() + 1;
    NameEntry.Parameters.Flags = 0;
    NameEntry.Parameters.AlignedSizeInBytes =
        align(NameEntry.Parameters.NameSizeInBytes +
              sizeof(dxbc::SourceInfo::Names::Entry));
    NameEntriesSizeInBytes += NameEntry.Parameters.AlignedSizeInBytes;

    object::DirectX::SourceInfo::SourceContents::Entry ContentEntry;
    ContentEntry.FileContent = NameContent.second.str();
    ContentEntry.Parameters.ContentSizeInBytes =
        NameEntry.Parameters.ContentSizeInBytes;
    ContentEntry.Parameters.Flags = 0;
    ContentEntry.Parameters.AlignedSizeInBytes =
        align(ContentEntry.Parameters.ContentSizeInBytes +
              sizeof(dxbc::SourceInfo::Contents::Entry));
    ContentEntriesSizeInBytes += ContentEntry.Parameters.AlignedSizeInBytes;

    BaseData.Names.Entries.emplace_back(std::move(NameEntry));
    BaseData.Contents.Entries.push_back(std::move(ContentEntry));
  }
  BaseData.Names.Parameters.EntriesSizeInBytes = align(NameEntriesSizeInBytes);
  BaseData.Names.GenericHeader.updateSize(
      BaseData.Names.Parameters.EntriesSizeInBytes +
      sizeof(dxbc::SourceInfo::Names::HeaderOnDisk));
  BaseData.Contents.Parameters.UncompressedEntriesSizeInBytes =
      align(ContentEntriesSizeInBytes);

  BaseData.Args.GenericHeader.Flags = 0;
  BaseData.Args.GenericHeader.Type = dxbc::SourceInfo::SectionType::Args;
  BaseData.Args.Parameters.Flags = 0;
  BaseData.Args.Parameters.Count = Args.size();
  BaseData.Args.Args.clear();
  BaseData.Args.Args.reserve(BaseData.Args.Parameters.Count);
  size_t ArgEntriesSizeInBytes = 0;
  for (StringRef Arg : Args) {
    BaseData.Args.Args.emplace_back(Arg, "");
    // Null-terminated argument name and empty null-terminated argument value.
    ArgEntriesSizeInBytes += Arg.size() + 1 + 1;
  }
  BaseData.Args.Parameters.SizeInBytes = ArgEntriesSizeInBytes;
  BaseData.Args.GenericHeader.updateSize(
      BaseData.Args.Parameters.SizeInBytes +
      sizeof(dxbc::SourceInfo::Args::Header));
}

void SourceInfo::finalize() {
  if (Override)
    compute();

  IsFinalized = true;

  // Compress Contents right here, to calculate compressed size.
  CompressedContents.clear();
  SmallString<256> Data;
  {
    raw_svector_ostream OS(Data);
    for (auto &E : BaseData.Contents.Entries) {
      uint64_t EntryOffset = OS.tell();
      writeStruct(OS, E.Parameters);
      writeString(OS, E.FileContent);
      writePadding(OS, EntryOffset);
    }
    writePadding(OS, 0);
  }
  switch (BaseData.Contents.Parameters.Type) {
  case dxbc::SourceInfo::Contents::CompressionType::Zlib: {
    if (!compression::zlib::isAvailable())
      reportFatalUsageError(Twine("DXContainer SRCI Contents should be "
                                  "compressed with Zlib, but ") +
                            Twine(compression::getReasonIfUnsupported(
                                compression::Format::Zlib)));

    SmallVector<uint8_t, 128> CompressedData;
    compression::zlib::compress(
        ArrayRef(reinterpret_cast<uint8_t *>(Data.data()), Data.size()),
        CompressedData, compression::zlib::BestSizeCompression);
    raw_svector_ostream OS(CompressedContents);
    OS.write(reinterpret_cast<char *>(CompressedData.data()),
             CompressedData.size());
    if (Override)
      BaseData.Contents.Parameters.EntriesSizeInBytes =
          CompressedContents.size();
    break;
  }
  case dxbc::SourceInfo::Contents::CompressionType::None: {
    CompressedContents = std::move(Data);
    if (Override)
      BaseData.Contents.Parameters.EntriesSizeInBytes =
          align(CompressedContents.size());
    break;
  }
  }
  if (BaseData.Contents.Parameters.EntriesSizeInBytes !=
      CompressedContents.size())
    reportFatalUsageError(
        formatv("DXContainer SRCI Contents compressed size in header ({0}) "
                "doesn't match the actual compressed size ({1})",
                BaseData.Contents.Parameters.EntriesSizeInBytes,
                CompressedContents.size()));

  if (Override) {
    BaseData.Contents.Parameters.AlignedSizeInBytes =
        align(BaseData.Contents.Parameters.EntriesSizeInBytes +
              sizeof(dxbc::SourceInfo::Contents::Header));
    BaseData.Contents.GenericHeader.updateSize(
        BaseData.Contents.Parameters.AlignedSizeInBytes);
    BaseData.Parameters.AlignedSizeInBytes =
        align(sizeof(dxbc::SourceInfo::Header) +
              BaseData.Names.GenericHeader.AlignedSizeInBytes +
              BaseData.Contents.GenericHeader.AlignedSizeInBytes +
              BaseData.Args.GenericHeader.AlignedSizeInBytes);
  }
}

void SourceInfo::write(raw_ostream &OS) const {
  assert(IsFinalized &&
         "SourceInfo::finalize must be called before SourceInfo::write");

  writeStruct(OS, BaseData.Parameters);

  // Write Names section.
  {
    auto &Names = BaseData.Names;
    uint64_t NamesOffset = OS.tell();
    writeStruct(OS, Names.GenericHeader);
    support::endian::write(OS, Names.Parameters.Flags, endianness::little);
    support::endian::write(OS, Names.Parameters.Count, endianness::little);
    support::endian::write(OS, Names.Parameters.EntriesSizeInBytes,
                           endianness::little);
    for (auto &E : Names.Entries) {
      uint64_t EntryOffset = OS.tell();
      writeStruct(OS, E.Parameters);
      writeString(OS, E.FileName);
      writePadding(OS, EntryOffset);
    }
    writePadding(OS, NamesOffset);
  }

  // Write Contents section.
  {
    auto &Contents = BaseData.Contents;
    uint64_t ContentsOffset = OS.tell();
    writeStruct(OS, Contents.GenericHeader);
    writeStruct(OS, Contents.Parameters);
    OS.write(CompressedContents.data(), CompressedContents.size());
    writePadding(OS, ContentsOffset);
  }

  // Write Args section.
  {
    auto &Args = BaseData.Args;
    uint64_t ArgsOffset = OS.tell();
    writeStruct(OS, Args.GenericHeader);
    writeStruct(OS, Args.Parameters);
    for (auto &E : Args.Args) {
      writeString(OS, E.first);
      writeString(OS, E.second);
    }
    writePadding(OS, ArgsOffset);
  }
}
