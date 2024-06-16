#include "Precompiled.h"
#include <WppIncludes.h>

#include <DriverEntryLib.h>

#include <ProcessList.h>
#include <DllInjection.h>
#include <ApcInject.h>

// open the following define to force injection on debug files only
// #define IN_DEBUG_TESTING


// Must be lower case
WCHAR BlackListProcs[][MAX_PATH] =
{
	L"c:\\windows\\system32\\lsaiso.exe",
	L"c:\\windows\\system32\\vmcompute.exe",
	L"c:\\windows\\system32\\vmwp.exe"
};

// Must be lower case
WCHAR DebugDoInject[][MAX_PATH] =
#ifdef IN_DEBUG_TESTING
{
	L"notepad.exe",
	L"notepad++.exe",
	L"powershell.exe",
	L"cff-explorer.exe",
};
#else
{
	L""
};
#endif

LONG volatile CDllInjection::m_ApcPendingCount;

CDllInjection::CDllInjection()
{
	m_Inited = FALSE;
	m_ApcPendingCount = 0;
}

CDllInjection::~CDllInjection()
{
}

typedef struct _DllInjectionWorkItemContext
{
	CDllInjection* This;
	ProcessDataElement* ProcListElement;
}DllInjectionWorkItemContext, *PDllInjectionWorkItemContext;

NTSTATUS CDllInjection::Start()
{
	auto ProcEntry = [](BOOL start)
	{
		if (start)
			LOG_OUT(DBG_TRACE_FUNCTIONS, ">>> Start file %!FILE! line: %!LINE!");
		else
			LOG_OUT(DBG_TRACE_FUNCTIONS, "<<< Finish file %!FILE! line: %!LINE!");

	};
	PROC_ENTRY;
	if (!m_Inited)
	{
// 		RtlInitUnicodeString(&m_Kernel32Path,
// 			L"\\Windows\\System32\\kernel32.dll" //Native 64 or 32
// 		);
// 
// 		RtlInitUnicodeString(&m_Kernel32PathWow,
// 			L"\\Windows\\SysWOW64\\kernel32.dll"// 32 on wow
// 		);

		RtlInitUnicodeString(&m_kernelBaseWowPath,
			L"\\Windows\\SysWOW64\\kernelBase.dll"
		);
		RtlInitUnicodeString(&m_kernelBasePath,
			L"\\Windows\\System32\\kernelBase.dll"
		);
		m_Inited = TRUE;
	}

	return STATUS_SUCCESS;
}

NTSTATUS CDllInjection::Stop()
{
	auto ProcEntry = [](BOOL start)
	{
		if (start)
			LOG_OUT(DBG_TRACE_FUNCTIONS, ">>> Start file %!FILE! line: %!LINE!");
		else
			LOG_OUT(DBG_TRACE_FUNCTIONS, "<<< Finish file %!FILE! line: %!LINE!");

	};
	PROC_ENTRY;
	NTSTATUS status = STATUS_UNSUCCESSFUL;

	if (m_Inited)
	{
		LOG_OUT(DBG_INFO,"The ApcPendingCount is %d",
			InterlockedCompareExchange(&m_ApcPendingCount, 0, 0)
		);

		m_Inited = FALSE;
	}

	return status;
}

void
CDllInjection::ImageLoadingCallBack(
	PUNICODE_STRING FullImageName,
	HANDLE ProcessId,
	PIMAGE_INFO ImageInfo
)
{
	UNREFERENCED_PARAMETER(ImageInfo);

	PEPROCESS ProcessObj = NULL;
	CProcessList* pProcessList = NULL;
	ProcessDataElement* pProcListElement = NULL;
	UserKernelUtilsLib::IUtilsInt* pUtilsInt;
	PUNICODE_STRING pkernelBasePath;
	// Logged and OldInjectionActive are used to log about InjectionActive policy changes but only once per change
	static BOOL Logged = FALSE;
	static INT OldInjectionActive = FALSE;
	PWCH Buffer = NULL;
	INT InjectionActive;
	DWORD dPid;

	if (!m_Inited)
	{
		goto Leave;
	}

	// If ProcessId is 0, then a kernel-mode driver image is being loaded, we're not interested in those, skip.
	if (!ProcessId)
	{
		goto Leave;
	}

	if (!FullImageName)
	{
		goto Leave;
	}

	dPid = DOWNCASTHANDLE(ProcessId);

	if (!NT_SUCCESS(PsLookupProcessByProcessId(ProcessId, &ProcessObj)))
	{
		LOG_OUT(DBG_ERROR, "ImageLoadingCallBack Failed PsLookupProcessByProcessId for PID %d", dPid);
		goto Leave;
	}
	// we can safely dereference maximum the insert apd will increase it or will fail it
	ObDereferenceObject(ProcessObj);

	if (ProcessObj != PsGetCurrentProcess())
	{
		// we are interested only in the loader events, 
		// and hence the process loading the dll is the target itself,
		// coz the loader runs in the new process context
		goto Leave;
	}

	//Check ActiveFeature policy status 
	InjectionActive = CheckIfHookActive();
	if (OldInjectionActive != InjectionActive)
	{
		OldInjectionActive = InjectionActive;
		Logged = FALSE;
	}

	if (!Logged)
	{
		if (InjectionActive)
		{
			LOG_OUT(DBG_INFO, "Doing injection coz Injection is active in policy");
		}
		else
		{
			LOG_OUT(DBG_INFO, "Skeeping injection coz not active in policy");
		}
		Logged = TRUE;
	}

	if (!InjectionActive)
	{
		goto Leave;
	}

	pUtilsInt = UserKernelUtilsLib::IUtilsInt::GetInstance();
	pProcessList = CProcessList::GetInstance();

	pProcListElement = pProcessList->GetElement(ProcessObj);
	if (!pProcListElement)
	{
		// it can happen if the process existed before the driver was loaded
		LOG_OUT(DBG_ERROR, "LoadImageInjection Failed This process not found in list for PID %d", dPid);
		goto Leave;
	}

	if (!CHeckIfIsMainExec(pProcListElement))
	{
		goto Leave;
	}

	// If we already know it is white listed, skip
	if (pProcListElement->m_DllinjectionParams.m_WhiteListProc)
	{
		goto Leave;
	}

	// If already treated skip
	if (CHeckIfHooked(pProcListElement))
	{
		goto Leave;
	}

	if (ShouldSkeepInject(pProcListElement))
	{
		goto Leave;
	}
#ifdef IN_DEBUG_TESTING
	LOG_OUT(DBG_INFO, "Debug allow inject to PID: 0x%x. Name: %S FullImageName %wZ",
		(ULONG)pProcListElement->m_ProcessPid,
		pProcListElement->m_ProccesDosExecName.Buffer,
		FullImageName
	);
#endif
	if (CheckIfSkeepHook(pProcListElement))
	{
		pProcListElement->m_DllinjectionParams.m_WhiteListProc = TRUE;
		LOG_OUT(DBG_INFO, "Skeeping injection on proc due to skeep policy %wZ", &pProcListElement->m_ProccesDosExecName);
		goto Leave;
	}

	pkernelBasePath = (pProcListElement->m_IsWow64) ? &m_kernelBaseWowPath : &m_kernelBasePath;
	// Parse only the 'kernelBaseWow.dll' image load events, return immediately on other image load events;

	if (pUtilsInt->SafeSearchString(FullImageName, pkernelBasePath, TRUE) == -1)
	{
// 		LOG_OUT(DBG_INFO, "SafeSearchString(FullImageName, pkernelBasePath) %wZ FullImageName %wZ", &pProcListElement->m_ProccesDosExecName, FullImageName);
		goto Leave;
	}

	if (pProcListElement->m_ProccesDosExecName.Buffer)
	{
		Buffer = pProcListElement->m_ProccesDosExecName.Buffer;
	}
	else
	if (pProcListElement->m_ProccesNtExecName.Buffer)
	{
		Buffer = pProcListElement->m_ProccesNtExecName.Buffer;
	}

	LOG_OUT(DBG_INFO, "Got TrigerFullImageName %wZ", FullImageName);

	LOG_OUT(DBG_INFO, "***Injecting DLL into the process with PID: 0x%x. Name: %S",
		(ULONG)pProcListElement->m_ProcessPid,
		Buffer);


	/*
	The Injection has two main phases
	one is to copy and or allocate the code in the remote process
	the second is to run it
	inside this callback context there are memory locks (at least in OS bellow 10)
	that will deadlock if try to allocate process virtual memory or map a file
	this is why we must use here an Inject Apc and then a "run" APC or a system thread
	*/

	if (!DoInjection(
		&CProcessList::m_ExitRunDown,
		&m_ApcPendingCount,
		PsGetCurrentThread(),
		pProcListElement,
		APCInjectionType,
		ImageInfo->ImageBase
	))
	{
		LOG_OUT(DBG_ERROR, "***FAILED DoInjection DLL into the process with PID: 0x%x. Name: %S",
			(ULONG)pProcListElement->m_ProcessPid,
			Buffer);
		goto Leave;
	}

Leave:
	return;
}

BOOL CDllInjection::DoInjection(
	PEX_RUNDOWN_REF ExitRunDown,
	volatile LONG* ApcPendingCount,
	PETHREAD Kernel32LoaderThread,
	ProcessDataElement* pProcListElement,
	HookType TypeOfHook,
	PVOID Shel32MapAddress
)
{
	BOOL RetVal = FALSE;
	BOOLEAN Acquaired = FALSE;

	pProcListElement->m_DllinjectionParams.m_Kernel32LoaderThread = Kernel32LoaderThread;
	pProcListElement->m_DllinjectionParams.m_InjectStarted = TRUE;

	switch (TypeOfHook)
	{
		case APCInjectionType:
		{
			if (!ExAcquireRundownProtection(ExitRunDown))
			{
				goto Leave;
			}
			Acquaired = TRUE;
			InterlockedIncrement(ApcPendingCount);
			RetVal = ApcInject::InsertInjectionApc(
				ExitRunDown,
				ApcPendingCount,
				pProcListElement,
				Shel32MapAddress
			);
		}
		break;
		case ThreadInjectionType:
		{
			// TODO
			goto Leave;
		}
		break;
		default:
		{
			goto Leave;
		}
		break;
	}

Leave:
	if (!RetVal)
	{
		pProcListElement->m_DllinjectionParams.m_InjectStarted = FALSE;
		pProcListElement->m_DllinjectionParams.m_Kernel32LoaderThread = NULL;
		pProcListElement->m_DllinjectionParams.m_WhiteListProc = FALSE;
		if (APCInjectionType == TypeOfHook)
		{
			InterlockedDecrement(ApcPendingCount);
			if (Acquaired)
			{
				ExReleaseRundownProtection(ExitRunDown);
			}
		}
	}
	return RetVal;
}

/// Util functions

INT CDllInjection::CheckIfHookActive(
)
{
// Should use a profile manager
// for now hard coded we return active
	return 1;
}

// TODO: Here is the place to add checks on white-listed policy or hard-coded white-list.
BOOL CDllInjection::CheckIfSkeepHook(
	ProcessDataElement* pProcListElement
)
{
	BOOL RetVal = TRUE;

	if (pProcListElement->m_IsProtectedProccess)
	{
		LOG_OUT(DBG_INFO, "Skiping ProtectedProccess PID %d", pProcListElement->m_ProcessPid);
		goto Leave;
	}

	if (pProcListElement->m_IsProtectedProccessLight)
	{
		LOG_OUT(DBG_INFO, "Skiping ProtectedProccessLight PID %d", pProcListElement->m_ProcessPid);
		goto Leave;
	}

	//Here is the place to add checks on white-listed policy or hard-coded white-list.

	RetVal = FALSE;
Leave:
	return RetVal;
}

BOOL CDllInjection::CHeckIfHooked(
	ProcessDataElement* pProcListElement
)
{
	return pProcListElement->m_DllinjectionParams.m_InjectStarted;
}

BOOL CDllInjection::CHeckIfIsMainExec(
	ProcessDataElement* pProcListElement
)
{
	// Declare that the image executable was loaded (happens only once per process, when the main exec file is loaded)
	if (!pProcListElement->m_DllinjectionParams.m_ImageExecutableLoaded)
	{
		pProcListElement->m_DllinjectionParams.m_ImageExecutableLoaded = TRUE;
		return FALSE;
	}
	return TRUE;
}

BOOL CDllInjection::ShouldSkeepInject(
	ProcessDataElement* pProcListElement
)
{
	BOOL RetVal = TRUE;
#ifndef IN_DEBUG_TESTING
	RetVal = FALSE; 
#endif
	UserKernelUtilsLib::IUtilsInt* pUtilsInt = UserKernelUtilsLib::IUtilsInt::GetInstance();
	UNICODE_STRING TmpUniStr;

#ifdef IN_DEBUG_TESTING
	for (UINT i=0;i<ARRAYSIZE(DebugDoInject);i++)
	{
		RtlInitUnicodeString(&TmpUniStr, DebugDoInject[i]);
		if (pUtilsInt->SafeSearchString(&pProcListElement->m_ProccesDosExecName, &TmpUniStr, TRUE) != -1)
		{
			RetVal = FALSE;
			break;
		}
	}
#else
	for (UINT i = 0; i < ARRAYSIZE(BlackListProcs); i++)
	{
		RtlInitUnicodeString(&TmpUniStr, BlackListProcs[i]);
		if (pUtilsInt->SafeSearchString(&pProcListElement->m_ProccesDosExecName, &TmpUniStr, TRUE) != -1)
		{
			RetVal = TRUE;
			break;
		}
	}
#endif
	return RetVal;
}

