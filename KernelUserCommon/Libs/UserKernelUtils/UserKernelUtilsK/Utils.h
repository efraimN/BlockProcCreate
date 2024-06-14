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

private:
	Utils();
	virtual ~Utils();

};
