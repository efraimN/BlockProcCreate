#pragma once
#include <IUserKernelUtils.h>
class Utils  : public UserKernelUtilsLib::IUtilsInt
{
	friend IUtilsInt;
	virtual BOOL IsProcessWow64(
		_In_ HANDLE ProcessHandle,
		_Out_ PBOOL IsWow
	);

private:
	Utils();
	virtual ~Utils();

};
