; RUN: llc -filetype=obj < %s -o %t 2>&1 | FileCheck --allow-empty --implicit-check-not='warning:' %s
; RUN: llvm-dwarfdump -verify %t

; Check that when DISubprogram is attached to two functions, DWARF is produced
; correctly.

; TODO add check for DIEs.

; ModuleID = 'example'

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64--linux"

; Compile unit
!0 = distinct !DISubprogram(name: "foo", scope: !1, file: !1, line: 1, type: !2, scopeLine: 1, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !3, retainedNodes: !{})

; File
!1 = !DIFile(filename: "example.c", directory: "/home/user")

; Function type (int foo())
!2 = !DISubroutineType(types: !4)

!4 = !{!5}
!5 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)

; Compile unit scope
!3 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "example", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !{})

; Local variable for first function
!10 = !DILocalVariable(name: "a", scope: !0, file: !1, line: 2, type: !5)

; DICompositeType local to foo
!11 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "bar", scope: !0, file: !1, line: 2, size: 32, elements: !12)
!12 = !{!13}
!13 = !DIDerivedType(tag: DW_TAG_member, name: "m", scope: !11, file: !1, line: 2, baseType: !5, size: 32)

; Local variable of type struct bar, local to foo
!14 = !DILocalVariable(name: "c", scope: !0, file: !1, line: 2, type: !11)

; DILocations
!101 = !DILocation(line: 2, column: 5, scope: !0)
!102 = !DILocation(line: 3, column: 1, scope: !0)
!103 = !DILocation(line: 2, column: 12, scope: !0)

!llvm.dbg.cu = !{!3}
!llvm.module.flags = !{!30}

!30 = !{i32 1, !"Debug Info Version", i32 3}

; === Function foo ===
define i32 @foo() !dbg !0 {
entry:
  ; local variable 'a' debug info
  %a.addr = alloca i32, align 4, !dbg !101
  call void @llvm.dbg.declare(metadata i32* %a.addr, metadata !10, metadata !DIExpression()), !dbg !101
  store i32 42, i32* %a.addr, align 4, !dbg !101

  ; local variable 'c' (struct bar) debug info
  %c.addr = alloca %struct.bar, align 4, !dbg !103
  call void @llvm.dbg.declare(metadata %struct.bar* %c.addr, metadata !14, metadata !DIExpression()), !dbg !103

  ret i32 42, !dbg !102
}

; === Function foo_clone ===
define i32 @foo_clone() !dbg !0 {
entry:
  ; local variable 'a' debug info
  %a.addr = alloca i32, align 4, !dbg !101
  call void @llvm.dbg.declare(metadata i32* %a.addr, metadata !10, metadata !DIExpression()), !dbg !101
  store i32 42, i32* %a.addr, align 4, !dbg !101

  ; local variable 'c' (struct bar) debug info
  %c.addr = alloca %struct.bar, align 4, !dbg !103
  call void @llvm.dbg.declare(metadata %struct.bar* %c.addr, metadata !14, metadata !DIExpression()), !dbg !103

  ret i32 42, !dbg !102
}

%struct.bar = type { i32 }
declare void @llvm.dbg.declare(metadata, metadata, metadata)
