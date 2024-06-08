#pragma once

#include <DriverEntryLib.h>

class CDriverObject
{
public:
	static CDriverObject* GetInstance();

	virtual
	NTSTATUS DriverObjectMain(
		IN PUNICODE_STRING RegistryPath,
		IN CDriverObjectInt *DriverObjectInstance
	);

	virtual
	void DriverExit();

private:

	CDriverObject();
	~CDriverObject();

	static
	NTSTATUS
	MajorFunctionDispatcher__(
		IN PDEVICE_OBJECT DeviceObject,
		IN PIRP Irp);

	HANDLE m_hFile;
	CDriverObjectInt* m_DriverObjectInt;
};
