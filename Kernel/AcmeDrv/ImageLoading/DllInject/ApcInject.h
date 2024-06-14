#pragma once

#include <IApcLib.h>
#include "ProcessList.h"

class ApcInject
{
public:
	static
	BOOL RunApcInjection(
		PEX_RUNDOWN_REF ExitRunDown,
		volatile LONG* ApcPendingCount,
		ProcessDataElement* pProcListElement,
		PVOID Shel32MapAddress
	);


private:

	typedef struct _APC_PARAMS
	{
		PEX_RUNDOWN_REF ExitRunDown;
		volatile LONG* ApcPendingCount;
		ProcessDataElement* pProcListElement;
		PVOID Shel32MapAddress;
	}APC_PARAMS, * PAPC_PARAMS;

	static
	BOOL
	RunApcCommon(
		PAPC_PARAMS pAPC_PARAMS,
		IApcLib** ppIApcLib,
		KPROCESSOR_MODE eProcessorMode,
		PFN_KERNEL_ROUTINE KernelRoutine,
		PFN_RUNDOWN_ROUTINE RundownRoutine,
		PFN_NORMAL_ROUTINE NormalRoutine,
		PVOID NormalRoutineContext
	);

	static
	VOID
	NTAPI
	InjectApcRunDownRoutine(
		PRKAPC Apc
	);

	static
	VOID
	NTAPI
	InjectApcKernelRoutine(
		PKAPC Apc,
		PFN_NORMAL_ROUTINE* NormalRoutine,
		PVOID* NormalContext,
		PVOID* SystemArgument1,
		PVOID* SystemArgument2
	);

	static
	VOID
	NTAPI
	InjectApcNormalRutine(
		PVOID NormalContext,
		PVOID SystemArgument1,
		PVOID SystemArgument2
	);

	static
	VOID
	InjectApcCleanup(
		PAPC_PARAMS pAPC_PARAMS
	);

	static
	BOOL RunUserApc(
		PAPC_PARAMS pAPC_PARAMS,
		PFN_NORMAL_ROUTINE pNormalUsermodeRoutine,
		PVOID NormalRoutineCOntext
		);

	static
	VOID
	NTAPI
	UserApcRunDownRoutine(
		PRKAPC Apc
	);

	static
	VOID
	NTAPI
	UserApcKernelRoutine(
		PKAPC Apc,
		PFN_NORMAL_ROUTINE* NormalRoutine,
		PVOID* NormalContext,
		PVOID* SystemArgument1,
		PVOID* SystemArgument2
	);

	static
	VOID
	UserApcCleanup(
		PAPC_PARAMS pAPC_PARAMS
	);

};

