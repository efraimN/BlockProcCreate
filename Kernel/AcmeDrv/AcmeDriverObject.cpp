#include "Precompiled.h"
#include <WppIncludes.h>

#include <DriverEntryLib.h>
#include "AcmeDriverObject.h"

#include ".\ProcessCreation\ProcessList.h"
#include ".\ProcessCreation\ProcessCreationCallBack.h"
#include ".\ImageLoading\ImageLoadingCallBack.h"


CDriverObjectInt* CDriverObjectInt::GetInstance()
{
	static CMyDriverObject SingleTone;
	return &SingleTone;
}


NTSTATUS CMyDriverObject::DoStartStop(BOOL Start_Stop)
{
	NTSTATUS status = STATUS_INSUFFICIENT_RESOURCES;

	auto Start = [&]()->NTSTATUS
	{
		/* Add initialization for each module used */
		if (!CProcessList::GetInstance()->Start())
		{
			LOG_OUT(DBG_ERROR, "CProcessList::Start Failed");
			goto Leave;
		}

		status = CProcCreateCallback::GetInstance()->Start();
		if (!NT_SUCCESS(status))
		{
			LOG_OUT(DBG_ERROR, "CProcCreateCallback::Start Failed status =  0x%08X", status);
			goto Leave;
		}

		status = CImageLoadingCallBack::GetInstance()->Start();
		if (!NT_SUCCESS(status))
		{
			LOG_OUT(DBG_INFO, "CImageLoadingCallBack::Start Failed status =  0x%08X", status);
			goto Leave;
		}

		status = STATUS_SUCCESS;
		goto Leave;
	Leave:
		return status;
	};
	auto Stop = [&]()
	{
		//Here do de-init of modules (usually in inverse init order)
		CImageLoadingCallBack::GetInstance()->Stop();
		CProcCreateCallback::GetInstance()->Stop();
		CProcessList::GetInstance()->Stop();

	};

	if (Start_Stop)
	{
		return Start();
	}
	else
	{
		Stop();
		return STATUS_SUCCESS;
	}
	 
}

NTSTATUS
CMyDriverObject::MajorFunctionDispatcher(
	IN PDEVICE_OBJECT DeviceObject,
	IN PIRP Irp)
{
	auto ProcEntry = [](BOOL start)
	{
		if (start)
			LOG_OUT(DBG_TRACE_FUNCTIONS, ">>> Start file %!FILE! line: %!LINE!");
		else
			LOG_OUT(DBG_TRACE_FUNCTIONS, "<<< Finish file %!FILE! line: %!LINE!");

	};
	PROC_ENTRY;

	// We have no devices.... Our "devices" are on the ApdVdr driver object....
	UNREFERENCED_PARAMETER(DeviceObject);

	LOG_OUT(DBG_ERROR, ("Returning STATUS_INVALID_DEVICE_REQUEST the DeviceObject is invalid"));
	Irp->IoStatus.Status = STATUS_INVALID_DEVICE_REQUEST;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return STATUS_INVALID_DEVICE_REQUEST;
}

