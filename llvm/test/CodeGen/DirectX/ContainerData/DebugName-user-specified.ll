; RUN: llc %S/Inputs/SourceInfo.ll --filetype=obj -o - --dx-pdb-file=ofile.pdb | obj2yaml | FileCheck %s

; CHECK:       - Name:            ILDN
; CHECK-NEXT:    Size:            16
; CHECK-NEXT:    DebugName:
; CHECK-NEXT:      Flags:           0
; CHECK-NEXT:      NameLength:      9
; CHECK-NEXT:      DebugName:       ofile.pdb
