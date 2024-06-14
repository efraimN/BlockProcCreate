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
