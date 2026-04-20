// RUN: %clang_cc1 -triple dxil-pc-shadermodel6.3-library -x hlsl -emit-llvm \
// RUN:   -debug-info-kind=constructor -I %S/Inputs \
// RUN:   -o - %s | FileCheck %s

#include "dx-source-metadata-helper1.hlsl"
#include "dx-source-metadata-helper2.hlsl"

// The main file appears first in dx.source.contents; included files are
// appended in sorted order afterwards.

// CHECK: !dx.source.contents = !{![[MAIN:[0-9]+]], ![[H1:[0-9]+]], ![[H2:[0-9]+]]}
// CHECK: ![[MAIN]] = !{!"{{.*[\\/]dx-source-metadata-includes.hlsl}}", !"{{.*}}"}
// CHECK: ![[H1]] = !{!"{{.*[\\/]dx-source-metadata-helper1.hlsl}}", !"{{.*}}"}
// CHECK: ![[H2]] = !{!"{{.*[\\/]dx-source-metadata-helper2.hlsl}}", !"{{.*}}"}

float foo(float a, float b) {
  return helper1_add(a, b) + helper2_mul(a, b);
}
