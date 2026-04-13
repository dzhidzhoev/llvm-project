// RUN: mlir-translate --import-dxsa-bin %S/inputs/ret.bin | FileCheck %s

// CHECK:      module {
// CHECK-NEXT:   dxsa.instruction "ret"
// CHECK-NEXT: }

module {
  dxsa.instruction "ret"
}
