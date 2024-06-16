#include "Precompiled.h"
#include <WppIncludes.h>

#include "ApcLib.h"
#include "ApcLibAsm.h"


IApcLib* IApcLib::GetNewInstance()
{
	return new ('ApcL') ApcLibImp();
}

VOID IApcLib::ReleaseInstance(
	IApcLib* Instance
)
{
	if (Instance)
	{
		delete (ApcLibImp*)Instance;
	}
}

ApcLibImp::ApcLibImp()
{
	m_KernelRoutine = NULL;
	m_RundownRoutine = NULL;
	m_NormalRoutine = NULL;
};

ApcLibImp::~ApcLibImp()
{
}

BOOL
ApcLibImp::Start(
	PDRIVER_OBJECT DriverObject,
	KPROCESSOR_MODE eProcessorMode,
	PFN_KERNEL_ROUTINE KernelRoutine,
	PFN_RUNDOWN_ROUTINE RundownRoutine,
	// if NormalRoutine is NULL then the eProcessorMode is ignored and set to kernel
	// (coz it will be an special kernel apc)
	PFN_NORMAL_ROUTINE NormalRoutine
)
{
	m_DriverObject = DriverObject;
	m_eProcessorMode = eProcessorMode;
	m_KernelRoutine = KernelRoutine;
	m_RundownRoutine = RundownRoutine;
	m_NormalRoutine = NormalRoutine;
	return TRUE;
}

VOID
ApcLibImp::Stop(
)
{
}

BOOL
ApcLibImp::InsertApc(
	PETHREAD ptThread,
	PVOID SystemArgument1,
	PVOID SystemArgument2,
	PVOID NormalRoutineCOntext,
	KPRIORITY Increment
)
{
	BOOLEAN RetVal = FALSE;
	BOOLEAN Incremented = FALSE;
	PFN_NORMAL_ROUTINE NormalRoutine;

	if (UserMode == m_eProcessorMode)
	{
		if (!m_NormalRoutine)
		{
			LOG_OUT(DBG_ERROR, "Error a usermode APC must have a m_NormalRoutine");
			goto Leave;
		}
		NormalRoutine = m_NormalRoutine;
	}
	else
	{
		NormalRoutine = m_NormalRoutine ? _AsmApcNormalRutine : NULL;
	}

	KeInitializeApc(
		&m_MYApc.m_KAPC,
		ptThread,
		OriginalApcEnvironment,
		_AsmApcKernelRoutine,
		_AsmApcRunDownRoutine,
		NormalRoutine,
		m_NormalRoutine ? m_eProcessorMode : KernelMode,
		NormalRoutineCOntext
	);

	m_MYApc.m_APC_RUN_PARAMS.pvSystemArgument2 = SystemArgument2;
	m_MYApc.m_APC_RUN_PARAMS.DrvObj = m_DriverObject;
	m_MYApc.m_APC_RUN_PARAMS.This = this;

	ObReferenceObject(m_DriverObject);
	Incremented = TRUE;

	RetVal = KeInsertQueueApc(
		&m_MYApc.m_KAPC,
		SystemArgument1,
		&m_MYApc.m_APC_RUN_PARAMS,
		Increment
	);

Leave:
	if (!RetVal)
	{
		if (Incremented)
		{
			ObDereferenceObject(m_DriverObject);
		}
	}
	return RetVal;
}

PDRIVER_OBJECT
NTAPI
_CppApcRunDownRoutine(
	PRKAPC Apc
)
{
	PAPC_RUN_PARAMS pPAPC_RUN_PARAMS = (PAPC_RUN_PARAMS)Apc->SystemArgument2;
	ApcLibImp* pApcLibImp;
	PDRIVER_OBJECT DrvObj;
	PFN_RUNDOWN_ROUTINE RundownRoutine;

	// be aware after return, the ApcLib class and data may be not available.
	// this routine is static and the code will be here, but if needed any data from the ApcLib
	// must be cached here
	DrvObj = pPAPC_RUN_PARAMS->DrvObj;
	pApcLibImp = (ApcLibImp*)pPAPC_RUN_PARAMS->This;
	Apc->SystemArgument2 = pPAPC_RUN_PARAMS->pvSystemArgument2;
	RundownRoutine = pApcLibImp->m_RundownRoutine;

	RundownRoutine(
		Apc
	);
	
	return DrvObj;
}

PDRIVER_OBJECT
NTAPI
_CppApcNormalRutine(
	PVOID NormalContext,
	PVOID SystemArgument1,
	PVOID SystemArgument2
)
{
	PAPC_RUN_PARAMS pPAPC_RUN_PARAMS = (PAPC_RUN_PARAMS)SystemArgument2;
	ApcLibImp* pApcLibImp;
	PDRIVER_OBJECT DrvObj;
	PFN_NORMAL_ROUTINE NormalRoutine;

	// be aware after return, the ApcLib class and data may be not available.
	// this routine is static and the code will be here, but if needed any data from the ApcLib
	// must be cached here
	DrvObj = pPAPC_RUN_PARAMS->DrvObj;
	pApcLibImp = (ApcLibImp*)pPAPC_RUN_PARAMS->This;
	NormalRoutine = pApcLibImp->m_NormalRoutine;
	
	NormalRoutine(
		NormalContext,
		SystemArgument1,
		pPAPC_RUN_PARAMS->pvSystemArgument2
	);

	return DrvObj;
}

PDRIVER_OBJECT
NTAPI
_CppApcKernelRoutine(
	PKAPC Apc,
	PFN_NORMAL_ROUTINE* NormalRoutine,
	PVOID* NormalContext,
	PVOID* SystemArgument1,
	PVOID* SystemArgument2
)
{
	PAPC_RUN_PARAMS pPAPC_RUN_PARAMS = (PAPC_RUN_PARAMS)(*(PVOID*)SystemArgument2);

	ApcLibImp* pApcLibImp;
	PDRIVER_OBJECT DrvObj;
	KPROCESSOR_MODE eProcessorMode;
	PFN_KERNEL_ROUTINE KernelRoutine;

	// be aware after return, the ApcLib class and data may be not available.
	// this routine is static and the code will be here, but if needed any data from the ApcLib
	// must be cached here
	DrvObj = pPAPC_RUN_PARAMS->DrvObj;
	pApcLibImp = (ApcLibImp*)pPAPC_RUN_PARAMS->This;
	eProcessorMode = pApcLibImp->m_eProcessorMode;

	KernelRoutine = pApcLibImp->m_KernelRoutine;

	KernelRoutine(
		Apc,
		NormalRoutine,
		NormalContext,
		SystemArgument1,
		&pPAPC_RUN_PARAMS->pvSystemArgument2
	);

	// a user mode APC always has a normal routine, but it runs in user space
	// so we must decrement here
	// a kernel mode APC may or may not have a normal routine.
	// if a kernel mode APC has normal routine then instead of dereferencing here 
	// we let the dereference to the normal routine

	if (KernelMode == eProcessorMode)
	{
		if (*NormalRoutine != NULL)
		{
			// Since it is a kernel mode and there is a normal routine, let the normal routine to decrement the DrvObj
			DrvObj = NULL;
		}
	}
	else
	{
		// is a usermode routine, there will be NOT a kernel NormalRoutine, will be a usermode one
		*SystemArgument2 = pPAPC_RUN_PARAMS->pvSystemArgument2;
	}

	return DrvObj;
}
