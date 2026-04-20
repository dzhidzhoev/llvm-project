//===- unittests/Driver/EscapedCommandLineTest.cpp ------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Unit tests for escapeSpacesAndBackslashes and parseEscapedCommandLine.
//
//===----------------------------------------------------------------------===//

#include "clang/Driver/CommonArgs.h"
#include "llvm/ADT/SmallString.h"
#include "gtest/gtest.h"

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
// Round-trip: escape then parse
// ---------------------------------------------------------------------------

static std::string roundTrip(std::initializer_list<const char *> Args) {
  SmallString<256> Joined;
  bool First = true;
  for (const char *Arg : Args) {
    if (!First)
      Joined += ' ';
    escapeSpacesAndBackslashes(Arg, Joined);
    First = false;
  }
  auto Parsed = parse(Joined);
  SmallString<256> Result;
  for (size_t I = 0; I < Parsed.size(); ++I) {
    if (I)
      Result += ' ';
    Result += Parsed[I];
  }
  return std::string(Result);
}

TEST(EscapedCommandLineTest, RoundTripSimple) {
  EXPECT_EQ(roundTrip({"clang", "-O2", "foo.cpp"}), "clang -O2 foo.cpp");
}

TEST(EscapedCommandLineTest, RoundTripArgWithSpace) {
  EXPECT_EQ(roundTrip({"clang", "path with spaces/file.cpp"}),
            "clang path with spaces/file.cpp");
}

TEST(EscapedCommandLineTest, RoundTripArgWithBackslash) {
  EXPECT_EQ(roundTrip({"clang", "C:\\path\\file.cpp"}),
            "clang C:\\path\\file.cpp");
}

TEST(EscapedCommandLineTest, RoundTripArgWithSpaceAndBackslash) {
  EXPECT_EQ(roundTrip({"clang", "C:\\path with space\\file.cpp"}),
            "clang C:\\path with space\\file.cpp");
}
