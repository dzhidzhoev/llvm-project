// RUN: %clang_analyze_cc1 -analyzer-checker=core,debug.ExprInspection -analyzer-config c++-inlining=constructors -verify %s

#include "Inputs/system-header-simulator-cxx.h"

void clang_analyzer_eval(bool);

struct s {
  int x;
  int y;
};

void a1(void) {
  s arr[3];
  int x = arr[0].x;
  // expected-warning@-1{{Assigned value is uninitialized}}
}

void a2(void) {
  s arr[3];
  int x = arr[1].x;
  // expected-warning@-1{{Assigned value is uninitialized}}
}

void a3(void) {
  s arr[3];
  int x = arr[2].x;
  // expected-warning@-1{{Assigned value is uninitialized}}
}

struct s2 {
  int x;
  int y = 2;
};

void b1(void) {
  s2 arr[3];

  clang_analyzer_eval(arr[0].y == 2); // expected-warning{{TRUE}}
  int x = arr[0].x;
  // expected-warning@-1{{Assigned value is uninitialized}}
}

void b2(void) {
  s2 arr[3];

  clang_analyzer_eval(arr[1].y == 2); // expected-warning{{TRUE}}
  int x = arr[1].x;
  // expected-warning@-1{{Assigned value is uninitialized}}
}

void b3(void) {
  s2 arr[3];

  clang_analyzer_eval(arr[2].y == 2); // expected-warning{{TRUE}}
  int x = arr[2].x;
  // expected-warning@-1{{Assigned value is uninitialized}}
}

void c1(void) {
  {
    s2 arr[2];
    arr[1].x = 3;

    clang_analyzer_eval(arr[1].y == 2); // expected-warning{{TRUE}}
    clang_analyzer_eval(arr[1].x == 3); // expected-warning{{TRUE}}
  }

  {
    s2 arr[2];

    clang_analyzer_eval(arr[1].y == 2); // expected-warning{{TRUE}}
    int x = arr[1].x;
    // expected-warning@-1{{Assigned value is uninitialized}}
  }
}

struct s3 {
  int x = 1;
  int y = 2;
};

struct s4 {
  s3 arr[2];
  s sarr[2];
};

void e1(void) {
  s4 arr[2];

  clang_analyzer_eval(arr[0].arr[0].x == 1); // expected-warning{{TRUE}}
  clang_analyzer_eval(arr[0].arr[0].y == 2); // expected-warning{{TRUE}}

  clang_analyzer_eval(arr[0].arr[1].x == 1); // expected-warning{{TRUE}}
  clang_analyzer_eval(arr[0].arr[1].y == 2); // expected-warning{{TRUE}}

  clang_analyzer_eval(arr[1].arr[0].x == 1); // expected-warning{{TRUE}}
  clang_analyzer_eval(arr[1].arr[0].y == 2); // expected-warning{{TRUE}}

  clang_analyzer_eval(arr[1].arr[1].x == 1); // expected-warning{{TRUE}}
  clang_analyzer_eval(arr[1].arr[1].y == 2); // expected-warning{{TRUE}}

  int x = arr[1].sarr[1].x;
  // expected-warning@-1{{Assigned value is uninitialized}}
}

void f1(void) {
  s2 arr[2][2];

  clang_analyzer_eval(arr[1][1].y == 2); // expected-warning{{TRUE}}
  int x = arr[1][1].x;
  // expected-warning@-1{{Assigned value is uninitialized}}
}

struct s5 {
  static int c;
  int x;

  s5() : x(c++) {}
};

void g1(void) {
  s5::c = 0;
  s5 *arr = new s5[4];
  new (arr + 1) s5[3];

  clang_analyzer_eval(arr[0].x == 0); // expected-warning{{TRUE}}
  clang_analyzer_eval(arr[1].x == 4); // expected-warning{{TRUE}}
  clang_analyzer_eval(arr[2].x == 5); // expected-warning{{TRUE}}
  clang_analyzer_eval(arr[3].x == 6); // expected-warning{{TRUE}}
}

void g2(void) {
  s5::c = 0;
  s5 arr[4];

  clang_analyzer_eval(arr[0].x == 0); // expected-warning{{TRUE}}
  clang_analyzer_eval(arr[1].x == 1); // expected-warning{{TRUE}}
  clang_analyzer_eval(arr[2].x == 2); // expected-warning{{TRUE}}
  clang_analyzer_eval(arr[3].x == 3); // expected-warning{{TRUE}}
}

void g3(void) {
  s5::c = 0;
  s5 arr[2][2];

  clang_analyzer_eval(arr[0][0].x == 0); // expected-warning{{TRUE}}
  clang_analyzer_eval(arr[0][1].x == 1); // expected-warning{{TRUE}}
  clang_analyzer_eval(arr[1][0].x == 2); // expected-warning{{TRUE}}
  clang_analyzer_eval(arr[1][1].x == 3); // expected-warning{{TRUE}}
}

void h1(void) {
  s5::c = 0;
  s5 a[2][2], b[2][2];

  clang_analyzer_eval(a[0][0].x == 0); // expected-warning{{TRUE}}
  clang_analyzer_eval(a[0][1].x == 1); // expected-warning{{TRUE}}
  clang_analyzer_eval(a[1][0].x == 2); // expected-warning{{TRUE}}
  clang_analyzer_eval(a[1][1].x == 3); // expected-warning{{TRUE}}

  clang_analyzer_eval(b[0][0].x == 4); // expected-warning{{TRUE}}
  clang_analyzer_eval(b[0][1].x == 5); // expected-warning{{TRUE}}
  clang_analyzer_eval(b[1][0].x == 6); // expected-warning{{TRUE}}
  clang_analyzer_eval(b[1][1].x == 7); // expected-warning{{TRUE}}
}

void h2(void) {
  s a[2][2], b[2][2];

  int x = a[1][1].x;
  // expected-warning@-1{{Assigned value is uninitialized}}
}

void h3(void) {
  s a[2][2], b[2][2];

  int x = b[1][1].y;
  // expected-warning@-1{{Assigned value is uninitialized}}
}

struct Base {
  int x;
  int y;

  Base(int x, int y) : x(x), y(y) {}
};

struct Derived : public Base {
  int i;
  int j;

  Derived(int x, int y, int i, int j) : Base(x, y), i(i), j(j) {}
};

void delegate(void) {
  Derived arr[2] = {{1, 2, 3, 4}, {5, 6, 7, 8}};

  clang_analyzer_eval(arr[0].x == 1); // expected-warning{{TRUE}}
  clang_analyzer_eval(arr[0].y == 2); // expected-warning{{TRUE}}
  clang_analyzer_eval(arr[0].i == 3); // expected-warning{{TRUE}}
  clang_analyzer_eval(arr[0].j == 4); // expected-warning{{TRUE}}

  clang_analyzer_eval(arr[1].x == 5); // expected-warning{{TRUE}}
  clang_analyzer_eval(arr[1].y == 6); // expected-warning{{TRUE}}
  clang_analyzer_eval(arr[1].i == 7); // expected-warning{{TRUE}}
  clang_analyzer_eval(arr[1].j == 8); // expected-warning{{TRUE}}
}

void delegate_heap(void) {
  Derived *arr = new Derived[2]{{1, 2, 3, 4}, {5, 6, 7, 8}};

  clang_analyzer_eval(arr[0].x == 1); // expected-warning{{TRUE}}
  clang_analyzer_eval(arr[0].y == 2); // expected-warning{{TRUE}}
  clang_analyzer_eval(arr[0].i == 3); // expected-warning{{TRUE}}
  clang_analyzer_eval(arr[0].j == 4); // expected-warning{{TRUE}}

  clang_analyzer_eval(arr[1].x == 5); // expected-warning{{TRUE}}
  clang_analyzer_eval(arr[1].y == 6); // expected-warning{{TRUE}}
  clang_analyzer_eval(arr[1].i == 7); // expected-warning{{TRUE}}
  clang_analyzer_eval(arr[1].j == 8); // expected-warning{{TRUE}}
}

struct Member {
  int x;
  int y;
};

struct Parent {
  Member arr[2];

  Parent() : arr{{1, 2}, {3, 4}} {}
};

void member() {
  Parent arr[2];

  clang_analyzer_eval(arr[0].arr[0].x == 1); // expected-warning{{TRUE}}
  clang_analyzer_eval(arr[0].arr[0].y == 2); // expected-warning{{TRUE}}
  clang_analyzer_eval(arr[0].arr[1].x == 3); // expected-warning{{TRUE}}
  clang_analyzer_eval(arr[0].arr[1].y == 4); // expected-warning{{TRUE}}

  clang_analyzer_eval(arr[1].arr[0].x == 1); // expected-warning{{TRUE}}
  clang_analyzer_eval(arr[1].arr[0].y == 2); // expected-warning{{TRUE}}
  clang_analyzer_eval(arr[1].arr[1].x == 3); // expected-warning{{TRUE}}
  clang_analyzer_eval(arr[1].arr[1].y == 4); // expected-warning{{TRUE}}
}

struct HasArr {
  int arrDefault[2] = {1, 2};
  int arr[2];
  HasArr(int x, int y) : arr{x, y} {}
};

struct ArrCombination : public HasArr {
    HasArr membDefault = {5, 6};
    HasArr memb;
    ArrCombination(int x) : HasArr(3, 4), memb{7, x} {}
};

void derived_and_member() {
  ArrCombination a{8};
  // FIXME: Default initializers for array members are not modeled.
  clang_analyzer_eval(a.arrDefault[0] == 1); // expected-warning{{UNKNOWN}}
  clang_analyzer_eval(a.arrDefault[1] == 2); // expected-warning{{UNKNOWN}}
  clang_analyzer_eval(a.arr[0] == 3); // expected-warning{{TRUE}}
  clang_analyzer_eval(a.arr[1] == 4); // expected-warning{{TRUE}}
  clang_analyzer_eval(a.membDefault.arrDefault[0] == 1); // expected-warning{{UNKNOWN}}
  clang_analyzer_eval(a.membDefault.arrDefault[1] == 2); // expected-warning{{UNKNOWN}}
  clang_analyzer_eval(a.membDefault.arr[0] == 5); // expected-warning{{UNKNOWN}}
  clang_analyzer_eval(a.membDefault.arr[1] == 6); // expected-warning{{UNKNOWN}}
  clang_analyzer_eval(a.memb.arrDefault[0] == 1); // expected-warning{{UNKNOWN}}
  clang_analyzer_eval(a.memb.arrDefault[1] == 2); // expected-warning{{UNKNOWN}}
  clang_analyzer_eval(a.memb.arr[0] == 7); // expected-warning{{TRUE}}
  clang_analyzer_eval(a.memb.arr[1] == 8); // expected-warning{{TRUE}}

}

struct IncompleteArrInit {
  int arr[2];
  int arrDefault[3] = {1, 2, 3};
  IncompleteArrInit() : arr{1}, arrDefault{2, 3} {}
};

void incomplete_array_init() {
  IncompleteArrInit a;
  clang_analyzer_eval(a.arr[0] == 1); // expected-warning{{TRUE}}
  clang_analyzer_eval(a.arr[1] == 0); // expected-warning{{TRUE}}
  clang_analyzer_eval(a.arrDefault[0] == 2); // expected-warning{{TRUE}}
  clang_analyzer_eval(a.arrDefault[1] == 3); // expected-warning{{TRUE}}
  clang_analyzer_eval(a.arrDefault[2] == 0); // expected-warning{{TRUE}}
}
