; RUN: llvm-link %s %s -S -o %t
; RUN: opt --passes=verify %t -o /dev/null
; RUN: FileCheck --input-file=%t %s --implicit-check-not=DICompositeType

; During module loading, if a local type appears in retainedNodes
; field of multiple DISubprograms due to ODR-uniquing,
; LLParser should clean up retainedNodes, so that only one DISubprogram
; will have this type in its retainedNodes.

; CHECK: [[BAR1:![0-9]+]] = distinct !DISubprogram(name: "bar", {{.*}}, retainedNodes: [[RN_BAR1:![0-9]+]])
; CHECK: [[EMPTY:![0-9]+]] = !{}
; CHECK: [[RN_BAR1]] = !{[[T1:![0-9]+]]}
; CHECK: [[T1]] = distinct !DICompositeType(tag: DW_TAG_class_type, scope: [[BAR1]], {{.*}}, identifier: "local_type")
; CHECK: {{![0-9]+}} = distinct !DISubprogram(name: "bar", {{.*}}, retainedNodes: [[EMPTY:![0-9]+]])

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@llvm.used = appending global [1 x ptr] [ptr @bar]

define internal void @bar(ptr %this) !dbg !10 {
  ret void
}

!llvm.module.flags = !{!0, !1, !2, !3, !4, !5, !6, !7}
!llvm.dbg.cu = !{!8}

!0 = !{i32 2, !"SDK Version", [2 x i32] [i32 11, i32 4]}
!1 = !{i32 7, !"Dwarf Version", i32 2}
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = !{i32 1, !"wchar_size", i32 4}
!4 = !{i32 7, !"openmp", i32 50}
!5 = !{i32 7, !"openmp-device", i32 50}
!6 = !{i32 7, !"PIC Level", i32 2}
!7 = !{i32 7, !"frame-pointer", i32 2}
!8 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus_14, file: !9, producer: "clang version 14.0.0", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug, splitDebugInlining: false, nameTableKind: None)
!9 = !DIFile(filename: "tmp.cpp", directory: "/tmp/")
!10 = distinct !DISubprogram(name: "bar", scope: !9, file: !9, line: 68, type: !11, scopeLine: 68, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition | DISPFlagOptimized, unit: !8, retainedNodes: !14)
!11 = !DISubroutineType(types: !13)
!12 = !DICompositeType(tag: DW_TAG_class_type, scope: !10, file: !9, line: 210, size: 8, flags: DIFlagTypePassByValue, elements: !13, identifier: "local_type")
!13 = !{}
!14 = !{!12}
