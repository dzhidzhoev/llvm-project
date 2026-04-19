// REQUIRES: asserts
// RUN: not %clang_cc1 -triple dxil-pc-shadermodel6.3-library -x hlsl -emit-llvm \
// RUN:   -debug-info-kind=constructor \
// RUN:   -fdx-record-command-line "clang_dxc \\" \
// RUN:   -o - %s 2>%t.err
// RUN: FileCheck %s < %t.err

// CHECK: Only escaped backslashes and spaces are supported.

float foo(float a, float b) {
  return a + b;
}
