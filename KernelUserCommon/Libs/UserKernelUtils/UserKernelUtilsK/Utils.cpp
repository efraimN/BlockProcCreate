#include "Precompiled.h"

#include <WppIncludes.h>

#include "Utils.h"

using namespace UserKernelUtilsLib;

IUtilsInt* IUtilsInt::GetInstance()
{
	static Utils Singletone;
	return &Singletone;
}

Utils::Utils()
{
}

Utils::~Utils()
{
}

BOOL
Utils::IsProcessWow64(
	_In_ HANDLE ProcessHandle,
	_Out_ PBOOL IsWow
)
{
	BOOL RetVal = FALSE;
	ULONG Size = 0;
	if (!IsWow)
	{
		goto Leave;
	}

// #ifdef _M_X64
	PVOID Peb32;
	if (!NT_SUCCESS(ZwQueryInformationProcess(ProcessHandle, ProcessWow64Information, &Peb32, sizeof(PVOID), &Size)))
	{
		goto Leave;
	}
	*IsWow = Peb32 ? TRUE : FALSE;
// #else
// 	UNREFERENCED_PARAMETER(ProcessHandle);
// 	UNREFERENCED_PARAMETER(Size);
// 	*IsWow = FALSE;
// #endif

	RetVal = TRUE;
Leave:
	return RetVal;
}

LONG
Utils::SafeSearchString(
	IN PUNICODE_STRING Source,
	IN PUNICODE_STRING Target,
	IN BOOLEAN CaseInSensitive
)
{
	// Size mismatch
	if (Source->Length < Target->Length)
		return -1;

	USHORT diff = Source->Length - Target->Length;
	for (USHORT i = 0; i <= diff; i++)
	{
		if (RtlCompareUnicodeStrings(
			Source->Buffer + i / sizeof(WCHAR),
			Target->Length / sizeof(WCHAR),
			Target->Buffer,
			Target->Length / sizeof(WCHAR),
			CaseInSensitive
		) == 0)
		{
			return i;
		}
	}

	return -1;
}

LONG Utils::StrLenSafe(
	_Out_ const char* str,
	_In_ ULONG MaxLen,
	_In_ HANDLE ProcHandle
)
{
	CHAR Character = 0;
	LONG Counter = 0;
	const CHAR* CharAddress;
	LONG RetVal = -1;

	// Initialize parameters
	CharAddress = str;
	Counter = MaxLen;

	do
	{
		__try
		{
			// Read byte from char string
			ProbeForRead(
				(LPVOID)CharAddress,
				sizeof(CHAR),
				__alignof(CHAR));
			ReadMemory((PVOID)CharAddress, &Character, sizeof(CHAR), ProcHandle);

			// Go to the next character
			CharAddress++;
			Counter--;
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			LOG_OUT(DBG_ERROR, "StrLenSafe catch");
			goto Leave;
		}
	} while (Character && (Counter >= 0));

	RetVal = MaxLen - Counter - 1;

Leave:
	return RetVal;
}

#ifndef _NTDDK_
VOID
NTAPI
ProbeForRead(
	const volatile VOID* Address,
	SIZE_T Length,
	ULONG Alignment
)
{
	UNREFERENCED_PARAMETER(Address);
	UNREFERENCED_PARAMETER(Length);
	UNREFERENCED_PARAMETER(Alignment);
}
#else
extern "C"
BOOL
NTAPI
WriteProcessMemory(
	_In_ HANDLE hProcess,
	_In_ LPVOID lpBaseAddress,
	_In_ LPCVOID lpBuffer,
	_In_ SIZE_T nSize,
	_Out_opt_ SIZE_T * lpNumberOfBytesWritten
)
{
	BOOL RetVal = FALSE;

	BOOLEAN Acquired = FALSE;
	KAPC_STATE apc;
	PKPROCESS pKprocess = NULL;
	NTSTATUS status;
	if (hProcess)
	{
		status = ObReferenceObjectByHandle(hProcess, GENERIC_ALL, NULL, KernelMode, (PVOID*)&pKprocess, NULL);
		if (!NT_SUCCESS(status))
		{
			goto Leave;
		}

		if (PsGetCurrentProcess() != pKprocess)
		{
			if (!NT_SUCCESS(PsAcquireProcessExitSynchronization(pKprocess)))
			{
				goto Leave;
			}
			KeStackAttachProcess(pKprocess, &apc);
			Acquired = TRUE;
		}
	}

	[&]()
	{
		__try
		{
			memcpy(lpBaseAddress, lpBuffer, nSize);
			RetVal = TRUE;
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
		}
	}();

	if (!RetVal)
	{
		goto Leave;
	}

	if (lpNumberOfBytesWritten)
	{
		*lpNumberOfBytesWritten = nSize;
	}

Leave:
	if (Acquired)
	{
		KeUnstackDetachProcess(&apc);
		PsReleaseProcessExitSynchronization(pKprocess);
	}
	if (pKprocess)
	{
		ObDereferenceObject(pKprocess);
	}

	return RetVal;
}

extern "C"
BOOL
NTAPI
ReadProcessMemory(
	_In_ HANDLE hProcess,
	_In_ LPCVOID lpBaseAddress,
	_Out_ LPVOID lpBuffer,
	_In_ SIZE_T nSize,
	_Out_opt_ SIZE_T * lpNumberOfBytesRead
)
{
	BOOL RetVal = FALSE;

	BOOLEAN Acquired = FALSE;
	KAPC_STATE apc;
	PKPROCESS pKprocess = NULL;
	NTSTATUS status;
	if (hProcess)
	{
		status = ObReferenceObjectByHandle(hProcess, GENERIC_ALL, NULL, KernelMode, (PVOID*)&pKprocess, NULL);
		if (!NT_SUCCESS(status))
		{
			goto Leave;
		}

		if (PsGetCurrentProcess() != pKprocess)
		{
			if (!NT_SUCCESS(PsAcquireProcessExitSynchronization(pKprocess)))
			{
				goto Leave;
			}
			KeStackAttachProcess(pKprocess, &apc);
			Acquired = TRUE;
		}
	}
	[&]()
	{
		__try
		{
			memcpy(lpBuffer, lpBaseAddress, nSize);
			RetVal = TRUE;
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
		}
	}();

	if (!RetVal)
	{
		goto Leave;
	}

	if (lpNumberOfBytesRead)
	{
		*lpNumberOfBytesRead = nSize;
	}

Leave:
	if (Acquired)
	{
		KeUnstackDetachProcess(&apc);
		PsReleaseProcessExitSynchronization(pKprocess);
	}
	if (pKprocess)
	{
		ObDereferenceObject(pKprocess);
	}

	return RetVal;
}

#endif
