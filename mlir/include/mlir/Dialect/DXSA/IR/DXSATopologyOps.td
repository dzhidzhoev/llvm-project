//===- DXSTopologyOps.td - DXSA topology ops ---------------*- tablegen -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Topology instructions of the DXSA dialect.
//
//===----------------------------------------------------------------------===//

#ifndef MLIR_DIALECT_DXSA_IR_DXSATOPOLOGYOPS
#define MLIR_DIALECT_DXSA_IR_DXSATOPOLOGYOPS

include "mlir/Dialect/DXSA/IR/DXSAOpBase.td"

//===----------------------------------------------------------------------===//
// dxsa.emit
//===----------------------------------------------------------------------===//

def DXSA_Emit : DXSA_NullaryOp<"emit"> {
  let summary = "outputs a geometry-shader vertex from declared output registers";
  let description = [{
    The `dxsa.emit` operation reads every declared `o#` output register and
    outputs a vertex from the geometry shader. Each `dxsa.emit` produces one
    vertex; successive emits assemble primitives according to the output
    topology declared by `dxsa.dcl_output_topology`.

    A geometry shader may issue `dxsa.emit` any number of times, including
    inside flow control. If output streams have been declared with
    `dxsa.dcl_stream`, `dxsa.emit_stream` must be used instead.

    Example:

    ```mlir
    dxsa.emit
    ```
  }];
}

//===----------------------------------------------------------------------===//
// dxsa.cut
//===----------------------------------------------------------------------===//

def DXSA_Cut : DXSA_NullaryOp<"cut"> {
  let summary = "ends the current geometry-shader output strip";
  let description = [{
    The `dxsa.cut` operation ends the current output strip at the last emitted
    vertex, so the next `dxsa.emit` begins a new strip. This only affects
    linestrip and trianglestrip output topologies; for pointlist output,
    `dxsa.cut` has no effect.

    A geometry shader may issue `dxsa.cut` any number of times, including
    inside flow control. If output streams have been declared with
    `dxsa.dcl_stream`, `dxsa.cut_stream` must be used instead.

    Example:

    ```mlir
    dxsa.cut
    ```
  }];
}

//===----------------------------------------------------------------------===//
// dxsa.emit_then_cut
//===----------------------------------------------------------------------===//

def DXSA_EmitThenCut : DXSA_NullaryOp<"emit_then_cut"> {
  let summary = "outputs a geometry-shader vertex and ends the current strip";
  let description = [{
    The `dxsa.emit_then_cut` operation has the same effect as `dxsa.emit`,
    immediately followed by `dxsa.cut`.

    A geometry shader may issue `dxsa.emit_then_cut` any number of times,
    including inside flow control. If output streams have been declared with
    `dxsa.dcl_stream`, `dxsa.emit_then_cut_stream` must be used instead.

    Example:

    ```mlir
    dxsa.emit_then_cut
    ```
  }];
}

#endif // MLIR_DIALECT_DXSA_IR_DXSATOPOLOGYOPS
