; Compare source info emission with and without --dx-source-in-debug-module flag.

; RUN: llc %s --filetype=obj -o %t.dxbc
; RUN: llvm-objcopy --dump-section=DXIL=%t.dxil.bc %t.dxbc
; RUN: llvm-dis %t.dxil.bc -o - | FileCheck %s --check-prefix=ILDB-DIS

; RUN: llc %s --filetype=obj -o %t.dxbc --dx-source-in-debug-module
; RUN: llvm-objcopy --dump-section=DXIL=%t.dxil.bc %t.dxbc
; RUN: llvm-dis %t.dxil.bc -o - | FileCheck %s --check-prefix=ILDB-SOURCE-DIS

; Without the flag, dx.source should be replaced with dummy metadata.
; ILDB-DIS: !dx.source.contents = !{![[CONTENTS:[0-9]+]]}
; ILDB-DIS: !dx.source.defines = !{![[EMPTY_ARR:[0-9]+]]}
; ILDB-DIS: !dx.source.mainFileName = !{![[MAIN:[0-9]+]]}
; ILDB-DIS: !dx.source.args = !{![[EMPTY_ARR]]}
; ILDB-DIS: ![[CONTENTS]] = !{!"", !""}
; ILDB-DIS: ![[EMPTY_ARR]] = !{}
; ILDB-DIS: ![[MAIN]] = !{!""}

; With the flag, dx.source should be be preserved.
; ILDB-SOURCE-DIS: !dx.source.args = !{![[ARGS:[0-9]+]]}
; ILDB-SOURCE-DIS: !dx.source.contents = !{![[FILE1:[0-9]+]], ![[FILE2:[0-9]+]], ![[FILE3:[0-9]+]]}
; ILDB-SOURCE-DIS: !dx.source.mainFileName = !{![[MAIN:[0-9]+]]}
; ILDB-SOURCE-DIS: !dx.source.defines = !{![[DEFINES:[0-9]+]]}
; ILDB-SOURCE-DIS: ![[FILE1]] = !{!"C:\\dx-source-metadata.hlsl",
; ILDB-SOURCE-DIS: ![[FILE2]] = !{!"C:\\a.hlsl"
; ILDB-SOURCE-DIS: ![[FILE3]] = !{!"C:\\b.hlsl"
; ILDB-SOURCE-DIS: ![[MAIN]] = !{!"C:\\dx-source-metadata.hlsl"}
; ILDB-SOURCE-DIS: ![[DEFINES]] = !{!"USER_DEF0=42", !"USER_DEF1=43"}

target triple = "dxilv1.3-pc-shadermodel6.3-library"

define float @_Z3fooff(float %a, float %b) {
entry:
  %add = fadd float %a, %b
  ret float %add
}

!llvm.dbg.cu = !{!4}
!llvm.module.flags = !{!6, !7}

!dx.source.args = !{!0}
!dx.source.contents = !{!1, !2, !3}
!dx.source.mainFileName = !{!8}
!dx.source.defines = !{!9}

!0 = !{!"-g", !"-Tlib_6_3", !"-DUSER_DEF0=42", !"-DUSER_DEF1=43", !"C:\\\\dx-source-metadata.hlsl"}
!1 = !{!"C:\\dx-source-metadata.hlsl", !"#include \22a.hlsl\22\0A#include \22b.hlsl\22\0A\0Afloat foo(float a, float b) {\0A  return a + b;\0A}\0A"}
!2 = !{!"C:\\a.hlsl", !"#include \22b.hlsl\22\0A"}
!3 = !{!"C:\\b.hlsl", !"#include <c.hlsl>\0A"}
!4 = distinct !DICompileUnit(language: DW_LANG_C99, file: !5, emissionKind: FullDebug)
!5 = !DIFile(filename: "dx-source-metadata.hlsl", directory: "C:\\")
!6 = !{i32 2, !"Dwarf Version", i32 4}
!7 = !{i32 2, !"Debug Info Version", i32 3}
!8 = !{!"C:\\dx-source-metadata.hlsl"}
!9 = !{!"USER_DEF0=42", !"USER_DEF1=43"}
