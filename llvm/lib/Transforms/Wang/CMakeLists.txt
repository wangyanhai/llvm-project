add_llvm_library( LLVMWangPass 
  WangPass.cpp

  ADDITIONAL_HEADER_DIRS
  ${LLVM_MAIN_INCLUDE_DIR}/llvm/Transforms

  DEPENDS
  intrinsics_gen
#  PLUGIN_TOOL
#  opt
  )
