#pragma once

// Each driver must implement this class, 
// A pointer to the GetInstance() function is a required param for the DriverEntryLib::MAin function
class CDriverObjectInt
{
public:

	static
	CDriverObjectInt *GetInstance();

	virtual
	NTSTATUS
	MajorFunctionDispatcher(
		IN PDEVICE_OBJECT DeviceObject,
		IN PIRP Irp
	) = 0;

	virtual
	NTSTATUS
	DoStartStop(
		BOOL Start_Stop
	) = 0;

protected:
	CDriverObjectInt() {};
	virtual ~CDriverObjectInt() {};

};

// Implement a class from BasicDriverDevice if a device is needed
class BasicDriverDevice
{
public:

	static

	void* __cdecl operator new(size_t nSize, PDRIVER_OBJECT DriverObject, BOOLEAN Exclusive = FALSE, wchar_t* pDeviceName = NULL, DEVICE_TYPE Type = FILE_DEVICE_UNKNOWN);
	void __cdecl operator delete(void* p);
	BasicDriverDevice(UINT32 Signature);
	virtual ~BasicDriverDevice();

	virtual
	NTSTATUS
	MajorFunctionDispatcher(
		IN PIRP Irp
	) = 0;

	UINT32 m_Signature;
	PDEVICE_OBJECT m_pDeviceObject;      // Ptr to the device object

protected:
};


// This is the driver entry code
namespace DriverEntryLib
{
	// To be implemented in the driver code. will be called by the lib
	typedef
	VOID
	(__cdecl* pfnStartDebuggerTracer)(
		PDRIVER_OBJECT DriverObject,
		PUNICODE_STRING RegistryPath
	);

	// To be implemented in the driver code. will be called by the lib
	typedef
	VOID
	(__cdecl* pfnStopDebuggerTracer)(
		PDRIVER_OBJECT DriverObject
	);

	// To be called from the driver code. It is implemented on the lib
	NTSTATUS
	Main(
		IN PDRIVER_OBJECT DriverObject,
		IN PUNICODE_STRING RegistryPath,
		IN PCHAR DriverFileName,
		IN PCHAR DriverFileVersion,
		IN decltype(CDriverObjectInt::GetInstance)* DriverObjectInstance,
		// a pointer to the driver implemented CDriverObjectInt::GetInstance function
		pfnStartDebuggerTracer StartDebuggerTracer,
		pfnStopDebuggerTracer StopDebuggerTracer
	);

	extern PDRIVER_OBJECT G_DriverObject;
};
