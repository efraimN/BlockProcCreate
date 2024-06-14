#pragma once
#include <IUserKernelUtils.h>
class Utils  : public UserKernelUtilsLib::IUtilsInt
{
	friend IUtilsInt;
	virtual BOOL IsProcessWow64(
		_In_ HANDLE ProcessHandle,
		_Out_ PBOOL IsWow
	);

	virtual
	LONG
	SafeSearchString(
		IN PUNICODE_STRING Source,
		IN PUNICODE_STRING Target,
		IN BOOLEAN CaseInSensitive
	);

	virtual
		LONG
		StrLenSafe(
			_Out_ const char* str,
			_In_ ULONG MaxLen,
			_In_ HANDLE ProcHandle
		);

private:
	Utils();
	virtual ~Utils();

};
