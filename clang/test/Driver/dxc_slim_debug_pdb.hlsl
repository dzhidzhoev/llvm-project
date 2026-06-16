// Check that /Zs emits a companion PDB with slim debug parts, and omits ILDB.

// REQUIRES: directx-registered-target
// RUN: %clang_dxc -Tlib_6_7 /Fo /dev/null /Zs /Fd %t.zs.pdb %s 2>&1
// RUN: llvm-pdbutil pdb2yaml --dxcontainer %t.zs.pdb | FileCheck %s --implicit-check-not=ILDB

// CHECK:     Parts:
// CHECK-DAG:   - Name: HASH
// CHECK-DAG:   - Name: ILDN
// CHECK-DAG:   - Name: VERS
// CHECK-DAG:   - Name: SRCI

[numthreads(1, 1, 1)] void main() {}
