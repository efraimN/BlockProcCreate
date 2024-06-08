#pragma once

#include <IUserKernelUtils.h>

class CUtils  : public UserKernelUtilsLib::CUtilsInt
{
public:
	friend CUtilsInt;

	virtual
	BOOLEAN
	MapFileIntoMem(
		_In_ PUNICODE_STRING FullFilePath,
		_Inout_ PVOID* DataMemPtr,
		_Inout_ ULONG* DataLength,
		_In_ BOOL ReadOnly
	);

	virtual
	BOOLEAN
	MapFileIntoMem(
		_In_ HANDLE FileHandle,
		_Inout_ PVOID* DataMemPtr,
		_Inout_ ULONG* DataLength,
		_In_ BOOL ReadOnly
	);

	virtual
	BOOLEAN
	MapFileIntoMemEx(
		_In_ HANDLE FileHandle,
		_Inout_ PVOID* DataMemPtr,
		_Inout_ ULONG* DataLength,
		_In_ BOOL ReadOnly,
		_In_ BOOL Image,
		_Inout_ PVOID* SectionHandle
	);

	virtual
	BOOLEAN
	MapFileIntoMemEx(
		_In_ PUNICODE_STRING FullFilePath,
		_Inout_ PVOID* DataMemPtr,
		_Inout_ ULONG* DataLength,
		_In_ BOOL ReadOnly,
		_In_ BOOL Image,
		_Inout_ PVOID* SectionHandle
	);


private:
	CUtils();
	virtual ~CUtils();

};
