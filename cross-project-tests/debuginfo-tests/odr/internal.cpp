// TODO: remove extra llvm-dis'es

// TODO split this test into a test where DICompositeTypes are not distinct and are distinct
// regardless of the platform (currently, the are all distinct in CodeView format).

// RUN: split-file %s %t
// RUN: %clang -flto=full -c -g -fstandalone-debug -O0 -o %t/odr1.bc %t/odr1.cpp
// RUN: llvm-dis %t/odr1.bc
// RUN: %clang -flto=full -c -g -fstandalone-debug -O0 -o %t/odr2.bc %t/odr2.cpp
// RUN: llvm-dis %t/odr2.bc
// RUN: llvm-lto --save-linked-module -o %t/odr %t/odr1.bc %t/odr2.bc
// RUN: llvm-dis %t/odr.linked.bc
// RUN: llvm-dis -o - %t/odr.linked.bc | FileCheck %s

// CHECK-DAG: [[SP1:![0-9]+]] = !DISubprogram(name: "foo"
// CHECK-DAG: [[CT1:![0-9]+]] = {{(distinct )?}}!DICompositeType({{.*}}, name: "S_int", scope: [[SP1]]
// CHECK-DAG: [[CT2:![0-9]+]] = {{(distinct )?}}!DICompositeType({{.*}}, name: "S_int", scope: [[SP2:![0-9]+]]
// CHECK-DAG: [[SP2]] = !DISubprogram(name: "foo"

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

