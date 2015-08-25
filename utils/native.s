	.text
	.file	"utils/native.c"
	.globl	outb
	.align	16, 0x90
	.type	outb,@function
outb:                                   # @outb
# BB#0:                                 # %entry
	pushl	%ebp
	movl	%esp, %ebp
	movzwl	8(%ebp), %edx
	movb	12(%ebp), %al
                                        # kill: DX<def> DX<kill> EDX<kill>
	#APP
	outb	%al, %dx
	#NO_APP
	popl	%ebp
	retl
.Ltmp0:
	.size	outb, .Ltmp0-outb

	.globl	inb
	.align	16, 0x90
	.type	inb,@function
inb:                                    # @inb
# BB#0:                                 # %entry
	pushl	%ebp
	movl	%esp, %ebp
	movzwl	8(%ebp), %edx
                                        # kill: DX<def> DX<kill> EDX<kill>
	#APP
	inb	%dx, %al
	#NO_APP
	movzbl	%al, %eax
	popl	%ebp
	retl
.Ltmp1:
	.size	inb, .Ltmp1-inb

	.globl	outw
	.align	16, 0x90
	.type	outw,@function
outw:                                   # @outw
# BB#0:                                 # %entry
	pushl	%ebp
	movl	%esp, %ebp
	movzwl	8(%ebp), %edx
	movzwl	12(%ebp), %eax
                                        # kill: DX<def> DX<kill> EDX<kill>
                                        # kill: AX<def> AX<kill> EAX<kill>
	#APP
	outw	%ax, %dx
	#NO_APP
	popl	%ebp
	retl
.Ltmp2:
	.size	outw, .Ltmp2-outw

	.globl	inw
	.align	16, 0x90
	.type	inw,@function
inw:                                    # @inw
# BB#0:                                 # %entry
	pushl	%ebp
	movl	%esp, %ebp
	movzwl	8(%ebp), %edx
                                        # kill: DX<def> DX<kill> EDX<kill>
	#APP
	inw	%dx, %ax
	#NO_APP
	movzwl	%ax, %eax
	popl	%ebp
	retl
.Ltmp3:
	.size	inw, .Ltmp3-inw

	.globl	outl
	.align	16, 0x90
	.type	outl,@function
outl:                                   # @outl
# BB#0:                                 # %entry
	pushl	%ebp
	movl	%esp, %ebp
	movzwl	8(%ebp), %edx
	movl	12(%ebp), %eax
                                        # kill: DX<def> DX<kill> EDX<kill>
	#APP
	outl	%eax, %dx
	#NO_APP
	popl	%ebp
	retl
.Ltmp4:
	.size	outl, .Ltmp4-outl

	.globl	inl
	.align	16, 0x90
	.type	inl,@function
inl:                                    # @inl
# BB#0:                                 # %entry
	pushl	%ebp
	movl	%esp, %ebp
	movzwl	8(%ebp), %edx
                                        # kill: DX<def> DX<kill> EDX<kill>
	#APP
	inl	%dx, %eax
	#NO_APP
	popl	%ebp
	retl
.Ltmp5:
	.size	inl, .Ltmp5-inl

	.globl	wrmsr
	.align	16, 0x90
	.type	wrmsr,@function
wrmsr:                                  # @wrmsr
# BB#0:                                 # %entry
	pushl	%ebp
	movl	%esp, %ebp
	movl	12(%ebp), %eax
	movl	16(%ebp), %edx
	movl	8(%ebp), %ecx
	#APP
	wrmsr
	#NO_APP
	popl	%ebp
	retl
.Ltmp6:
	.size	wrmsr, .Ltmp6-wrmsr

	.globl	rdmsr
	.align	16, 0x90
	.type	rdmsr,@function
rdmsr:                                  # @rdmsr
# BB#0:                                 # %entry
	pushl	%ebp
	movl	%esp, %ebp
	movl	8(%ebp), %ecx
	#APP
	rdmsr
	#NO_APP
	popl	%ebp
	retl
.Ltmp7:
	.size	rdmsr, .Ltmp7-rdmsr


	.ident	"clang version 3.6.2 (tags/RELEASE_362/final)"
	.section	".note.GNU-stack","",@progbits
