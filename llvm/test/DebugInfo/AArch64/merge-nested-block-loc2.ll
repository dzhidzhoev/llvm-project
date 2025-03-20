; RUN: opt -mtriple=aarch64-unknown-linux-gnu -S %s -passes=sroa -o - | FileCheck %s

; ModuleID = '<stdin>'
source_filename = "main.preproc.c"
target datalayout = "e-m:o-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-n32:64-S128-Fn32"
target triple = "arm64-apple-macosx15.0.0"

; Function Attrs: noinline nounwind ssp uwtable(sync)
define i32 @foo(i32 noundef %a) #0 !dbg !9 {
entry:
  %a.addr = alloca i32, align 4
  %i = alloca i32, align 4
  store i32 %a, ptr %a.addr, align 4
    #dbg_declare(ptr %a.addr, !15, !DIExpression(), !16)
    #dbg_declare(ptr %i, !17, !DIExpression(), !18)
  store i32 0, ptr %i, align 4, !dbg !18
  %0 = load i32, ptr %a.addr, align 4, !dbg !19
  %and = and i32 %0, 1, !dbg !21
  %cmp = icmp eq i32 %and, 1, !dbg !22
  br i1 %cmp, label %if.then, label %if.else, !dbg !22

if.then:                                          ; preds = %entry
  %1 = load i32, ptr %a.addr, align 4, !dbg !23
  %sub = sub nsw i32 %1, 1, !dbg !23
  store i32 %sub, ptr %a.addr, align 4, !dbg !23
  br label %do.body, !dbg !25

do.body:                                          ; preds = %if.then
  %2 = load i32, ptr %a.addr, align 4, !dbg !28
  %3 = load i32, ptr %i, align 4, !dbg !32
  %add = add nsw i32 %3, %2, !dbg !32
  store i32 %add, ptr %i, align 4, !dbg !32
  %4 = load i32, ptr %a.addr, align 4, !dbg !33
  %mul = mul nsw i32 10, %4, !dbg !34
  %5 = load i32, ptr %i, align 4, !dbg !35
  %sub1 = sub nsw i32 %5, %mul, !dbg !35
  store i32 %sub1, ptr %i, align 4, !dbg !35
  %6 = load i32, ptr %a.addr, align 4, !dbg !36
  %7 = load i32, ptr %a.addr, align 4, !dbg !37
  %mul2 = mul nsw i32 %6, %7, !dbg !38
  %8 = load i32, ptr %i, align 4, !dbg !39
  %mul3 = mul nsw i32 %8, %mul2, !dbg !39
  store i32 %mul3, ptr %i, align 4, !dbg !39
  br label %do.end, !dbg !40

do.end:                                           ; preds = %do.body
  br label %if.end, !dbg !42

if.else:                                          ; preds = %entry
  %9 = load i32, ptr %a.addr, align 4, !dbg !44
  %add4 = add nsw i32 %9, 3, !dbg !44
  store i32 %add4, ptr %a.addr, align 4, !dbg !44
  br label %do.body5, !dbg !46

do.body5:                                         ; preds = %if.else
  %10 = load i32, ptr %a.addr, align 4, !dbg !48
  %11 = load i32, ptr %i, align 4, !dbg !52
  %add6 = add nsw i32 %11, %10, !dbg !52
  store i32 %add6, ptr %i, align 4, !dbg !52
  %12 = load i32, ptr %a.addr, align 4, !dbg !53
  %mul7 = mul nsw i32 10, %12, !dbg !54
  %13 = load i32, ptr %i, align 4, !dbg !55
  %sub8 = sub nsw i32 %13, %mul7, !dbg !55
  store i32 %sub8, ptr %i, align 4, !dbg !55
  %14 = load i32, ptr %a.addr, align 4, !dbg !56
  %15 = load i32, ptr %a.addr, align 4, !dbg !57
  %mul9 = mul nsw i32 %14, %15, !dbg !58
  %16 = load i32, ptr %i, align 4, !dbg !59
  %mul10 = mul nsw i32 %16, %mul9, !dbg !59
  store i32 %mul10, ptr %i, align 4, !dbg !59
  br label %do.end11, !dbg !60

do.end11:                                         ; preds = %do.body5
  br label %if.end

if.end:                                           ; preds = %do.end11, %do.end
  %17 = load i32, ptr %i, align 4, !dbg !62
  ret i32 %17, !dbg !63
}

attributes #0 = { noinline nounwind ssp uwtable(sync) "frame-pointer"="non-leaf" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="apple-m1" "target-features"="+aes,+altnzcv,+ccdp,+ccidx,+ccpp,+complxnum,+crc,+dit,+dotprod,+flagm,+fp-armv8,+fp16fml,+fptoint,+fullfp16,+jsconv,+lse,+neon,+pauth,+perfmon,+predres,+ras,+rcpc,+rdm,+sb,+sha2,+sha3,+specrestrict,+ssbs,+v8.1a,+v8.2a,+v8.3a,+v8.4a,+v8a,+zcm,+zcz" }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3, !4, !5, !6, !7}
!llvm.ident = !{!8}

!0 = distinct !DICompileUnit(language: DW_LANG_C11, file: !1, producer: "clang version 21.0.0git", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, splitDebugInlining: false, nameTableKind: Apple, sysroot: "/")
!1 = !DIFile(filename: "main.preproc.c", directory: "")
!2 = !{i32 7, !"Dwarf Version", i32 5}
!3 = !{i32 2, !"Debug Info Version", i32 3}
!4 = !{i32 1, !"wchar_size", i32 4}
!5 = !{i32 8, !"PIC Level", i32 2}
!6 = !{i32 7, !"uwtable", i32 1}
!7 = !{i32 7, !"frame-pointer", i32 1}
!8 = !{!"clang version 21.0.0git (git@github.com:dzhidzhoev/llvm-project.git bd80a265c8f05123ea19bfcd3e5b1aa52e152ee2)"}
!9 = distinct !DISubprogram(name: "foo", scope: !10, file: !10, line: 1, type: !11, scopeLine: 1, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !14)
!10 = !DIFile(filename: "main.c", directory: "")
!11 = !DISubroutineType(types: !12)
!12 = !{!13, !13}
!13 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!14 = !{}
!15 = !DILocalVariable(name: "a", arg: 1, scope: !9, file: !10, line: 1, type: !13)
!16 = !DILocation(line: 1, column: 13, scope: !9)
!17 = !DILocalVariable(name: "i", scope: !9, file: !10, line: 2, type: !13)
!18 = !DILocation(line: 2, column: 7, scope: !9)
!19 = !DILocation(line: 3, column: 8, scope: !20)
!20 = distinct !DILexicalBlock(scope: !9, file: !10, line: 3, column: 7)
!21 = !DILocation(line: 3, column: 10, scope: !20)
!22 = !DILocation(line: 3, column: 15, scope: !20)
!23 = !DILocation(line: 4, column: 7, scope: !24)
!24 = distinct !DILexicalBlock(scope: !20, file: !10, line: 3, column: 21)
!25 = !DILocation(line: 300, column: 1, scope: !26)
!26 = !DILexicalBlockFile(scope: !24, file: !27, discriminator: 0)
!27 = !DIFile(filename: "y.c", directory: "")
!28 = !DILocation(line: 100, column: 6, scope: !29)
!29 = !DILexicalBlockFile(scope: !31, file: !30, discriminator: 0)
!30 = !DIFile(filename: "z1.c", directory: "")
!31 = distinct !DILexicalBlock(scope: !26, file: !27, line: 300, column: 4)
!32 = !DILocation(line: 100, column: 3, scope: !29)
!33 = !DILocation(line: 101, column: 9, scope: !29)
!34 = !DILocation(line: 101, column: 8, scope: !29)
!35 = !DILocation(line: 101, column: 3, scope: !29)
!36 = !DILocation(line: 102, column: 6, scope: !29)
!37 = !DILocation(line: 102, column: 8, scope: !29)
!38 = !DILocation(line: 102, column: 7, scope: !29)
!39 = !DILocation(line: 102, column: 3, scope: !29)
!40 = !DILocation(line: 306, column: 1, scope: !41)
!41 = !DILexicalBlockFile(scope: !31, file: !27, discriminator: 0)
!42 = !DILocation(line: 7, column: 2, scope: !43)
!43 = !DILexicalBlockFile(scope: !24, file: !10, discriminator: 0)
!44 = !DILocation(line: 8, column: 7, scope: !45)
!45 = distinct !DILexicalBlock(scope: !20, file: !10, line: 7, column: 9)
!46 = !DILocation(line: 300, column: 1, scope: !47)
!47 = !DILexicalBlockFile(scope: !45, file: !27, discriminator: 0)
!48 = !DILocation(line: 200, column: 6, scope: !49)
!49 = !DILexicalBlockFile(scope: !51, file: !50, discriminator: 0)
!50 = !DIFile(filename: "z2.c", directory: "")
!51 = distinct !DILexicalBlock(scope: !47, file: !27, line: 300, column: 4)
!52 = !DILocation(line: 200, column: 3, scope: !49)
!53 = !DILocation(line: 201, column: 9, scope: !49)
!54 = !DILocation(line: 201, column: 8, scope: !49)
!55 = !DILocation(line: 201, column: 3, scope: !49)
!56 = !DILocation(line: 202, column: 6, scope: !49)
!57 = !DILocation(line: 202, column: 8, scope: !49)
!58 = !DILocation(line: 202, column: 7, scope: !49)
!59 = !DILocation(line: 202, column: 3, scope: !49)
!60 = !DILocation(line: 306, column: 1, scope: !61)
!61 = !DILexicalBlockFile(scope: !51, file: !27, discriminator: 0)
!62 = !DILocation(line: 12, column: 10, scope: !9)
!63 = !DILocation(line: 12, column: 3, scope: !9)
