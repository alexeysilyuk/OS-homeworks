	.file	"file2.c"
	.intel_syntax noprefix
	.section	.rodata.str1.1,"aMS",@progbits,1
.LC0:
	.string	"HELLO WORLD in main2!"
	.section	.text.unlikely,"ax",@progbits
.LCOLDB1:
	.text
.LHOTB1:
	.globl	callme
	.type	callme, @function
callme:
.LFB13:
	.cfi_startproc
	push	rax
	.cfi_def_cfa_offset 16
	mov	edi, OFFSET FLAT:.LC0
	call	puts
	xor	eax, eax
	pop	rdx
	.cfi_def_cfa_offset 8
	ret
	.cfi_endproc
.LFE13:
	.size	callme, .-callme
	.section	.text.unlikely
.LCOLDE1:
	.text
.LHOTE1:
	.section	.text.unlikely
.LCOLDB2:
	.section	.text.startup,"ax",@progbits
.LHOTB2:
	.globl	main
	.type	main, @function
main:
.LFB14:
	.cfi_startproc
	push	rax
	.cfi_def_cfa_offset 16
	xor	eax, eax
	call	callme
	mov	eax, 1
	pop	rdx
	.cfi_def_cfa_offset 8
	ret
	.cfi_endproc
.LFE14:
	.size	main, .-main
	.section	.text.unlikely
.LCOLDE2:
	.section	.text.startup
.LHOTE2:
	.ident	"GCC: (Ubuntu 5.4.0-6ubuntu1~16.04.9) 5.4.0 20160609"
	.section	.note.GNU-stack,"",@progbits
