; RUN: llc -filetype=obj < %s | llvm-dwarfdump -verify -

; Check that when DISubprogram is attached to two functions, DWARF is produced
; correctly.

; TODO add check for DIEs.

; ModuleID = 'example'

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64--linux"

; Compile unit
!0 = !DISubprogram(name: "foo", scope: !1, file: !1, line: 1, type: !2, scopeLine: 1, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !3, retainedNodes: !{})

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

; Local variable for second function
!20 = !DILocalVariable(name: "b", scope: !0, file: !1, line: 2, type: !5)

; DILocations for instructions inside foo()
!101 = !DILocation(line: 2, column: 5, scope: !0)
!102 = !DILocation(line: 3, column: 1, scope: !0)

; DILocations for instructions inside foo_clone()
!201 = !DILocation(line: 2, column: 5, scope: !0)
!202 = !DILocation(line: 3, column: 1, scope: !0)

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
  ret i32 42, !dbg !102
}

; === Function foo_clone ===
define i32 @foo_clone() !dbg !0 {
entry:
  ; local variable 'b' debug info
  %b.addr = alloca i32, align 4, !dbg !201
  call void @llvm.dbg.declare(metadata i32* %b.addr, metadata !20, metadata !DIExpression()), !dbg !201
  store i32 100, i32* %b.addr, align 4, !dbg !201
  ret i32 100, !dbg !202
}

declare void @llvm.dbg.declare(metadata, metadata, metadata)
