#pragma once

extern "C" 
{
	/*
	This functions are used to wrap the "C++";
	the assembly code calls them
	and then jumps directly to the ObDereferenceObject function
	*/

	// Imported by the asm code

	PDRIVER_OBJECT
	NTAPI
	_CppApcRunDownRoutine(
		PRKAPC Apc
	);

	PDRIVER_OBJECT
	NTAPI
	_CppApcKernelRoutine(
		PKAPC Apc,
		PFN_NORMAL_ROUTINE* NormalRoutine,
		PVOID* NormalContext,
		PVOID* SystemArgument1,
		PVOID* /*SystemArgument2*/
	);

	PDRIVER_OBJECT
	NTAPI
	_CppApcNormalRutine(
		PVOID NormalContext,
		PVOID SystemArgument1,
		PVOID SystemArgument2
	);


	// exported by the asm code
	VOID
	NTAPI
	_AsmApcRunDownRoutine(
		PRKAPC Apc
	);

	VOID
	NTAPI
	_AsmApcKernelRoutine(
		PKAPC Apc,
		PFN_NORMAL_ROUTINE* NormalRoutine,
		PVOID* NormalContext,
		PVOID* SystemArgument1,
		PVOID* SystemArgument2
	);

	VOID
	NTAPI
	_AsmApcNormalRutine(
		PVOID NormalContext,
		PVOID SystemArgument1,
		PVOID SystemArgument2
	);

}
