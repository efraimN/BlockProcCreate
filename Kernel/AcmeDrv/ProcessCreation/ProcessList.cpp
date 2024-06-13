#include "Precompiled.h"
#include <WppIncludes.h>

#include "ProcessList.h"

#include <KernelUtilsLib.h>
using namespace KernelUtilsLib;

#include <IUserKernelUtils.h>
using namespace UserKernelUtilsLib;

__declspec(align(8))
EX_RUNDOWN_REF CProcessList::m_ExitRunDown;



BOOL CProcessList::InitElement(ProcessDataElement* Element)
{
	BOOL RetVal = FALSE;

	IProcessUtils* pProcesssUtils = NULL;
	PUNICODE_STRING Tmp;
	NTSTATUS status;
	HANDLE hProcess = NULL;

	Element->m_ProcessPid = DOWNCASTHANDLE(PsGetProcessId(Element->m_ProcessObj));

	Element->m_ProccesNtExecName.MaximumLength = MAX_PATH_;
	Element->m_ProccesNtExecName.Buffer = Element->m_ProccesNtExecNameBuff;
	Element->m_ProccesNtExecName.Length = 0;

	Element->m_ProccesDosExecName.Length = 0;
	Element->m_ProccesDosExecName.MaximumLength = MAX_PATH_;
	Element->m_ProccesDosExecName.Buffer = Element->m_ProccesDosExecNameBuff;


	Element->m_DllinjectionParams.m_InjectStarted = FALSE;
	Element->m_DllinjectionParams.m_ImageExecutableLoaded = FALSE;
	Element->m_DllinjectionParams.m_Kernel32LoaderThread = FALSE;
	Element->m_DllinjectionParams.m_InjectionUserAPC = NULL;
	Element->m_DllinjectionParams.m_InjectionKernelAPC = NULL;
	Element->m_DllinjectionParams.m_WhiteListProc = FALSE;

	status = ObOpenObjectByPointer(
		Element->m_ProcessObj,
		OBJ_KERNEL_HANDLE,
		NULL,
		PROCESS_QUERY_INFORMATION,
		*PsProcessType, 
		KernelMode, 
		&hProcess
	);

	if (!UserKernelUtilsLib::IUtilsInt::GetInstance()->IsProcessWow64(hProcess, &Element->m_IsWow64))
	{
		LOG_OUT(DBG_ERROR, "IUtilsInt->IsProcessWow64 Failed ProcessPid %d", Element->m_ProcessPid);
		goto Leave;
	}

	pProcesssUtils = IProcessUtils::GetNewInstance(Element->m_ProcessObj);
	if (!pProcesssUtils)
	{
		LOG_OUT(DBG_ERROR, "IProcessUtils::GetNewInstance Failed ProcessPid %d", Element->m_ProcessPid);
		goto Leave;
	}

	Tmp = pProcesssUtils->GetNtFullPathName();
	if (Tmp)
	{
		status = RtlUnicodeStringCopy(&Element->m_ProccesNtExecName, Tmp);
		if (!NT_SUCCESS(status))
		{
			Element->m_ProccesNtExecName.Length = 0;
			LOG_OUT(DBG_ERROR, "RtlUnicodeStringCopy Failed m_ProccesNtExecNameBuff %S status 0x%08X", Tmp->Buffer, status);
// 			goto Leave;
		}
	}
	else
	{
		Element->m_ProccesNtExecName.Length = 0;
		LOG_OUT(DBG_ERROR, "GetNtFullPathName Failed ProcessPid %d", Element->m_ProcessPid);
// 		goto Leave;
	}

	Tmp = pProcesssUtils->GetDosFullPathName();
	if (Tmp)
	{
		status = RtlUnicodeStringCopy(&Element->m_ProccesDosExecName, Tmp);
		if (!NT_SUCCESS(status))
		{
			Element->m_ProccesDosExecName.Length = 0;
			LOG_OUT(DBG_ERROR, "RtlUnicodeStringCopy Failed m_ProccesDosExecNameBuff %S status 0x%08X", Tmp->Buffer, status);
		}
	}
	else
	{
		Element->m_ProccesDosExecName.Length = 0;
		LOG_OUT(DBG_ERROR, "GetDosFullPathName Failed ProcessPid %d", Element->m_ProcessPid);
	}

	if (!Element->m_ProccesDosExecName.Length)
	{
		if (Element->m_ProccesNtExecName.Length)
		{
			Element->m_ProccesDosExecName.Length = Element->m_ProccesNtExecName.Length;
			Element->m_ProccesDosExecName.MaximumLength = Element->m_ProccesNtExecName.MaximumLength;
			Element->m_ProccesDosExecName.Buffer = Element->m_ProccesNtExecNameBuff;
			LOG_OUT(DBG_ERROR, "Will use m_ProccesNtExecName %S instead of the m_ProccesDosExecName", Element->m_ProccesNtExecNameBuff);
		}
	}

	for (UINT Index = 0; Index < (Element->m_ProccesNtExecName.Length / sizeof(WCHAR)); ++Index)
	{
		Element->m_ProccesNtExecName.Buffer[Index] = RtlDowncaseUnicodeChar(Element->m_ProccesNtExecName.Buffer[Index]);
	}

	for (UINT Index = 0; Index < (Element->m_ProccesDosExecName.Length / sizeof(WCHAR)); ++Index)
	{
		Element->m_ProccesDosExecName.Buffer[Index] = RtlDowncaseUnicodeChar(Element->m_ProccesDosExecName.Buffer[Index]);
	}

	Element->m_CreationTime = PsGetProcessCreateTimeQuadPart(Element->m_ProcessObj);

	Tmp = NULL;

	ObReferenceObject(Element->m_ProcessObj);

	Element->m_IsProtectedProccess = PsIsProtectedProcess(Element->m_ProcessObj);

	Element->m_IsProtectedProccessLight = pProcesssUtils->PsIsProtectedProcessLight(Element->m_ProcessObj);

	RetVal = TRUE;
Leave:
	if (pProcesssUtils)
	{
		IProcessUtils::ReleaseInstance(pProcesssUtils);
	}

	if (hProcess)
	{
		ZwClose(hProcess);
	}
	return RetVal;
}

VOID CProcessList::ReleaseElement(ProcessDataElement* Element)
{
	if (Element->m_ProcessObj)
	{
		ObDereferenceObject(Element->m_ProcessObj);
	}
}

CProcessList::CProcessList()
{
}

CProcessList::~CProcessList()
{
}

CProcessList* CProcessList::GetInstance()
{
	static CProcessList SingleTone;
	return &SingleTone;
}

BOOLEAN CProcessList::Start()
{
	BOOLEAN RetVal = FALSE;
	size_t ListMemPol;

	m_pProcessList = ILinkListLibInt::GetNewInstance();

	if (!m_pProcessList)
	{
		goto Leave;
	}

	if (!m_pProcessList->Start())
	{
		ILinkListLibInt::FreeInstance(m_pProcessList);
		m_pProcessList = NULL;
		goto Leave;
	}

	m_ProcessListElementsPool = IMemPoolMgr::GetNewInstance();
	if (!m_ProcessListElementsPool)
	{
		goto Leave;
	}

	ListMemPol = m_ProcessListElementsPool->GetRequiredMemoryZoneLength(sizeof(ProcessDataElement), 1000);
	LOG_OUT(DBG_INFO, "Required ListMemPool is %llu", ListMemPol);
	m_ListElementsPoolMem = new ('tsiL') BYTE[ListMemPol];
	if (!m_ListElementsPoolMem)
	{
		LOG_OUT(DBG_ERROR, "Failing allocating m_ListElementsPoolMem");
		goto Leave;
	}

	if(!m_ProcessListElementsPool->Start(m_ListElementsPoolMem, ListMemPol, sizeof(ProcessDataElement), 1000))
	{
		LOG_OUT(DBG_ERROR, "Failing m_ProcessListElementsPool->Start");
		goto Leave;
	}


	ExInitializeRundownProtection(&m_ExitRunDown);

	RetVal = TRUE;
Leave:
	return RetVal;
}

VOID CProcessList::Stop()
{
	if (!m_pProcessList)
	{
		return;
	}

	LOG_OUT(DBG_INFO, "CDllInjection::Stop() ExWaitForRundownProtectionRelease Before Wait");
	// We will wait here till all the Apc/Workitems are finished, to avoid destroy objects they may depend on
	ExWaitForRundownProtectionRelease(&m_ExitRunDown);
	LOG_OUT(DBG_INFO, "CDllInjection::Stop() ExWaitForRundownProtectionRelease After Wait");

	RemoveAllElements();
	m_pProcessList->Stop();
	ILinkListLibInt::FreeInstance(m_pProcessList);

	if (m_ProcessListElementsPool)
	{
		IMemPoolMgr::FreeInstance(m_ProcessListElementsPool);
	}
	if (m_ListElementsPoolMem)
	{
		delete[](PBYTE)m_ListElementsPoolMem;
	}
}



BOOL CProcessList::AddElement(PEPROCESS ProcessObj, HANDLE /*ParentProcessId*/)
{
	BOOL RetVal = FALSE;
	ProcessDataElement* Element = NULL;

	Element = (ProcessDataElement*)m_ProcessListElementsPool->AllocateFromPool();
	if (!Element)
	{
		LOG_OUT(DBG_ERROR, "%!FUNC! AllocateFromPool Failed new element for PID %d", DOWNCASTHANDLE(PsGetProcessId(ProcessObj)));
		goto Leave;
	}

	Element->m_ProcessObj = ProcessObj;
	Element->m_RefCount = 0;


	if (!InitElement(Element))
	{
		LOG_OUT(DBG_ERROR, "%!FUNC! InitElement Failed SourceProcessPid %d", DOWNCASTHANDLE(PsGetProcessId(ProcessObj)));
		goto Leave;
	}

	m_pProcessList->PushItemHead(Element);

	RetVal = TRUE;

Leave:
	if (!RetVal)
	{
		if (Element)
		{
			m_ProcessListElementsPool->FreeFromPool(Element);
		}
	}
	return RetVal;
}

static
BOOLEAN _cdecl GetElementCallBack(ILinkListElement* Element, PVOID FindContext)
{
	ProcessDataElement* pElement = (ProcessDataElement * )Element;
	PEPROCESS ProcessObj = (PEPROCESS)FindContext;

	return (pElement->m_ProcessObj == ProcessObj)? TRUE :FALSE;
}

BOOL CProcessList::RemoveElement(PEPROCESS ProcessObj)
{
	BOOL RetVal = FALSE;
	ProcessDataElement* Element;
	Element = (ProcessDataElement * )m_pProcessList->GetElement(GetElementCallBack, ProcessObj, TRUE);
	if (!Element)
	{
		LOG_OUT(DBG_ERROR, "CProcessList::RemoveElement GetElement Failed Not found element for PID %d", DOWNCASTHANDLE(PsGetProcessId(ProcessObj)));
		goto Leave;
	}
	ReleaseElement(Element);

	m_ProcessListElementsPool->FreeFromPool(Element);

	RetVal = TRUE;
Leave:
	return RetVal;
}

ProcessDataElement* CProcessList::GetElement(PEPROCESS ProcessObj)
{
	ProcessDataElement* Element;

	Element = (ProcessDataElement*)m_pProcessList->GetElement(GetElementCallBack, ProcessObj, FALSE);
	if (!Element)
	{
// 		LOG_OUT(DBG_ERROR, "CProcessList::GetElement GetElement Failed Not found element for PID %d", DOWNCASTHANDLE(PsGetProcessId(Element->m_ProcessObj)));
	}

	return Element;
}

VOID CProcessList::RemoveAllElements()
{
	ProcessDataElement* pElement = NULL;
	for (;;)
	{
		pElement = (ProcessDataElement*)m_pProcessList->PopItemHead();
		if (!pElement)
		{
			break;
		}

		ReleaseElement(pElement);
		m_ProcessListElementsPool->FreeFromPool(pElement);
	}

}

ProcessDataElement* CProcessList::GetElementEx(
	UserKernelUtilsLib::ILinkListLibInt::pfnFoundElementEx GetFoundElement,
	PVOID FindContext,
	BOOLEAN Removal)
{
	return (ProcessDataElement*)
		m_pProcessList->GetElementEx(GetFoundElement, FindContext, Removal);
}
