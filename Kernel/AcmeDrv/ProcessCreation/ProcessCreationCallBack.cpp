#include "Precompiled.h"
#include <WppIncludes.h>

#include "ProcessCreationCallBack.h"
#include "ProcessList.h"

#include <KernelUtilsLib.h>

CProcCreateCallback* CProcCreateCallback::GetInstance()
{
	static CProcCreateCallback s_ProcCreateCallback;
	return &s_ProcCreateCallback;
}

CProcCreateCallback::CProcCreateCallback()
{
}

CProcCreateCallback::~CProcCreateCallback()
{
}

NTSTATUS CProcCreateCallback::Start()
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


	status = PsSetCreateProcessNotifyRoutineEx(ProcessNotifyCallBack_, FALSE);
	if (!NT_SUCCESS(status))
	{
		LOG_OUT(DBG_ERROR, "Start PsSetCreateProcessNotifyRoutineEx Failed");
		goto Leave;
	}

Leave:
	return status;
}

NTSTATUS CProcCreateCallback::Stop()
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

	status = PsSetCreateProcessNotifyRoutineEx(ProcessNotifyCallBack_, TRUE);

	return status;
}

void __stdcall CProcCreateCallback::ProcessNotifyCallBack_(
	_Inout_  PEPROCESS              Process,
	_In_     HANDLE                 ProcessId,
	_In_opt_ PPS_CREATE_NOTIFY_INFO CreateInfo
)
{
	CProcCreateCallback* pProcCreateCallback = CProcCreateCallback::GetInstance();
	pProcCreateCallback->ProcessNotifyCallBack(
		Process,
		ProcessId,
		CreateInfo
	);
}

void CProcCreateCallback::ProcessNotifyCallBack(
	_Inout_  PEPROCESS              Process,
	_In_     HANDLE                 ProcessId,
	_In_opt_ PPS_CREATE_NOTIFY_INFO CreateInfo
)
{
	CProcessList* pProcessList = NULL;
	ProcessDataElement* pProcessListElement = NULL;
	DWORD Pid;
	// If ProcessId is 0, then a kernel-mode driver image is being loaded, we're not interested in those, skip.
	if (!ProcessId)
	{
		goto Leave;
	}

	Pid = DOWNCASTHANDLE(ProcessId);
	// Skip unsuccessful creation process 
	pProcessList = CProcessList::GetInstance();
	if (CreateInfo)
	{
		//////////////////
		// Create Process
		//////////////////

		// Skip unsuccessful creation process 
		if (!NT_SUCCESS(CreateInfo->CreationStatus))
		{
			LOG_OUT(DBG_INFO, "Ignoring proccess PID %d", Pid);
			goto Leave;
		}
		if (pProcessList->AddElement(Process, CreateInfo->ParentProcessId))
		{
			pProcessListElement = pProcessList->GetElement(Process);
		}
		if (!pProcessListElement)
		{
			LOG_OUT(DBG_ERROR, "GetElement Failure PID %d", Pid);
			goto Leave;
		}

		LOG_OUT(DBG_INFO, "*Added proccess PID = %d Parent PID =  %d FIlePath %wZ",
			pProcessListElement->m_ProcessPid,
			pProcessListElement->m_ParentProcessPid,
			&pProcessListElement->m_ProccesDosExecName
			);
	}
	else
	{
		//////////////////
		// Exit Process
		//////////////////

		pProcessListElement = pProcessList->GetElement(Process);
		if (!pProcessListElement)
		{
			LOG_OUT(DBG_ERROR, "exit process callback failing the GetElement for PID %d while in ", Pid);
			goto Leave;
		}

		LOG_OUT(DBG_INFO, "*Exit proccess  PID = %d FIlePath %wZ", 
			pProcessListElement->m_ProcessPid,
			&pProcessListElement->m_ProccesDosExecName
		);

		pProcessList->RemoveElement(Process);
	}

Leave:
	return;
}

