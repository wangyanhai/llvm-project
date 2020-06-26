; ModuleID = 'test function'
source_filename = "test function"

@global_var = external global [1146880 x i8]

define void @function1() {
  %1 = alloca i32, align 4
  store i32 1098488218, i32* %1, align 4
  %2 = bitcast i32* %1 to float*
  %3 = load float, float* %2, align 4
  %4 = load float, float* bitcast (i8* getelementptr inbounds ([1146880 x i8], [1146880 x i8]* @global_var, i32 0, i32 4096) to float*)
  %5 = fadd float %3, %4
  store float %5, float* bitcast (i8* getelementptr inbounds ([1146880 x i8], [1146880 x i8]* @global_var, i32 0, i32 4096) to float*)
  ret void
}
