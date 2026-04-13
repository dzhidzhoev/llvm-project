set(LLVM_TARGET_DEFINITIONS DXSAOps.td)
add_mlir_dialect(DXSAOps dxsa)
add_mlir_doc(DXSAOps DXSADialect Dialects/ -gen-dialect-doc)
