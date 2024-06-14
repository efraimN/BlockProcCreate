#pragma once

#include <ImageLoadingCallBackClient.h>
#include <ProcessList.h>

class CDllInjection : public ImageLoadingCallBackClient
{
public:

	enum HookType
	{
		APCInjectionType = 1,
		ThreadInjectionType, // Not yet implemented, needed for first time loading
	};

	virtual
	NTSTATUS Start();

	virtual
	NTSTATUS Stop();

	virtual
	void
	ImageLoadingCallBack(
		PUNICODE_STRING FullImageName,
		HANDLE ProcessId,
		PIMAGE_INFO ImageInfo
	);

	CDllInjection();
	~CDllInjection();

private:

	CDllInjection(const CDllInjection& other);
	CDllInjection& operator=(const CDllInjection& other);

	INT CheckIfHookActive(
	);

	// TODO: Here is the place to add checks on white-listed policy or hard-coded white-list.
	BOOL CheckIfSkeepHook(
		ProcessDataElement* pProcListElement
	);

	BOOL CHeckIfHooked(
		ProcessDataElement* pProcListElement
	);

	BOOL CHeckIfIsMainExec(
		ProcessDataElement* pProcListElement
	);

	// This function is added only as a debug aid It usually will return FALSE
	// Unless compiled to inject on test process only
	BOOL CheckIfOnDebuging(
		ProcessDataElement* pProcListElement
	);

	BOOL DoHook(
		PEX_RUNDOWN_REF ExitRunDown,
		volatile LONG* ApcPendingCount,
		PETHREAD Kernel32LoaderThread,
		ProcessDataElement* pProcListElement,
		HookType TypeOfHook,
		PVOID Shel32MapAddress
	);

	static
	LONG volatile m_ApcPendingCount;

	BOOL m_Inited;
	UNICODE_STRING m_Kernel32Path;
	UNICODE_STRING m_Kernel32PathWow;
	UNICODE_STRING m_kernelBaseWowPath;

	WCHAR m_WowDllPAth[MAX_PATH];
	WCHAR m_NormalDllPath[MAX_PATH];
// 	IInjectorMapperLib* m_pInjectorMapper;
	BOOL m_InjectorMapperIsReady;
};

