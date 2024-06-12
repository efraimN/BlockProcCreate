
extern ObDereferenceObject	:PROC

extern _CppApcKernelRoutine		:PROC
extern _CppApcRunDownRoutine	:PROC
extern _CppApcNormalRutine	:PROC

PUBLIC _AsmApcKernelRoutine
PUBLIC _AsmApcRunDownRoutine
PUBLIC _AsmApcNormalRutine

	;Fastcall 64 bit convention
	;func1(int a, int b, int c, int d, int e);  
	; a in RCX, b in RDX, c in R8, d in R9, e pushed on stack

	; Build the stack for the forwarded function call.
	; Shadow stack is 4*8 = 32 bytes. Every parameter from the 5th onwards starts from there:
	; for example, the 5th parameter would be "rsp + 32 + 8".
	; the 6th parameter is at "rsp + 32 + 8*2".
	; Every other parameter would be in increments of 8 bytes.
	; The following two opcodes copy the last parameter to the top of the stack.
	; Since the RSP decreases for each push, it will copy the next parameter each time
	; it runs.

	; This macro is ok only for functions with an odd number of parameters
	; if an even number of params then increment by one
JmpObDereferenceObject MACRO __Funct__ : REQ, NumParams1 : REQ
	IF NumParams1 le 4
		NumParams = 5
	ELSE
		NumParams = NumParams1
	EndIf
		
	REPEAT (NumParams-4)
		mov rax, qword ptr [rsp+(NumParams-4)*8 + 32];
		push rax
	ENDM
		sub rsp,  32; 
		call __Funct__
		add rsp,  32 + (NumParams-4)*8 ; this will clean the stack we prepared to call our C Function
		; At this point the stack is the stack required for the original caller 
		; the original caller will clean it
		
		mov     rcx, rax

		test    eax,eax
		jne     ObDereferenceObject
		ret
ENDM

       .CODE

		ORG 0

; the library will use the SystemArgument1 as a pointer to an internal structure, alocated on the same blob as the KAPC

;VOID
;NTAPI
;_AsmApcKernelRoutine(
;	PKAPC Apc,
;	PFN_NORMAL_ROUTINE* NormalRoutine,
;	PVOID* NormalContext,
;	PVOID* SystemArgument1,
;	PVOID* SystemArgument2
;);
_AsmApcKernelRoutine PROC
	JmpObDereferenceObject _CppApcKernelRoutine, 5
_AsmApcKernelRoutine ENDP

;VOID
;NTAPI
;_AsmApcRunDownRoutine(
;	PRKAPC Apc
;);
_AsmApcRunDownRoutine PROC
	JmpObDereferenceObject _CppApcRunDownRoutine, 1
_AsmApcRunDownRoutine ENDP


;VOID
;NTAPI
;_AsmApcNormalRutine(
;	PVOID NormalContext,
;	PVOID SystemArgument1,
;	PVOID SystemArgument2
;);
_AsmApcNormalRutine PROC
	JmpObDereferenceObject _CppApcNormalRutine, 3
_AsmApcNormalRutine ENDP



END