; The .bc file was generated from this source using llvm-as from r347766.
; A 7.0 release version should work to recreate it if necessary.
; RUN: llvm-dis < %s.bc | FileCheck %s

; CHECK: define void @_Z3foov() !dbg !9
define void @_Z3foov() !dbg !9 {
  ret void
}

; CHECK: !named = !{!0, !1, !2, !3, !4, !5, !6, !7, !8, !9, !10, !11, !12, !13, !14, !15, !16, !17, !18}
!named = !{!0, !1, !2, !3, !4, !5, !6, !7, !8, !9, !10, !11, !12, !13, !14, !15, !16, !17, !18}

!0 = !{null}
!1 = distinct !DICompositeType(tag: DW_TAG_structure_type)
!2 = !DIFile(filename: "path/to/file", directory: "/path/to/dir")
!3 = !DISubroutineType(types: !0)
!4 = distinct !DICompositeType(tag: DW_TAG_structure_type)
!5 = distinct !{}
!6 = distinct !{}

; CHECK: !7 = !DISubprogram(scope: null, spFlags: DISPFlagDefinition, unit: !8)
!7 = !DISubprogram(unit: !8)

!8 = distinct !DICompileUnit(language: DW_LANG_C99, producer: "clang",
                             file: !2, isOptimized: true, flags: "-O2")

; CHECK: !9 = !DISubprogram(scope: null, spFlags: 0)
!9 = !DISubprogram(isDefinition: false)

; CHECK: !10 = !DISubprogram({{.*}}, spFlags: DISPFlagPureVirtual | DISPFlagLocalToUnit | DISPFlagDefinition | DISPFlagOptimized,
!10 = !DISubprogram(name: "foo", linkageName: "_Zfoov", scope: !1,
                            file: !2, line: 7, type: !3, containingType: !4,
							isLocal: true, isDefinition: true, isOptimized: true,
							virtuality: DW_VIRTUALITY_pure_virtual,
                            unit: !8)

; CHECK: !11 = !DISubprogram({{.*}}, spFlags: DISPFlagVirtual | DISPFlagLocalToUnit | DISPFlagDefinition | DISPFlagOptimized,
!11 = !DISubprogram(name: "foo", linkageName: "_Zfoov", scope: !1,
                            file: !2, line: 7, type: !3, containingType: !4,
							isLocal: true, isDefinition: true, isOptimized: true,
							virtuality: DW_VIRTUALITY_virtual,
                            unit: !8)

; CHECK: !12 = !DISubprogram({{.*}}, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition | DISPFlagOptimized,
!12 = !DISubprogram(name: "foo", linkageName: "_Zfoov", scope: !1,
                            file: !2, line: 7, type: !3, containingType: !4,
							isLocal: true, isDefinition: true, isOptimized: true,
							virtuality: DW_VIRTUALITY_none,
                            unit: !8)

; CHECK: !13 = !DISubprogram({{.*}}, spFlags: DISPFlagDefinition | DISPFlagOptimized,
!13 = !DISubprogram(name: "foo", linkageName: "_Zfoov", scope: !1,
                            file: !2, line: 7, type: !3, containingType: !4,
							isLocal: false, isDefinition: true, isOptimized: true,
                            unit: !8)

; CHECK: !14 = !DISubprogram({{.*}}, spFlags: DISPFlagLocalToUnit | DISPFlagOptimized)
!14 = !DISubprogram(name: "foo", linkageName: "_Zfoov", scope: !1,
                            file: !2, line: 7, type: !3, containingType: !4,
							isLocal: true, isDefinition: false, isOptimized: true)

; CHECK: !15 = !DISubprogram({{.*}}, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition,
!15 = !DISubprogram(name: "foo", linkageName: "_Zfoov", scope: !1,
                            file: !2, line: 7, type: !3, containingType: !4,
							isLocal: true, isDefinition: true, isOptimized: false,
                            unit: !8)

; CHECK: !16 = !DISubprogram({{.*}}, spFlags: DISPFlagLocalToUnit)
!16 = !DISubprogram(name: "foo", linkageName: "_Zfoov", scope: !1,
                            file: !2, line: 7, type: !3, containingType: !4,
							isLocal: true, isDefinition: false, isOptimized: false)

; CHECK: !17 = !DISubprogram({{.*}}, spFlags: DISPFlagDefinition,
!17 = !DISubprogram(name: "foo", linkageName: "_Zfoov", scope: !1,
                            file: !2, line: 7, type: !3, containingType: !4,
							isLocal: false, isDefinition: true, isOptimized: false,
                            unit: !8)

; CHECK: !18 = !DISubprogram({{.*}}, spFlags: DISPFlagOptimized)
!18 = !DISubprogram(name: "foo", linkageName: "_Zfoov", scope: !1,
                            file: !2, line: 7, type: !3, containingType: !4,
							isLocal: false, isDefinition: false, isOptimized: true)

!19 = !{i32 1, !"Debug Info Version", i32 3}
!llvm.module.flags = !{!19}
!llvm.dbg.cu = !{!8}
