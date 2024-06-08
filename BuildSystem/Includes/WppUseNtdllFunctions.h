#pragma once
#include <evntrace.h>

#ifdef __cplusplus
extern "C" {
#endif

ULONG
__stdcall
EtwTraceMessage(
    _In_ TRACEHANDLE LoggerHandle,
    _In_ ULONG MessageFlags,
    _In_ LPCGUID MessageGuid,
    _In_ USHORT MessageNumber,
    ...
    );

TRACEHANDLE
__stdcall
EtwGetTraceLoggerHandle(
    _In_ PVOID Buffer
    );

UCHAR
__stdcall
EtwGetTraceEnableLevel(
    _In_ TRACEHANDLE LoggerHandle
    );

ULONG
__stdcall
EtwGetTraceEnableFlags(
    _In_ TRACEHANDLE LoggerHandle
    );

ULONG 
__stdcall
EtwRegisterTraceGuidsW(
    _In_ WMIDPREQUEST RequestAddress,
    _In_ PVOID RequestContext,
    _In_ LPCGUID ControlGuid,
    _In_ ULONG GuidCount,
    _In_ PTRACE_GUID_REGISTRATION GuidReg,
    _In_opt_ LPCWSTR MofImagePath,
    _In_opt_ LPCWSTR MofResourceName,
    _Out_ PTRACEHANDLE RegistrationHandle
    );

ULONG
__stdcall
EtwUnregisterTraceGuids(
    _In_ TRACEHANDLE RegistrationHandle
    );

#define WPP_TRACE                   EtwTraceMessage
#define WPP_GET_TRACE_LOGGER_HANDLE EtwGetTraceLoggerHandle
#define WPP_GET_TRACE_ENABLE_LEVEL  EtwGetTraceEnableLevel
#define WPP_GET_TRACE_ENABLE_FLAGS  EtwGetTraceEnableFlags
#define WPP_REGISTER_TRACE_GUIDS    EtwRegisterTraceGuidsW
#define WPP_UNREGISTER_TRACE_GUIDS  EtwUnregisterTraceGuids

#ifdef __cplusplus
}
#endif
