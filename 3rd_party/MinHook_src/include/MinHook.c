#include <windows.h>

#include "../include/MinHook.h"

BOOL WINAPI MH_DoHook(
	LPVOID FunctionToHookAddress,
	LPVOID Hookfunc,
	LPVOID* OriginalFunction,
	BOOL Enable
)
{
	BOOL RetVal = FALSE;
	MH_STATUS status;


	status = MH_CreateHook(
		FunctionToHookAddress,
		Hookfunc,
		OriginalFunction
	);
	if (MH_OK != status)
	{
		goto Leave;
	}

	if (Enable)
	{
		status = MH_EnableHook(FunctionToHookAddress);
		if (MH_OK != status)
		{
			status = MH_DisableHook(FunctionToHookAddress);
			goto Leave;
		}
	}

	RetVal = TRUE;
Leave:
	return RetVal;
}

BOOL WINAPI MH_StopHook(
	LPVOID FunctionToHookAddress
)
{
	BOOL RetVal = FALSE;
	MH_STATUS status;

// 	status = MH_DisableHook(FunctionToHookAddress);
// 	if (MH_OK != status)
// 	{
// 		goto Leave;
// 	}

	status = MH_RemoveHook(FunctionToHookAddress);
	if (MH_OK != status)
	{
		goto Leave;
	}

	RetVal = TRUE;
Leave:
	return RetVal;
}