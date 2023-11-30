; ModuleID = 'EB'
source_filename = "EB"

@0 = private unnamed_addr constant [1 x i8] zeroinitializer, align 1
@1 = private unnamed_addr constant [1 x i8] zeroinitializer, align 1

declare i32 @puts(ptr)

define void @_main_() {
entry:
  %b = alloca double, align 8
  %a = alloca double, align 8
  store double 7.000000e+00, ptr %a, align 8
  store double 9.000000e+00, ptr %b, align 8
  %callSystem = call i32 @puts(ptr @0)
  %callSystem1 = call i32 @puts(ptr @1)
  ret void
}
