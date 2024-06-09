#include "Precompiled.h"
#include <WppIncludes.h>

#include "ProcessCreationCallBack.h"

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
	KernelUtilsLib::IProcessUtils* pProcesssUtils = NULL;
	PUNICODE_STRING Tmp;
	UNICODE_STRING Tmp1;
	DWORD Pid;
	NTSTATUS status;

	Tmp1.Buffer = NULL;
	// If ProcessId is 0, then a kernel-mode driver image is being loaded, we're not interested in those, skip.
	if (!ProcessId)
	{
		goto Leave;
	}

	Pid = DOWNCASTHANDLE(ProcessId);
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
		pProcesssUtils = KernelUtilsLib::IProcessUtils::GetNewInstance(Process);
		if (!pProcesssUtils)
		{
			LOG_OUT(DBG_ERROR, "IProcessUtils::GetNewInstance Failed ProcessPid %d", Pid);
			goto Leave;
		}
		Tmp = pProcesssUtils->GetDosFullPathName();
		if (!Tmp)
		{
			LOG_OUT(DBG_ERROR, "IProcessUtils::GetDosFullPathName Failed ProcessPid %d", Pid);
		}
		Tmp1.Buffer = new ('1pmt') WCHAR[Tmp->MaximumLength];
		if (!Tmp1.Buffer)
		{
			LOG_OUT(DBG_ERROR, "new WCHAR Failed ProcessPid %d", Pid);
		}
		Tmp1.MaximumLength = Tmp->MaximumLength;
		status = RtlUnicodeStringCopy(&Tmp1, Tmp);
		if (!NT_SUCCESS(status))
		{
			LOG_OUT(DBG_ERROR, "RtlUnicodeStringCopy Failed m_ProccesDosExecNameBuff %S status 0x%08X", Tmp->Buffer, status);
		}
		for (UINT Index = 0; Index < (Tmp->Length / sizeof(WCHAR)); ++Index)
		{
			Tmp1.Buffer[Index] = RtlDowncaseUnicodeChar(Tmp->Buffer[Index]);
		}
		LOG_OUT(DBG_ERROR, "***Found Proccess %S***", Tmp1.Buffer);

	}
	else
	{
		//////////////////
		// Exit Process
		//////////////////

		LOG_OUT(DBG_INFO, "*Exit proccess  PID = %d", Pid);
	}

Leave:
	if (pProcesssUtils)
	{
		KernelUtilsLib::IProcessUtils::ReleaseInstance(pProcesssUtils);
	}
	if (Tmp1.Buffer)
	{
		delete[] Tmp1.Buffer;
	}
	return;
}

