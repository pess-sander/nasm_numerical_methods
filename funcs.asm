section .data
	const3 dq 3.0

section .bss
	tmp resq 1

section .text
global f1, f2, f3
f1:
    push ebp
    mov ebp, esp

    finit
    fld qword [ebp+8] ; st0 = x
	fld st0 ; st1 = x
	frndint ; st1 = [x]
	fxch    ; st0 = [x], st1 = x
	fsub st1 ; st0 = {x}
	f2xm1   ; st0 = 2^{x} - 1
	fld1    ; st0 = 1.0
	faddp   ; st0 = 2^{x}
	fscale  ; st0 = 2^x
	fld1    ; st0 = 1
	faddp   ; st0 = 2^x + 1
	
	fxch
	fstp qword [tmp]

    leave
    ret

f2:
	push ebp
	mov ebp, esp

	finit
	fld qword [ebp+8] ; st0 = x
	fld st0 ; st1 = x
	fmul st0, st1 ; st0 = x^2
	fmul st0, st1 ; st0 = x^3
	fmul st0, st1 ; st0 = x^4
	fmulp ; st0 = x^5

	leave
    ret

f3:
    push ebp
	mov ebp, esp

	finit
	fld qword [ebp+8] ; st0 = x
	fld1 ; st0 = 1.0
	fxch ; st0 = x, st1 = 1.0
	fsubp ; st0 = 1-x
	fld qword [const3] ; st0 = 3.0
	fdivp ; st0 = (1-x)/3

	leave
    ret
