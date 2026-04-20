//===- unittests/Driver/EscapedCommandLineTest.cpp ------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Unit tests for escapeSpacesAndBackslashes, parseEscapedCommandLine and
// renderEscapedCommandLine.
//
//===----------------------------------------------------------------------===//

#include "clang/Basic/DiagnosticIDs.h"
#include "clang/Basic/DiagnosticOptions.h"
#include "clang/Driver/Compilation.h"
#include "clang/Driver/CommonArgs.h"
#include "clang/Driver/Driver.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/VirtualFileSystem.h"
#include "gtest/gtest.h"
#include <memory>

#include "SimpleDiagnosticConsumer.h"

using namespace clang;
using namespace clang::driver;
using namespace clang::driver::tools;
using namespace llvm;

static std::string escape(const char *Arg) {
  SmallString<64> Res;
  escapeSpacesAndBackslashes(Arg, Res);
  return std::string(Res);
}

// ---------------------------------------------------------------------------
// escapeSpacesAndBackslashes
// ---------------------------------------------------------------------------

TEST(EscapedCommandLineTest, EscapeEmpty) {
  EXPECT_EQ(escape(""), "");
}

TEST(EscapedCommandLineTest, EscapeNoSpecialChars) {
  EXPECT_EQ(escape("hello"), "hello");
  EXPECT_EQ(escape("-Tlib_6_3"), "-Tlib_6_3");
}

TEST(EscapedCommandLineTest, EscapeSpace) {
  EXPECT_EQ(escape("foo bar"), "foo\\ bar");
  EXPECT_EQ(escape(" leading"), "\\ leading");
  EXPECT_EQ(escape("trailing "), "trailing\\ ");
}

TEST(EscapedCommandLineTest, EscapeBackslash) {
  EXPECT_EQ(escape("a\\b"), "a\\\\b");
}

TEST(EscapedCommandLineTest, EscapeSpaceAndBackslash) {
  EXPECT_EQ(escape("a\\ b"), "a\\\\\\ b");
}

// ---------------------------------------------------------------------------
// parseEscapedCommandLine
// ---------------------------------------------------------------------------

static SmallVector<std::string> parse(StringRef CommandLine) {
  SmallVector<std::string> Res;
  for (const auto &Arg : parseEscapedCommandLine(CommandLine))
    Res.emplace_back(Arg.begin(), Arg.end());
  return Res;
}

TEST(EscapedCommandLineTest, ParseEmpty) {
  EXPECT_TRUE(parse("").empty());
}

TEST(EscapedCommandLineTest, ParseSingleArg) {
  EXPECT_EQ(parse("hello"), SmallVector<std::string>({"hello"}));
}

TEST(EscapedCommandLineTest, ParseMultipleArgs) {
  auto Args = parse("clang -Tlib_6_3 foo.hlsl");
  ASSERT_EQ(Args.size(), 3u);
  EXPECT_EQ(Args[0], "clang");
  EXPECT_EQ(Args[1], "-Tlib_6_3");
  EXPECT_EQ(Args[2], "foo.hlsl");
}

TEST(EscapedCommandLineTest, ParseEscapedSpace) {
  auto Args = parse("foo\\ bar baz");
  ASSERT_EQ(Args.size(), 2u);
  EXPECT_EQ(Args[0], "foo bar");
  EXPECT_EQ(Args[1], "baz");
}

TEST(EscapedCommandLineTest, ParseEscapedBackslash) {
  auto Args = parse("a\\\\b");
  ASSERT_EQ(Args.size(), 1u);
  EXPECT_EQ(Args[0], "a\\b");
}

// ---------------------------------------------------------------------------
// Round-trip: renderEscapedCommandLine then parseEscapedCommandLine
// ---------------------------------------------------------------------------

// Build a minimal driver with exec path "/bin/clang" and an in-memory file
// system containing "foo.cpp", then render ArgStrings through
// renderEscapedCommandLine and parse the result back.
static SmallVector<std::string>
renderAndParse(ArrayRef<const char *> ArgStrings) {
  auto FS = llvm::makeIntrusiveRefCnt<llvm::vfs::InMemoryFileSystem>();
  FS->addFile("foo.cpp", 0, llvm::MemoryBuffer::getMemBuffer(""));
  DiagnosticOptions DiagOpts;
  DiagnosticsEngine Diags(DiagnosticIDs::create(), DiagOpts,
                          new SimpleDiagnosticConsumer);
  Driver TheDriver("/bin/clang", "x86_64-unknown-linux-gnu", Diags, "clang",
                   FS);
  std::unique_ptr<Compilation> C(
      TheDriver.BuildCompilation({"clang", "foo.cpp"}));
  if (!C)
    return {};

  bool ContainsError = false;
  auto Args = TheDriver.ParseArgStrings(ArgStrings, false, ContainsError);
  if (ContainsError)
    return {};

  const char *Rendered =
      renderEscapedCommandLine(C->getDefaultToolChain(), Args);
  SmallVector<std::string> Result;
  for (const auto &S : parseEscapedCommandLine(Rendered))
    Result.emplace_back(S.begin(), S.end());
  return Result;
}

TEST(EscapedCommandLineTest, RoundTripSimple) {
  auto Args = renderAndParse({"-O2"});
  ASSERT_EQ(Args.size(), 2u);
  EXPECT_EQ(Args[0], "/bin/clang");
  EXPECT_EQ(Args[1], "-O2");
}

TEST(EscapedCommandLineTest, RoundTripMultipleArgs) {
  auto Args = renderAndParse({"-O2", "-DFOO"});
  ASSERT_EQ(Args.size(), 3u);
  EXPECT_EQ(Args[0], "/bin/clang");
  EXPECT_EQ(Args[1], "-O2");
  EXPECT_EQ(Args[2], "-DFOO");
}

TEST(EscapedCommandLineTest, RoundTripArgWithSpace) {
  // -isystem is a separate arg; its value "/path with spaces" contains spaces.
  auto Args = renderAndParse({"-isystem", "/path with spaces"});
  ASSERT_EQ(Args.size(), 3u);
  EXPECT_EQ(Args[0], "/bin/clang");
  EXPECT_EQ(Args[1], "-isystem");
  EXPECT_EQ(Args[2], "/path with spaces");
}

TEST(EscapedCommandLineTest, RoundTripArgWithBackslash) {
  auto Args = renderAndParse({"-isystem", "C:\\include"});
  ASSERT_EQ(Args.size(), 3u);
  EXPECT_EQ(Args[0], "/bin/clang");
  EXPECT_EQ(Args[1], "-isystem");
  EXPECT_EQ(Args[2], "C:\\include");
}

TEST(EscapedCommandLineTest, RoundTripArgWithSpaceAndBackslash) {
  auto Args = renderAndParse({"-isystem", "C:\\path with space\\include"});
  ASSERT_EQ(Args.size(), 3u);
  EXPECT_EQ(Args[0], "/bin/clang");
  EXPECT_EQ(Args[1], "-isystem");
  EXPECT_EQ(Args[2], "C:\\path with space\\include");
}
