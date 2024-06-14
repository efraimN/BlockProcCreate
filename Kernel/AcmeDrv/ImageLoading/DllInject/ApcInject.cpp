#include "Precompiled.h"
#include <WppIncludes.h>

#include "ApcInject.h"

#include <DriverEntryLib.h>

#define LOG_APC_ROUTINES

/// Inject apc is used to map the shelcode into the target process
// Is a kernel mode normal apc, that runs the NormalRutine in kernel passive mode
// we use it to inject the data into the process
VOID
ApcInject::InjectApcCleanup(
	PAPC_PARAMS pAPC_PARAMS
)
{
#ifdef LOG_APC_ROUTINES
	auto ProcEntry = [](BOOL start)
	{
		if (start)
			LOG_OUT(DBG_TRACE_FUNCTIONS, ">>> Start file %!FILE! line: %!LINE!");
		else
			LOG_OUT(DBG_TRACE_FUNCTIONS, "<<< Finish file %!FILE! line: %!LINE!");

	};
	PROC_ENTRY;

	LOG_OUT(DBG_INFO,"InjectApcCleanup");
#endif // LOG_APC_ROUTINES
	ProcessDataElement* pProcListElement = pAPC_PARAMS->pProcListElement;

	if (pProcListElement->m_DllinjectionParams.m_InjectionKernelAPC)
	{
		pProcListElement->m_DllinjectionParams.m_InjectionKernelAPC->Stop();
		IApcLib::ReleaseInstance(pProcListElement->m_DllinjectionParams.m_InjectionKernelAPC);
		pProcListElement->m_DllinjectionParams.m_InjectionKernelAPC = NULL;
	}
}

VOID
NTAPI
ApcInject::InjectApcRunDownRoutine(
	PRKAPC Apc
)
{
	PAPC_PARAMS pAPC_PARAMS = (PAPC_PARAMS)Apc->SystemArgument1;
#ifdef LOG_APC_ROUTINES
	ProcessDataElement* pProcessDataElement = pAPC_PARAMS->pProcListElement;

	auto ProcEntry = [](BOOL start)
	{
		if (start)
			LOG_OUT(DBG_TRACE_FUNCTIONS, ">>> Start file %!FILE! line: %!LINE!");
		else
			LOG_OUT(DBG_TRACE_FUNCTIONS, "<<< Finish file %!FILE! line: %!LINE!");

	};
	PROC_ENTRY;

	LOG_OUT(DBG_INFO,"Called InjectApcRunDownRoutine for Proc %wZ", &pProcessDataElement->m_ProccesDosExecName);
#endif // LOG_APC_ROUTINES

	InjectApcCleanup(pAPC_PARAMS);
	InterlockedDecrement(pAPC_PARAMS->ApcPendingCount);
	ExReleaseRundownProtection(pAPC_PARAMS->ExitRunDown);

	delete 	pAPC_PARAMS;
}

VOID
NTAPI
ApcInject::InjectApcKernelRoutine(
	PKAPC Apc,
	PFN_NORMAL_ROUTINE* NormalRoutine,
	PVOID* NormalContext,
	PVOID* SystemArgument1,
	PVOID* SystemArgument2
)
{
	UNREFERENCED_PARAMETER(Apc);
	UNREFERENCED_PARAMETER(NormalRoutine);
	UNREFERENCED_PARAMETER(NormalContext);
	UNREFERENCED_PARAMETER(SystemArgument2);

#ifndef LOG_APC_ROUTINES
	UNREFERENCED_PARAMETER(SystemArgument1);
#else
	PAPC_PARAMS pAPC_PARAMS = (PAPC_PARAMS)*SystemArgument1;
	ProcessDataElement* pProcessDataElement = pAPC_PARAMS->pProcListElement;

	auto ProcEntry = [](BOOL start)
	{
		if (start)
			LOG_OUT(DBG_TRACE_FUNCTIONS, ">>> Start file %!FILE! line: %!LINE!");
		else
			LOG_OUT(DBG_TRACE_FUNCTIONS, "<<< Finish file %!FILE! line: %!LINE!");

	};

	PROC_ENTRY;
	LOG_OUT(DBG_INFO, "Called InjectApcKernelRoutine for Proc %wZ", &pProcessDataElement->m_ProccesDosExecName);
#endif // LOG_APC_ROUTINES

}

VOID
NTAPI
ApcInject::InjectApcNormalRutine(
	PVOID NormalContext,
	PVOID SystemArgument1,
	PVOID SystemArgument2
)
{
	// here we are in the correct thread context, and the correct process, also the IRQ passive
	UNREFERENCED_PARAMETER(NormalContext);
	UNREFERENCED_PARAMETER(SystemArgument2);

	auto ProcEntry = [](BOOL start)
	{
		if (start)
			LOG_OUT(DBG_TRACE_FUNCTIONS, ">>> Start file %!FILE! line: %!LINE!");
		else
			LOG_OUT(DBG_TRACE_FUNCTIONS, "<<< Finish file %!FILE! line: %!LINE!");

	};
	PAPC_PARAMS pAPC_PARAMS = (PAPC_PARAMS)SystemArgument1;
	ProcessDataElement* pProcessDataElement = pAPC_PARAMS->pProcListElement;

	PROC_ENTRY;
	LOG_OUT(DBG_INFO, "InjectApcNormalRutine for Proc %wZ", &pProcessDataElement->m_ProccesDosExecName);

	UserKernelUtilsLib::IParsePe* pParsePe = UserKernelUtilsLib::IParsePe::GetInstance();

	PVOID ShellCodeContextData = NULL;
	PVOID pShellCodeApcProcAddress = NULL;
	BOOL Succeed = FALSE;

	pShellCodeApcProcAddress = pParsePe->InMemoryGetProcAddress(
		pAPC_PARAMS->Shel32MapAddress
		, "LoadLibraryExW", 
		ZwCurrentProcess()
	);


	{
		WCHAR DllName[] =  L"c:\\drivers\\hdll.dll" ;
		PWSTR UserModeDllPath = NULL;
		size_t UserModeDllPathLeng = sizeof(DllName);
		NTSTATUS status;

		status = ZwAllocateVirtualMemory(
			ZwCurrentProcess(),
			(PVOID*)&UserModeDllPath,
			0,
			&UserModeDllPathLeng,
			MEM_RESERVE | MEM_COMMIT,
			PAGE_READWRITE
		);
		if (!NT_SUCCESS(status))
		{
			goto Leave;
		}

		RtlCopyMemory(UserModeDllPath, DllName, sizeof(DllName));
		ShellCodeContextData = UserModeDllPath;
	}

	if (!RunUserApc(
		pAPC_PARAMS,
		(PFN_NORMAL_ROUTINE)pShellCodeApcProcAddress,
		ShellCodeContextData
	))
	{
		LOG_OUT(DBG_INFO, L"error on RunUserApcfor Proc %wZ",
			&pProcessDataElement->m_ProccesDosExecName);
		goto Leave;
	}

	Succeed = TRUE;
Leave:
	// at the end we must clean up
	InjectApcCleanup(pAPC_PARAMS);
	if (!Succeed)
	{
		LOG_OUT(DBG_INFO,L"Error calling on InjectApcNormalRutine for Proc %wZ",
			&pProcessDataElement->m_ProccesDosExecName);
		InterlockedDecrement(pAPC_PARAMS->ApcPendingCount);
		ExReleaseRundownProtection(pAPC_PARAMS->ExitRunDown);
		if (pAPC_PARAMS)
		{
			delete 	pAPC_PARAMS;
		}
	}
}

BOOL ApcInject::RunApcInjection(
	PEX_RUNDOWN_REF ExitRunDown,
	volatile LONG* ApcPendingCount,
	ProcessDataElement* pProcListElement,
	PVOID Shel32MapAddress
)
{
	auto ProcEntry = [](BOOL start)
	{
		if (start)
			LOG_OUT(DBG_TRACE_FUNCTIONS, ">>> Start file %!FILE! line: %!LINE!");
		else
			LOG_OUT(DBG_TRACE_FUNCTIONS, "<<< Finish file %!FILE! line: %!LINE!");

	};
	PROC_ENTRY;

	LOG_OUT(DBG_INFO, "Called RunApcInjection for Proc %wZ", &pProcListElement->m_ProccesDosExecName);

	PAPC_PARAMS pAPC_PARAMS;
	pAPC_PARAMS = new ('pCpA') APC_PARAMS;
	BOOL RetVal = FALSE;

	if (!pAPC_PARAMS)
	{
		LOG_OUT(DBG_ERROR,"new APC_PARAMS FAiled");
		goto Leave;
	}

	pAPC_PARAMS->ExitRunDown = ExitRunDown;
	pAPC_PARAMS->ApcPendingCount = ApcPendingCount;
	pAPC_PARAMS->pProcListElement = pProcListElement;
	pAPC_PARAMS->Shel32MapAddress = Shel32MapAddress;


	pProcListElement->m_DllinjectionParams.m_InjectionUserAPC = NULL;
	pProcListElement->m_DllinjectionParams.m_InjectionKernelAPC = NULL;

	if(!RunApcCommon(
		pAPC_PARAMS,
		&pProcListElement->m_DllinjectionParams.m_InjectionKernelAPC,
		KernelMode,
		InjectApcKernelRoutine,
		InjectApcRunDownRoutine,
		InjectApcNormalRutine,
		NULL
	))
	{
		goto Leave;
	}

	RetVal = TRUE;
Leave:
	if (!RetVal)
	{
		InjectApcCleanup(pAPC_PARAMS);
	}
	return RetVal;
}



/// UserApc to call the shelcode to load the dll
VOID
ApcInject::UserApcCleanup(
	PAPC_PARAMS pAPC_PARAMS
)
{
#ifdef LOG_APC_ROUTINES
	auto ProcEntry = [](BOOL start)
	{
		if (start)
			LOG_OUT(DBG_TRACE_FUNCTIONS, ">>> Start file %!FILE! line: %!LINE!");
		else
			LOG_OUT(DBG_TRACE_FUNCTIONS, "<<< Finish file %!FILE! line: %!LINE!");

	};
	PROC_ENTRY;
	LOG_OUT(DBG_INFO, "UserApcCleanup");
#endif // LOG_APC_ROUTINES
	ProcessDataElement* pProcListElement = pAPC_PARAMS->pProcListElement;

	if (pProcListElement->m_DllinjectionParams.m_InjectionUserAPC)
	{
		pProcListElement->m_DllinjectionParams.m_InjectionUserAPC->Stop();
		IApcLib::ReleaseInstance(pProcListElement->m_DllinjectionParams.m_InjectionUserAPC);
		pProcListElement->m_DllinjectionParams.m_InjectionUserAPC = NULL;
	}
}

VOID
NTAPI
ApcInject::UserApcRunDownRoutine(
	PRKAPC Apc
)
{
	PAPC_PARAMS pAPC_PARAMS = (PAPC_PARAMS)Apc->SystemArgument1;
#ifdef LOG_APC_ROUTINES
	ProcessDataElement* pProcessDataElement = pAPC_PARAMS->pProcListElement;

	auto ProcEntry = [](BOOL start)
	{
		if (start)
			LOG_OUT(DBG_TRACE_FUNCTIONS, ">>> Start file %!FILE! line: %!LINE!");
		else
			LOG_OUT(DBG_TRACE_FUNCTIONS, "<<< Finish file %!FILE! line: %!LINE!");

	};
	PROC_ENTRY;
	LOG_OUT(DBG_INFO, "Called UserApcRunDownRoutine for Proc %wZ", &pProcessDataElement->m_ProccesDosExecName);
#endif // LOG_APC_ROUTINES

	UserApcCleanup(pAPC_PARAMS);
	InterlockedDecrement(pAPC_PARAMS->ApcPendingCount);
	ExReleaseRundownProtection(pAPC_PARAMS->ExitRunDown);
	delete 	pAPC_PARAMS;
}

VOID
NTAPI
ApcInject::UserApcKernelRoutine(
	PKAPC /*Apc*/,
	PFN_NORMAL_ROUTINE* NormalRoutine,
	PVOID* NormalContext,
	PVOID* SystemArgument1,
	PVOID* SystemArgument2
)
{
	auto ProcEntry = [](BOOL start)
	{
		if (start)
			LOG_OUT(DBG_TRACE_FUNCTIONS, ">>> Start file %!FILE! line: %!LINE!");
		else
			LOG_OUT(DBG_TRACE_FUNCTIONS, "<<< Finish file %!FILE! line: %!LINE!");

	};
	PROC_ENTRY;
	BOOL Success = FALSE;
	NTSTATUS status;
	PAPC_PARAMS pAPC_PARAMS = (PAPC_PARAMS)*SystemArgument1;
	*SystemArgument1 = NULL;
	ProcessDataElement* pProcessDataElement = pAPC_PARAMS->pProcListElement;
	pAPC_PARAMS->pProcListElement->m_DllinjectionParams;
	LOG_OUT(DBG_INFO, "Called UserApcKernelRoutine for Proc %wZ", &pProcessDataElement->m_ProccesDosExecName);

	// Skip execution if the thread is terminating
	if (PsIsThreadTerminating(PsGetCurrentThread()))
	{
		goto Leave;
	}

	// Fix Wow64 APC
	LOG_OUT(DBG_DEBUG, "Before WOW64 Wrapping: NormalRoutine = 0x%p ; Arguments: 0x%p and 0x%p ; NormalContext = 0x%p.", *NormalRoutine, *SystemArgument1, *SystemArgument2, *NormalContext);
		status = PsWrapApcWow64Thread(NormalContext, (PVOID*)NormalRoutine);
	if (!NT_SUCCESS(status))
	{
		LOG_OUT(DBG_DEBUG, "Failed to call PsWrapApcWow64Thread in the APC's KernelRoutine - status code: 0x%x.", status);
		// TODO in this case we should insert another apc to remove the actual copy of the dll and user data
		// It can't be done here coz the IRQ is too high
		goto Leave;
	}
	LOG_OUT(DBG_DEBUG, "After WOW64 Wrapping: NormalRoutine = 0x%p ; Arguments: 0x%p and 0x%p ; NormalContext = 0x%p.", *NormalRoutine, *SystemArgument1, *SystemArgument2, *NormalContext);

	Success = TRUE;
Leave:
	if (!Success)
	{
		*NormalRoutine = NULL;
	}
	// should clean here coz the Userapc runs in user mode and cant acces this data
	UserApcCleanup(pAPC_PARAMS);
	InterlockedDecrement(pAPC_PARAMS->ApcPendingCount);
	ExReleaseRundownProtection(pAPC_PARAMS->ExitRunDown);
	delete 	pAPC_PARAMS;
}

BOOL ApcInject::RunUserApc(
	PAPC_PARAMS pAPC_PARAMS,
	PFN_NORMAL_ROUTINE pNormalUsermodeRoutine,
	PVOID NormalRoutineContext
)
{
#ifdef LOG_APC_ROUTINES
	auto ProcEntry = [](BOOL start)
	{
		if (start)
			LOG_OUT(DBG_TRACE_FUNCTIONS, ">>> Start file %!FILE! line: %!LINE!");
		else
			LOG_OUT(DBG_TRACE_FUNCTIONS, "<<< Finish file %!FILE! line: %!LINE!");

	};
	PROC_ENTRY;

	LOG_OUT(DBG_INFO, "RunUserApc");
#endif // LOG_APC_ROUTINES
	BOOL RetVal = FALSE;

	if (!RunApcCommon(
		pAPC_PARAMS,
		&pAPC_PARAMS->pProcListElement->m_DllinjectionParams.m_InjectionUserAPC,
		UserMode,
		UserApcKernelRoutine,
		UserApcRunDownRoutine,
		pNormalUsermodeRoutine,
		NormalRoutineContext
	))
	{
		goto Leave;
	}

	RetVal = TRUE;
Leave:
	if (!RetVal)
	{
		UserApcCleanup(pAPC_PARAMS);
	}
	return RetVal;
}

BOOL ApcInject::RunApcCommon(
	PAPC_PARAMS pAPC_PARAMS,
	IApcLib** ppIApcLib,
	KPROCESSOR_MODE eProcessorMode,
	PFN_KERNEL_ROUTINE KernelRoutine,
	PFN_RUNDOWN_ROUTINE RundownRoutine,
	PFN_NORMAL_ROUTINE NormalRoutine,
	PVOID NormalRoutineContext
)
{
#ifdef LOG_APC_ROUTINES
	LOG_OUT(DBG_INFO, "RunApcCommon");
#endif // LOG_APC_ROUTINES
	BOOL RetVal = FALSE;
	IApcLib* pIApcLib = NULL;

	*ppIApcLib = NULL; // just on case

	pIApcLib = IApcLib::GetNewInstance();
	if (!pIApcLib)
	{
		LOG_OUT(DBG_ERROR, "RunApcCommon IApcLib::GetNewInstance failed Not injecting to process %S",
			pAPC_PARAMS->pProcListElement->m_ProccesDosExecName.Buffer);
		goto Leave;
	}

	if (!pIApcLib->Start(
		DriverEntryLib::G_DriverObject,
		eProcessorMode,
		KernelRoutine,
		RundownRoutine,
		NormalRoutine
	))
	{
		LOG_OUT(DBG_ERROR, "RunApcCommon pIApcLib->Start failed Not injecting to process %S",
			pAPC_PARAMS->pProcListElement->m_ProccesDosExecName.Buffer);
		goto Leave;
	}

	if (!pIApcLib->InsertApc(
		pAPC_PARAMS->pProcListElement->m_DllinjectionParams.m_Kernel32LoaderThread,
		pAPC_PARAMS,
		NULL,
		NormalRoutineContext,
		IO_NO_INCREMENT
	))
	{
		LOG_OUT(DBG_ERROR, "RunApcCommon pIApcLib->InsertApc failed Not injecting to process %S",
			pAPC_PARAMS->pProcListElement->m_ProccesDosExecName.Buffer);
		goto Leave;
	}

	*ppIApcLib = pIApcLib;
	RetVal = TRUE;
Leave:
	return RetVal;
}
