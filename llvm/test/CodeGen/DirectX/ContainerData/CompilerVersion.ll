; RUN: llc %S/Inputs/SourceInfo.ll --filetype=obj -o - | obj2yaml | FileCheck %s

#CHECK:        - Name:            VERS
#CHECK-NOT:    - Name:
#CHECK:          CompilerVersion:
#CHECK-NEXT:       Major:           {{[0-9][0-9]+}}
#CHECK-NEXT:       Minor:           {{[0-9]+}}
#CHECK-NEXT:       IsDebugBuild:    {{true|false}}
#CHECK-NEXT:       IsValidated:     false
#CHECK-NEXT:       CommitCount:     {{[0-9]+}}
#CHECK-NEXT:       ContentSizeInBytes: {{[0-9][0-9]+}}
#CHECK-NEXT:       CommitSha:       {{.*}}
#CHECK-NEXT:       CustomVersionString: {{.*}}
