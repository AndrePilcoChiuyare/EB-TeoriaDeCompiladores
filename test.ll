; ModuleID = 'EB'
source_filename = "EB"

declare i32 @puts(ptr)

define void @_main_() {
entry:
  %b = alloca double, align 8
  %a = alloca double, align 8
  store double 7.000000e+00, ptr %a, align 8
  store double 9.000000e+00, ptr %b, align 8
  ret void
}
