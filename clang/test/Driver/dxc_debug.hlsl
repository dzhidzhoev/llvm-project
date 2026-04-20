// RUN: %clang_dxc -Tlib_6_7 -### -g %s 2>&1 | FileCheck %s
// RUN: %clang_dxc -Tlib_6_7 -### /Zi %s 2>&1 | FileCheck %s
// RUN: %clang_dxc -Tlib_6_7 -### /Zi /Qembed_debug %s 2>&1 | FileCheck %s
// RUN: %clang_dxc -Tlib_6_7 -### -Zi %s 2>&1 | FileCheck %s
// RUN: %clang_dxc -Tlib_6_7 -### -Zi -Qembed_debug %s 2>&1 | FileCheck %s
// RUN: %clang_dxc -Tlib_6_7 -### -Zi -gcodeview %s 2>&1 | FileCheck %s -check-prefixes=CHECK,CHECK-CV
// RUN: %clang_dxc -Tlib_6_7 -### -Zi -gdwarf %s 2>&1 | FileCheck %s -check-prefixes=CHECK,CHECK-DWARF
// RUN: %clang_dxc -Tlib_6_7 -### -gcodeview -Zi %s 2>&1 | FileCheck %s -check-prefixes=CHECK,CHECK-CV
// RUN: %clang_dxc -Tlib_6_7 -### -gdwarf -Zi %s 2>&1 | FileCheck %s -check-prefixes=CHECK,CHECK-DWARF
// RUN: %clang_dxc -Tlib_6_7 -### %s 2>&1 | FileCheck %s --check-prefix=CHECK-NO-DEBUG

// CHECK: "-cc1"
// CHECK-SAME: "-fdx-record-command-line" "{{.*}}"
// CHECK-CV-SAME: -gcodeview
// CHECK-SAME: "-debug-info-kind=constructor"
// Make sure dwarf-version is 4.
// CHECK-DWARF-SAME: -dwarf-version=4

// CHECK-NO-DEBUG: "-cc1"
// CHECK-NO-DEBUG-NOT: "-fdx-record-command-line"
