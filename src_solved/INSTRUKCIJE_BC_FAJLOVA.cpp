===================================== 1.bc ============================================
Inicijalizacija

  %1 = alloca i32, align 4
  %2 = alloca i32, align 4
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  store i32 0, i32* %1, align 4
  store i32 42, i32* %2, align 4
  %5 = load i32, i32* %2, align 4
  %6 = load i32, i32* %2, align 4
  %7 = add nsw i32 %5, %6
  store i32 %7, i32* %3, align 4
  %8 = load i32, i32* %3, align 4
  %9 = load i32, i32* %3, align 4
  %10 = add nsw i32 %8, %9
  %11 = load i32, i32* %3, align 4
  %12 = load i32, i32* %3, align 4
  %13 = sub nsw i32 %11, %12
  %14 = sub nsw i32 %10, %13
  store i32 %14, i32* %4, align 4
  %15 = load i32, i32* %4, align 4
  %16 = load i32, i32* %3, align 4
  %17 = add nsw i32 %15, %16
  ret i32 %17

Kraj inicijalizacije. Vracam listu_intervala
======================================= 2.bc ==========================================
Inicijalizacija

  %1 = alloca i32, align 4
  %2 = alloca i32, align 4
  %3 = alloca i32, align 4
  store i32 0, i32* %1, align 4
  store i32 987, i32* %2, align 4
  %4 = load i32, i32* %2, align 4
  %5 = load i32, i32* %2, align 4
  %6 = add nsw i32 %4, %5
  store i32 %6, i32* %3, align 4
  %7 = load i32, i32* %3, align 4
  ret i32 %7

Kraj inicijalizacije. Vracam listu_intervala
======================================= 3.bc ==========================================
Inicijalizacija

  %1 = alloca i32, align 4
  %2 = alloca i32, align 4
  store i32 0, i32* %1, align 4
  store i32 17, i32* %2, align 4
  %3 = load i32, i32* %2, align 4
  %4 = load i32, i32* %2, align 4
  %5 = add nsw i32 %3, %4
  ret i32 %5

Kraj inicijalizacije. Vracam listu_intervala

============================================ 4.bc ========================================
I:   %tmp = alloca i32, align 4 [opcode: 29]
I:   %tmp1 = alloca i32, align 4 [opcode: 29]
I:   %tmp2 = alloca i32, align 4 [opcode: 29]
I:   %tmp3 = alloca i32, align 4 [opcode: 29]
I:   %tmp4 = alloca i32, align 4 [opcode: 29]
I:   %tmp5 = alloca i32, align 4 [opcode: 29]
I:   store i32 0, i32* %tmp, align 4 [opcode: 31]
I:   store i32 3, i32* %tmp1, align 4 [opcode: 31]
I:   %tmp6 = load i32, i32* %tmp1, align 4 [opcode: 30]
I:   %tmp7 = load i32, i32* %tmp1, align 4 [opcode: 30]
I:   %tmp8 = add nsw i32 %tmp6, %tmp7 [opcode: 11]
I:   store i32 %tmp8, i32* %tmp2, align 4 [opcode: 31]
I:   %tmp9 = load i32, i32* %tmp2, align 4 [opcode: 30]
I:   %tmp10 = load i32, i32* %tmp1, align 4 [opcode: 30]
I:   %tmp11 = add nsw i32 %tmp9, %tmp10 [opcode: 11]
I:   store i32 %tmp11, i32* %tmp3, align 4 [opcode: 31]
I:   %tmp12 = load i32, i32* %tmp3, align 4 [opcode: 30]
I:   %tmp13 = load i32, i32* %tmp1, align 4 [opcode: 30]
I:   %tmp14 = add nsw i32 %tmp12, %tmp13 [opcode: 11]
I:   store i32 %tmp14, i32* %tmp4, align 4 [opcode: 31]
I:   %tmp15 = load i32, i32* %tmp4, align 4 [opcode: 30]
I:   %tmp16 = sub nsw i32 %tmp15, 5 [opcode: 13]
I:   store i32 %tmp16, i32* %tmp5, align 4 [opcode: 31]
I:   %tmp17 = load i32, i32* %tmp5, align 4 [opcode: 30]
I:   %tmp18 = sub nsw i32 %tmp17, 5 [opcode: 13]
I:   ret i32 %tmp18 [opcode: 1]




