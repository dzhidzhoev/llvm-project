// REQUIRES: directx-registered-target

// RUN: %clang_dxc -T lib_6_7 -Zss -### %s 2>&1 | FileCheck %s --check-prefix=CHECK-ZSS
// CHECK-ZSS: -dx-Zss

// RUN: %clang_dxc -T lib_6_7 -Zs -Zss -### %s 2>&1 | FileCheck %s --check-prefix=CHECK-ZSS

// RUN: not %clang_dxc -T lib_6_7 -Zss -Zsb -### %s 2>&1 | FileCheck %s --check-prefix=CHECK-BOTH
// CHECK-BOTH: cannot specify both /Zss and /Zsb
