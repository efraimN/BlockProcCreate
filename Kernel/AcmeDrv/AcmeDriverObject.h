#pragma once

#include <DriverEntryLib.h>

class CMyDriverObject : public CDriverObjectInt
{
public:
	friend CDriverObjectInt;
	virtual
	NTSTATUS
	MajorFunctionDispatcher(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);

	virtual
	NTSTATUS DoStartStop(BOOL Start_Stop);

	PDRIVER_OBJECT m_ApdStubDriverObject;

protected:
private:
	CMyDriverObject() { m_ApdStubDriverObject = NULL; };
	virtual ~CMyDriverObject() {};

};
