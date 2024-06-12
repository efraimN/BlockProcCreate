#include "Precompiled.h"

#include <WppIncludes.h>

#include "ImageLoadingCallBack.h"


void
CImageLoadingCallBack::InitCallBacks()
{
	static
	ImageLoadingCallBackClient* ImageLoadingCallBackClientArray[] =
	{
		NULL,
	};

	m_ImageLoadingCallBackClientArray = ImageLoadingCallBackClientArray;
}

CImageLoadingCallBack::CImageLoadingCallBack()
{
	auto ProcEntry = [](BOOL start)
	{
		if (start)
			LOG_OUT(DBG_TRACE_FUNCTIONS, ">>> Start file %!FILE! line: %!LINE!");
		else
			LOG_OUT(DBG_TRACE_FUNCTIONS, "<<< Finish file %!FILE! line: %!LINE!");

	};
	PROC_ENTRY;
	m_Inited = FALSE;
}

CImageLoadingCallBack::~CImageLoadingCallBack()
{
	auto ProcEntry = [](BOOL start)
	{
		if (start)
			LOG_OUT(DBG_TRACE_FUNCTIONS, ">>> Start file %!FILE! line: %!LINE!");
		else
			LOG_OUT(DBG_TRACE_FUNCTIONS, "<<< Finish file %!FILE! line: %!LINE!");

	};
	PROC_ENTRY;
}

CImageLoadingCallBack* CImageLoadingCallBack::GetInstance()
{
	static CImageLoadingCallBack s_pLoadCallback;
	return &s_pLoadCallback;
}

NTSTATUS  CImageLoadingCallBack::Start()
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

	InitCallBacks();

	for (UINT i=0;;i++)
	{
		if (!m_ImageLoadingCallBackClientArray[i])
		{
			break;
		}
		status = m_ImageLoadingCallBackClientArray[i]->Start();
		if (!NT_SUCCESS(status))
		{
			goto Leave;
		}
	}

	status = PsSetLoadImageNotifyRoutine(
		PloadImageNotifyRoutine_
	);

	if (!NT_SUCCESS(status))
	{
		goto Leave;
	}

	m_Inited = TRUE;

Leave:
	if (!NT_SUCCESS(status))
	{
		Stop();
	}
	return status;

}

NTSTATUS  CImageLoadingCallBack::Stop()
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
		status = PsRemoveLoadImageNotifyRoutine(
			PloadImageNotifyRoutine_
		);
		m_Inited = FALSE;
	}

	for (UINT i = 0;; i++)
	{
		if (!m_ImageLoadingCallBackClientArray[i])
		{
			break;
		}
		m_ImageLoadingCallBackClientArray[i]->Stop();
	}

	return status;
}

void CImageLoadingCallBack::PloadImageNotifyRoutine_(
	PUNICODE_STRING FullImageName,
	HANDLE ProcessId,
	PIMAGE_INFO ImageInfo
)
{
	// FullImageName may be null
	UNREFERENCED_PARAMETER(FullImageName);
	CImageLoadingCallBack* pLoadCallback;
	pLoadCallback = CImageLoadingCallBack::GetInstance();

	// MSDN warranties that this call will wait till all the callbacks are finished
	for (UINT i = 0;; i++)
	{
		if (!pLoadCallback->m_ImageLoadingCallBackClientArray[i])
		{
			break;
		}
		pLoadCallback->m_ImageLoadingCallBackClientArray[i]->ImageLoadingCallBack(
			FullImageName,
			ProcessId,
			ImageInfo
		);
	}
}

