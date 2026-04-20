// RUN: %clang_cc1 -triple dxil-pc-shadermodel6.3-library -x hlsl -emit-llvm \
// RUN:   -debug-info-kind=constructor -DUSER_DEF=42 \
// RUN:   -fdx-record-command-line "clang_dxc -Tlib_6_3 -DUSER_DEF=42 dx-source-metadata.hlsl" \
// RUN:   -o - %s | FileCheck %s

// CHECK: !dx.source.contents = !{![[CONTENTS:[0-9]+]]}
// CHECK: ![[CONTENTS]] = !{!"{{.*[\\/]dx-source-metadata.hlsl}}", !"{{.*}}"}

// CHECK: !dx.source.defines = !{![[DEFINES:[0-9]+]]}
// CHECK: ![[DEFINES]] = !{!"USER_DEF=42"}

// CHECK: !dx.source.mainFileName = !{![[MAIN:[0-9]+]]}
// CHECK: ![[MAIN]] = !{!"{{.*[\\/]dx-source-metadata.hlsl}}"}

// CHECK: !dx.source.args = !{![[ARGS:[0-9]+]]}
// CHECK: ![[ARGS]] = !{!"clang_dxc", !"-Tlib_6_3", !"-DUSER_DEF=42", !"dx-source-metadata.hlsl"}

float foo(float a, float b) {
  return a + b;
}
