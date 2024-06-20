#include "Precompiled.h"
#include <WppIncludes.h>

#include <DriverEntryLib.h>

#include "AcmeDriverObject.h"

BOOLEAN G_ShouldStopWpp = TRUE;

static
VOID
__cdecl
StartDebuggerTracer(
	PDRIVER_OBJECT DriverObject,
	PUNICODE_STRING RegistryPath
)
{
	WPP_INIT_TRACING(DriverObject, RegistryPath);
}

VOID
__cdecl
StopDebuggerTracer(
	PDRIVER_OBJECT DriverObject
)
{
	if (G_ShouldStopWpp)
	{
		WPP_CLEANUP(DriverObject);
	}
}

extern "C"
NTSTATUS
DriverEntry(
	IN PDRIVER_OBJECT DriverObject,
	IN PUNICODE_STRING RegistryPath
)
{
	return DriverEntryLib::Main(
		DriverObject, 
		RegistryPath, 
		ORIGINALFILENAME,
		VER_FILEVERSION_STR,
		CMyDriverObject::GetInstance,
		StartDebuggerTracer,
		StopDebuggerTracer
	);
}

