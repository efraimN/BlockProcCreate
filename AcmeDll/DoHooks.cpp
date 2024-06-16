#include "Precompiled.h"
#include <WppIncludes.h>

#include "DoHooks.h"
#include <MinHook_src\include\MinHook.h>

VOID DoAllHooks();

VOID HooksInit()
{
	MH_Initialize();

	DoAllHooks();

	MH_EnableHook(MH_ALL_HOOKS);
}

VOID HooksStop()
{
	MH_Uninitialize();
}

// better (but requires some research) is to hook NtCreateUserProcess
#define CREATE_FUNCTION "CreateProcessInternalW"
DWORD
WINAPI
MyCreateProcessInternalW(
__in		HANDLE hUserToken,
__in_opt	LPCTSTR lpApplicationName,
__inout_opt	LPTSTR lpCommandLine,
__in_opt	LPSECURITY_ATTRIBUTES lpProcessAttributes,
__in_opt	LPSECURITY_ATTRIBUTES lpThreadAttributes,
__in		BOOL bInheritHandles,
__in		DWORD dwCreationFlags,
__in_opt	LPVOID lpEnvironment,
__in_opt	LPCTSTR lpCurrentDirectory,
__in		LPSTARTUPINFO lpStartupInfo,
__in		LPPROCESS_INFORMATION lpProcessInformation,
__out		PHANDLE hNewToken
);


// TODO do it generic, i.e add a class that can handle an array of hooks
decltype(MyCreateProcessInternalW)* OriginalTargetAddress;

VOID DoAllHooks()
{
	PVOID HookFunctionAdrr;
	HMODULE hModule;

	if (!GetModuleHandleExW(
		GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
		L"KernelBase.dll",
		&hModule
	))
	{
		if (!GetModuleHandleExW(
			GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
			L"kernel32.dll",
			&hModule
		))
		{
			goto Leave;
		}
	}

	if (!hModule)
	{
		goto Leave;
	}

	HookFunctionAdrr = GetProcAddress(hModule, CREATE_FUNCTION);
	if (!HookFunctionAdrr)
	{
		goto Leave;
	}

	MH_DoHook(
		HookFunctionAdrr,
		MyCreateProcessInternalW,
		(PVOID*)&OriginalTargetAddress,
		FALSE
	);


Leave:
	return;
}

DWORD
WINAPI
MyCreateProcessInternalW(
	__in		HANDLE hUserToken,
	__in_opt	LPCTSTR lpApplicationName,
	__inout_opt	LPTSTR lpCommandLine,
	__in_opt	LPSECURITY_ATTRIBUTES lpProcessAttributes,
	__in_opt	LPSECURITY_ATTRIBUTES lpThreadAttributes,
	__in		BOOL bInheritHandles,
	__in		DWORD dwCreationFlags,
	__in_opt	LPVOID lpEnvironment,
	__in_opt	LPCTSTR lpCurrentDirectory,
	__in		LPSTARTUPINFO lpStartupInfo,
	__in		LPPROCESS_INFORMATION lpProcessInformation,
	__out		PHANDLE hNewToken
)
{
	DWORD dwLastError;
	BOOL Except = FALSE;
	ULONG Code = 0;
	DWORD originalReturnValue = 0;

	// HACK TODO use an LPC to tell another proc to dysplay this dlog. IS ABSLOUTELLY not safe to do it from here...

	LOG_OUT(DBG_DEBUG, "Inside MyCreateProcessInternalW lpApplicationName %S", lpApplicationName);
	if (IDOK != MessageBoxExW(
		NULL,
		lpApplicationName,
		lpCommandLine,
		MB_OKCANCEL,
		0
	))
	{
		dwLastError = 51;
		goto Leave;
	}

	__try
	{
		originalReturnValue = OriginalTargetAddress(
			hUserToken,
			lpApplicationName,
			lpCommandLine,
			lpProcessAttributes,
			lpThreadAttributes,
			bInheritHandles,
			dwCreationFlags,
			lpEnvironment,
			lpCurrentDirectory,
			lpStartupInfo,
			lpProcessInformation,
			hNewToken);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		Except = TRUE;
		Code = GetExceptionCode();
	}
	dwLastError = GetLastError();

	if (Except)
	{
		RaiseException(
			Code,
			0,
			0,
			NULL
		);
	}

Leave:
	SetLastError(dwLastError);
	return originalReturnValue;
}
