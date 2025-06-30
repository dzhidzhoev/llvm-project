// TODO: remove extra llvm-dis'es

// RUN: split-file %s %t
// RUN: %clang -flto=full -c -g -fstandalone-debug -O0 -o %t/odr1.bc %t/odr1.cpp
// RUN: llvm-dis %t/odr1.bc
// RUN: %clang -flto=full -c -g -fstandalone-debug -O0 -o %t/odr2.bc %t/odr2.cpp
// RUN: llvm-dis %t/odr2.bc
// RUN: llvm-lto --save-linked-module -o %t/odr %t/odr1.bc %t/odr2.bc
// RUN: llvm-dis %t/odr.linked.bc
// RUN: llvm-dis -o - %t/odr.linked.bc | FileCheck %s --check-prefix=TYPE
// RUN: llvm-dis -o - %t/odr.linked.bc | FileCheck %s --check-prefix=SP

// TYPE: !DICompositeType{{.*}}S_int
// TYPE-NOT: !DICompositeType

// SP: !DISubprogram{{.*}}foo
// SP-NOT: !DISubprogram{{.*}}foo

//--- odr.h
#pragma once

static int foo(int a, int b) {
  struct S_int {
    int x;
    int y;
  };
  S_int inst;
  inst.x = a;
  inst.y = b;
  return inst.x - inst.y;
}

//--- odr1.cpp
#include "odr.h"

int bar1(int a, int b) {
  return foo(a, b);
}

//--- odr2.cpp
#include "odr.h"

int bar2(int a, int b) {
  return foo(a, b);
}

