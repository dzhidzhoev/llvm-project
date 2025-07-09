; RUN: not llvm-as < %s -disable-output 2>&1 | FileCheck %s

; CHECK: <stdin>:[[@LINE+1]]:15: error: definition DISubprogram must be unique
!0 = distinct !DISubprogram(isDefinition: true)
