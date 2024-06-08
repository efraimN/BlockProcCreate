#pragma once

extern BOOLEAN G_ShouldStopWpp;

VOID
__cdecl
StopDebuggerTracer(
	PDRIVER_OBJECT DriverObject
);