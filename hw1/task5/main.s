	.file	"main.c"
	.intel_syntax noprefix
	.section	.rodata.str1.1,"aMS",@progbits,1
.LC0:
	.string	"HELLO WORLD!"
.LC1:
	.string	"%d\n"
	.section	.text.unlikely,"ax",@progbits
.LCOLDB2:
	.section	.text.startup,"ax",@progbits
.LHOTB2:
	.globl	main
	.type	main, @function
main:
.LFB13:
	.cfi_startproc
	push	rax
	.cfi_def_cfa_offset 16
	mov	edi, OFFSET FLAT:.LC0
	call	puts
	mov	edx, 8
	mov	esi, OFFSET FLAT:.LC1
	mov	edi, 1
	xor	eax, eax
	call	__printf_chk
	xor	eax, eax
	pop	rdx
	.cfi_def_cfa_offset 8
	ret
	.cfi_endproc
.LFE13:
	.size	main, .-main
	.section	.text.unlikely
.LCOLDE2:
	.section	.text.startup
.LHOTE2:
	.ident	"GCC: (Ubuntu 5.4.0-6ubuntu1~16.04.9) 5.4.0 20160609"
	.section	.note.GNU-stack,"",@progbits
