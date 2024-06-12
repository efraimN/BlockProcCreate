#pragma once

#include <IApcLib.h>

typedef struct _APC_RUN_PARAMS
{
	PVOID pvSystemArgument2;
	PDRIVER_OBJECT DrvObj;
	PVOID This;
}APC_RUN_PARAMS, *PAPC_RUN_PARAMS;

typedef struct _MYApc
{
	KAPC m_KAPC;
	APC_RUN_PARAMS m_APC_RUN_PARAMS;
}MYApc, *PMYApc;

class ApcLibImp : public IApcLib
{
public:

	friend IApcLib;

	virtual
	BOOL
	Start(
		PDRIVER_OBJECT DriverObject,
		KPROCESSOR_MODE eProcessorMode,
		PFN_KERNEL_ROUTINE KernelRoutine,
		PFN_RUNDOWN_ROUTINE RundownRoutine,
		// if NormalRoutine is NULL then the eProcessorMode is ignored and set to kernel
		// (coz it will be an special kernel apc)
		PFN_NORMAL_ROUTINE NormalRoutine
	);

	virtual
	VOID
	Stop(
	);

	virtual
	BOOL
	InsertApc(
		PETHREAD ptThread,
		PVOID SystemArgument1,
		PVOID SystemArgument2,
		PVOID NormalRoutineContext,
		KPRIORITY Increment
	);

	PFN_RUNDOWN_ROUTINE m_RundownRoutine;
	PFN_KERNEL_ROUTINE m_KernelRoutine;
	PFN_NORMAL_ROUTINE m_NormalRoutine;
	KPROCESSOR_MODE m_eProcessorMode;
protected:

private:

	ApcLibImp();
	virtual~ApcLibImp();

	// delete copy and move constructors and assign operators
	ApcLibImp(ApcLibImp const&);             // Copy construct
	ApcLibImp(ApcLibImp&&);                  // Move construct
	ApcLibImp& operator=(ApcLibImp const&);  // Copy assign
	ApcLibImp& operator=(ApcLibImp&&);       // Move assign


	PDRIVER_OBJECT m_DriverObject;
	MYApc m_MYApc;
};