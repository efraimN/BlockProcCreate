#include "Precompiled.h"
#include <WppIncludes.h>

#include <DriverEntryLib.h>

void* __cdecl BasicDriverDevice::operator new(size_t nSize, PDRIVER_OBJECT DriverObject, BOOLEAN Exclusive/* = FALSE*/, wchar_t* pDeviceName/* = NULL*/, DEVICE_TYPE Type/* = FILE_DEVICE_UNKNOWN*/)
{
	auto ProcEntry = [](BOOL start)
	{
		if (start)
			LOG_OUT(DBG_TRACE_FUNCTIONS, ">>> Start file %!FILE! line: %!LINE!");
		else
			LOG_OUT(DBG_TRACE_FUNCTIONS, "<<< Finish file %!FILE! line: %!LINE!");

	};
	PROC_ENTRY;

	void* RetVal = NULL;

	PDEVICE_OBJECT DeviceObject;
	UNICODE_STRING DeviceName;

	NTSTATUS status = STATUS_SUCCESS;
	DeviceName.Buffer = NULL;
	if (pDeviceName)
	{
		DeviceName.Length = (USHORT)wcslen(pDeviceName) * 2;
		DeviceName.MaximumLength = DeviceName.Length + 2;
		DeviceName.Buffer = new('vrdV') wchar_t[DeviceName.MaximumLength / 2];
		if (!DeviceName.Buffer)
		{
			LOG_OUT(DBG_ERROR, "DeviceName.Buffer Failed");
			goto Leave;
		}

		RtlCopyMemory(DeviceName.Buffer, pDeviceName, DeviceName.MaximumLength);

		LOG_OUT(DBG_INFO, "Going to create device named %S", DeviceName.Buffer);
		status = IoCreateDevice(
			DriverObject,
			(ULONG)nSize,
			&DeviceName,
			Type,
			0,
			Exclusive,
			&DeviceObject
		);
	}
	else
	{
		status = IoCreateDevice(
			DriverObject,
			(ULONG)nSize,
			NULL,
			Type,
			0,
			Exclusive,
			&DeviceObject
		);
	}

	if (!NT_SUCCESS(status))
	{
		LOG_OUT(DBG_ERROR, "IoCreateDevice Failed: status=0x%08X", status);
		goto Leave;
	}

	BasicDriverDevice* pMyDevices = (BasicDriverDevice*)DeviceObject->DeviceExtension;
	RtlZeroMemory(pMyDevices, (ULONG)nSize);

	pMyDevices->m_pDeviceObject = DeviceObject;
	RetVal = (void*)pMyDevices;

	Leave:
	if (DeviceName.Buffer)
	{
		delete[] DeviceName.Buffer;
	}
	return RetVal;
}

void __cdecl BasicDriverDevice::operator delete(void* p)
{
	BasicDriverDevice* DeviceExt = (BasicDriverDevice*)p;
	PDEVICE_OBJECT DeviceObject = DeviceExt->m_pDeviceObject;
	{	auto ProcEntry = [](BOOL start)
		{
			if (start)
				LOG_OUT(DBG_TRACE_FUNCTIONS, ">>> Start file %!FILE! line: %!LINE!");
			else
				LOG_OUT(DBG_TRACE_FUNCTIONS, "<<< Finish file %!FILE! line: %!LINE!");

		};
		PROC_ENTRY;

		if (NULL == DeviceExt)
		{
			LOG_OUT(DBG_ERROR, ("Received a NULL ptr"));
			return;
		}

		LOG_OUT(DBG_INFO, "Deleting Device with m_Signature = 0x%08X ReferenceCount %d", DeviceExt->m_Signature, DeviceObject->ReferenceCount);
	}

	IoDeleteDevice(DeviceObject);
}

BasicDriverDevice::BasicDriverDevice(UINT32 Signature)
{
	auto ProcEntry = [](BOOL start)
	{
		if (start)
			LOG_OUT(DBG_TRACE_FUNCTIONS, ">>> Start file %!FILE! line: %!LINE!");
		else
			LOG_OUT(DBG_TRACE_FUNCTIONS, "<<< Finish file %!FILE! line: %!LINE!");

	};
	PROC_ENTRY;
	m_Signature = Signature;
}

BasicDriverDevice::~BasicDriverDevice()
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

