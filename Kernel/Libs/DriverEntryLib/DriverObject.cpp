#include "Precompiled.h"
#include <WppIncludes.h>

#include "DriverObject.h"

#include <KernelUtilsLib.h>

CDriverObject::CDriverObject()
{
	auto ProcEntry = [](BOOL start)
	{
		if (start)
			LOG_OUT(DBG_TRACE_FUNCTIONS, ">>> Start file %!FILE! line: %!LINE!");
		else
			LOG_OUT(DBG_TRACE_FUNCTIONS, "<<< Finish file %!FILE! line: %!LINE!");

	};
	PROC_ENTRY;

	m_hFile = NULL;
}

CDriverObject::~CDriverObject()
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

CDriverObject* CDriverObject::GetInstance()
{
	static CDriverObject SingleTone;
	return &SingleTone;
}


NTSTATUS
CDriverObject::MajorFunctionDispatcher__(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
	return CDriverObject::GetInstance()->m_DriverObjectInt->MajorFunctionDispatcher(DeviceObject, Irp);
}

NTSTATUS
CDriverObject::DriverObjectMain(
	IN PUNICODE_STRING RegistryPath,
	IN CDriverObjectInt *DriverObjectInstance
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
	NTSTATUS RetVal = STATUS_INSUFFICIENT_RESOURCES;
	
	m_DriverObjectInt = DriverObjectInstance;

	RetVal = KernelUtilsLib::CRegistryUtil::InitDriverRegPath(RegistryPath);
	if (!NT_SUCCESS(RetVal))
	{
		LOG_OUT(DBG_ERROR, "Failed InitDriverRegPath 0x%08X", RetVal);
		goto Leave;
	}
	RetVal = STATUS_INSUFFICIENT_RESOURCES;

	LOG_OUT(DBG_INFO, "Registry path = %S", KernelUtilsLib::CRegistryUtil::m_RegistryPath.Buffer);
	LOG_OUT(DBG_INFO, "CurrentControlSetPath = %S", KernelUtilsLib::CRegistryUtil::m_CurrentControlSetPath.Buffer);
	LOG_OUT(DBG_INFO, "Driver Image path is %S", KernelUtilsLib::CRegistryUtil::m_DriverImageFileName.Buffer);


	for (int i = 0; i <= IRP_MJ_MAXIMUM_FUNCTION; i++)
	{
		DriverEntryLib ::G_DriverObject->MajorFunction[i] = MajorFunctionDispatcher__;
	}

	RetVal = m_DriverObjectInt->DoStartStop(TRUE);

Leave:
	LOG_OUT(DBG_INFO, "Return status =  0x%08X", RetVal);

	return RetVal;
}

void CDriverObject::DriverExit()
{
	auto ProcEntry = [](BOOL start)
	{
		if (start)
			LOG_OUT(DBG_TRACE_FUNCTIONS, ">>> Start file %!FILE! line: %!LINE!");
		else
			LOG_OUT(DBG_TRACE_FUNCTIONS, "<<< Finish file %!FILE! line: %!LINE!");

	};
	PROC_ENTRY;

	LOG_OUT(DBG_INFO, "Starting the de-init %!FUNC!");

	m_DriverObjectInt->DoStartStop(FALSE);

	KernelUtilsLib::CRegistryUtil::ReleaseDriverRegPath();

	if (m_hFile)
	{
		ZwClose(m_hFile);
	}

	return;
}
