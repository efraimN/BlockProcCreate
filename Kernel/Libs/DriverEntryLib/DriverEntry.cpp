#include "Precompiled.h"
#include <WppIncludes.h>

#include "DriverObject.h"
#include <DriverEntryLib.h>

static
void DriverExit(IN PDRIVER_OBJECT DriverObject);
static
void DriverExitInternal(IN PDRIVER_OBJECT DriverObject);

PDRIVER_OBJECT DriverEntryLib::G_DriverObject;

static
DriverEntryLib::pfnStopDebuggerTracer s_StopDebuggerTracer;

NTSTATUS
DriverEntryLib::Main(
	IN PDRIVER_OBJECT DriverObject,
	IN PUNICODE_STRING RegistryPath,
	IN PCHAR DriverFileName,
	IN PCHAR DriverFileVersion,
	IN decltype(CDriverObjectInt::GetInstance)* DriverObjectInstance,
	pfnStartDebuggerTracer StartDebuggerTracer,
	pfnStopDebuggerTracer StopDebuggerTracer
)
{
	NTSTATUS status = STATUS_INSUFFICIENT_RESOURCES;
	{
#if 0
		KdRefreshDebuggerNotPresent();
		if (KD_DEBUGGER_ENABLED && !KD_DEBUGGER_NOT_PRESENT)
		{
			[]() {
				// if a kernel debugger is attached the following will break into the debugger.
				__try
				{
					KdPrint((EXE_FILE_NAME)); KdPrint((" : "));  KdPrint((" ")); KdPrint(("This breakpoint is executed because a debugger is attached. you can safelly ignore it\n"));
					KdBreakPoint();
				}
				__except (EXCEPTION_EXECUTE_HANDLER)
				{
				}
			}();
		}
#endif
		G_DriverObject = DriverObject;
		s_StopDebuggerTracer = StopDebuggerTracer;

		DriverObject->DriverUnload = DriverExit;
		ExInitializeDriverRuntime(DrvRtPoolNxOptIn);

		if (StartDebuggerTracer)
		{
			StartDebuggerTracer(DriverObject, RegistryPath);
		}

		auto ProcEntry = [](BOOL start)
		{
			if (start)
				LOG_OUT(DBG_TRACE_FUNCTIONS, ">>> Start file %!FILE! line: %!LINE!");
			else
				LOG_OUT(DBG_TRACE_FUNCTIONS, "<<< Finish file %!FILE! line: %!LINE!");

		};
		PROC_ENTRY;

		LOG_OUT(DBG_INFO, "Driver Name %s Driver Build Date: %s Time: %s Version %s", DriverFileName, __DATE__, __TIME__, DriverFileVersion);

		CrtInit();
		
		status = CDriverObject::GetInstance()->DriverObjectMain(RegistryPath, DriverObjectInstance());
		if (!NT_SUCCESS(status))
		{
			goto Leave;
		}

		status = STATUS_SUCCESS;
	Leave:
		if (!NT_SUCCESS(status))
		{
			DriverExitInternal(DriverObject);
		}
	}
	if (!NT_SUCCESS(status))
	{
		if (s_StopDebuggerTracer)
		{
			s_StopDebuggerTracer(DriverObject);
		}
	}
	return status;
};

static
void DriverExitInternal(IN PDRIVER_OBJECT DriverObject)
{
	auto ProcEntry = [](BOOL start)
	{
		if (start)
			LOG_OUT(DBG_TRACE_FUNCTIONS, ">>> Start file %!FILE! line: %!LINE!");
		else
			LOG_OUT(DBG_TRACE_FUNCTIONS, "<<< Finish file %!FILE! line: %!LINE!");

	};
	PROC_ENTRY;

	UNREFERENCED_PARAMETER(DriverObject);
	LOG_OUT(DBG_INFO, "DriverExit: Going to call CDriverObject::GetInstance()->DriverExit()");

	CDriverObject::GetInstance()->DriverExit();

	CrtExit();
	LOG_OUT(DBG_INFO, "------- Driver Exit Finished ---------");
}

static
void DriverExit(IN PDRIVER_OBJECT DriverObject)
{

	{
		auto ProcEntry = [](BOOL start)
		{
			if (start)
				LOG_OUT(DBG_TRACE_FUNCTIONS, ">>> Start file %!FILE! line: %!LINE!");
			else
				LOG_OUT(DBG_TRACE_FUNCTIONS, "<<< Finish file %!FILE! line: %!LINE!");

		};
		PROC_ENTRY;

		DriverExitInternal(DriverObject);
	}

	if (s_StopDebuggerTracer)
	{
		s_StopDebuggerTracer(DriverObject);
	}
}
