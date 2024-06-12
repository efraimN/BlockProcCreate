.386
.MODEL FLAT
.safeseh SEH_handler

extern _ObDereferenceObject@4	:PROC

extern __CppApcKernelRoutine@20	:PROC
extern __CppApcRunDownRoutine@4	:PROC
extern __CppApcNormalRutine@12	:PROC

PUBLIC __AsmApcKernelRoutine@20
PUBLIC __AsmApcRunDownRoutine@4
PUBLIC __AsmApcNormalRutine@12

JmpObDereferenceObject MACRO __Funct__ : REQ, NumParams : REQ
	; This will copy the stack down, i.e. duplicating it for the call
	REPEAT (NumParams)
		mov ecx, dword ptr [esp+NumParams*4];
		push ecx
	ENDM
		call __Funct__ 
		; we need to clean the stack we receive coz we are _stdcall
		; for this we first recover our return address
		pop ecx
		; then restore the stack
		add esp,NumParams*4
		test    eax,eax
		je DoRet

		push eax; push the driverobject param
		push ecx
		Jmp _ObDereferenceObject@4 ; this function is stdcall it will clean the satck and return
ENDM

       .CODE

		ORG 0

; the library will use the SystemArgument1 as a pointer to an internal structure, alocated on the same blob as the KAPC


;VOID
;NTAPI
;__AsmApcKernelRoutine(
;	PKAPC Apc,
;	PFN_NORMAL_ROUTINE* NormalRoutine,
;	PVOID* NormalContext,
;	PVOID* SystemArgument1,
;	PVOID* SystemArgument2
;);
__AsmApcKernelRoutine@20 PROC
	JmpObDereferenceObject __CppApcKernelRoutine@20, 5
__AsmApcKernelRoutine@20 ENDP

;VOID
;NTAPI
;__AsmApcRunDownRoutine(
;	PRKAPC Apc
;);
__AsmApcRunDownRoutine@4 PROC
	JmpObDereferenceObject __CppApcRunDownRoutine@4, 1
__AsmApcRunDownRoutine@4 ENDP


;VOID
;NTAPI
;__AsmApcNormalRutine(
;	PVOID NormalContext,
;	PVOID SystemArgument1,
;	PVOID SystemArgument2
;);
__AsmApcNormalRutine@12 PROC
	JmpObDereferenceObject __CppApcNormalRutine@12, 3
__AsmApcNormalRutine@12 ENDP


DoRet:
		push ecx
		ret

SEH_handler   proc
ret
SEH_handler   endp


END
