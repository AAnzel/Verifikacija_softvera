===================================== 3.bc ============================================

I:   %tmp = alloca i32, align 4 [opcode: 29]
I:   %tmp1 = alloca i32, align 4 [opcode: 29]
I:   store i32 0, i32* %tmp, align 4 [opcode: 31]
I:   store i32 17, i32* %tmp1, align 4 [opcode: 31]
I:   %tmp2 = load i32, i32* %tmp1, align 4 [opcode: 30]
I:   %tmp3 = load i32, i32* %tmp1, align 4 [opcode: 30]
I:   %tmp4 = add nsw i32 %tmp2, %tmp3 [opcode: 11]
I:   ret i32 %tmp4 [opcode: 1]

======================================= 2.bc ==========================================

I:   %tmp = alloca i32, align 4 [opcode: 29]
I:   %tmp1 = alloca i32, align 4 [opcode: 29]
I:   %tmp2 = alloca i32, align 4 [opcode: 29]
I:   store i32 0, i32* %tmp, align 4 [opcode: 31]
I:   store i32 987, i32* %tmp1, align 4 [opcode: 31]
I:   %tmp3 = load i32, i32* %tmp1, align 4 [opcode: 30]
I:   %tmp4 = load i32, i32* %tmp1, align 4 [opcode: 30]
I:   %tmp5 = add nsw i32 %tmp3, %tmp4 [opcode: 11]
I:   store i32 %tmp5, i32* %tmp2, align 4 [opcode: 31]
I:   %tmp6 = load i32, i32* %tmp2, align 4 [opcode: 30]
I:   ret i32 %tmp6 [opcode: 1]

======================================= 1.bc ==========================================

I:   %tmp = alloca i32, align 4 [opcode: 29]
I:   %tmp1 = alloca i32, align 4 [opcode: 29]
I:   %tmp2 = alloca i32, align 4 [opcode: 29]
I:   %tmp3 = alloca i32, align 4 [opcode: 29]
I:   store i32 0, i32* %tmp, align 4 [opcode: 31]
I:   store i32 42, i32* %tmp1, align 4 [opcode: 31]
I:   %tmp4 = load i32, i32* %tmp1, align 4 [opcode: 30]
I:   %tmp5 = load i32, i32* %tmp1, align 4 [opcode: 30]
I:   %tmp6 = add nsw i32 %tmp4, %tmp5 [opcode: 11]
I:   store i32 %tmp6, i32* %tmp2, align 4 [opcode: 31]
I:   %tmp7 = load i32, i32* %tmp2, align 4 [opcode: 30]
I:   %tmp8 = load i32, i32* %tmp2, align 4 [opcode: 30]
I:   %tmp9 = add nsw i32 %tmp7, %tmp8 [opcode: 11]
I:   %tmp10 = load i32, i32* %tmp2, align 4 [opcode: 30]
I:   %tmp11 = load i32, i32* %tmp2, align 4 [opcode: 30]
I:   %tmp12 = sub nsw i32 %tmp10, %tmp11 [opcode: 13]
I:   %tmp13 = sub nsw i32 %tmp9, %tmp12 [opcode: 13]
I:   store i32 %tmp13, i32* %tmp3, align 4 [opcode: 31]
I:   %tmp14 = load i32, i32* %tmp3, align 4 [opcode: 30]
I:   %tmp15 = load i32, i32* %tmp2, align 4 [opcode: 30]
I:   %tmp16 = add nsw i32 %tmp14, %tmp15 [opcode: 11]
I:   ret i32 %tmp16 [opcode: 1]
