// RUN: %clang_dxc -Tlib_6_7 -### /Zi /Qsource_in_debug_module %s 2>&1 | FileCheck %s
// RUN: %clang_dxc -Tlib_6_7 -### /Zi -Qsource_in_debug_module %s 2>&1 | FileCheck %s
// RUN: %clang_dxc -Tlib_6_7 -### /Zi %s 2>&1 | FileCheck %s --check-prefix=NOFLAG

// CHECK: "-mllvm" "--dx-source-in-debug-module"
// NOFLAG-NOT: --dx-source-in-debug-module
