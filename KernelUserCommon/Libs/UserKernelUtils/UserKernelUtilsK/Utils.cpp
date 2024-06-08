#include "Precompiled.h"

#include <WppIncludes.h>

#include "Utils.h"

using namespace UserKernelUtilsLib;

CUtilsInt* CUtilsInt::GetInstance()
{
	static CUtils Singletone;
	return &Singletone;
}

CUtils::CUtils()
{
}

CUtils::~CUtils()
{
}

BOOLEAN CUtils::MapFileIntoMem(
	_In_ PUNICODE_STRING FullFilePath,
	_Out_ PVOID* DataMemPtr,
	_Out_ ULONG* DataLength,
	_In_ BOOL ReadOnly
)
{
	return MapFileIntoMemEx(
		FullFilePath,
		DataMemPtr,
		DataLength,
		ReadOnly,
		FALSE,
		NULL
	);
}

BOOLEAN CUtils::MapFileIntoMem(
	_In_ HANDLE FileHandle,
	_Out_ PVOID* DataMemPtr,
	_Out_ ULONG* DataLength,
	_In_ BOOL ReadOnly
)
{
	return MapFileIntoMemEx(
		FileHandle,
		DataMemPtr,
		DataLength,
		ReadOnly,
		FALSE,
		NULL
	);
}

BOOLEAN CUtils::MapFileIntoMemEx(
	_In_ PUNICODE_STRING FullFilePath,
	_Out_ PVOID* DataMemPtr,
	_Out_ ULONG* DataLength,
	_In_ BOOL ReadOnly,
	_In_ BOOL Image,
	_Inout_ PVOID* SectionHandle
)
{
	BOOLEAN RetVal = FALSE;
	HANDLE FileHandle = NULL;

	ACCESS_MASK DesiredAccess = GENERIC_READ;
	ULONG ShareAccess = FILE_SHARE_READ;

	if (!ReadOnly)
	{
		DesiredAccess |= GENERIC_WRITE;
		ShareAccess |= FILE_SHARE_WRITE;
	}

	if (!UserKernelUtilsLib::IFileUtilsInt::GetInstance()->OpenFile(
		FullFilePath,
		&FileHandle,
		DesiredAccess,
		ShareAccess
	))
	{
		goto Leave;
	}

	RetVal = MapFileIntoMemEx
	(
		FileHandle,
		DataMemPtr,
		DataLength,
		ReadOnly,
		Image,
		SectionHandle
	);

Leave:
	if (FileHandle)
	{
		ZwClose(FileHandle);
	}
	return RetVal;
}

BOOLEAN CUtils::MapFileIntoMemEx(
	_In_ HANDLE FileHandle,
	_Out_ PVOID* DataMemPtr,
	_Out_ ULONG* DataLength,
	_In_ BOOL ReadOnly,
	_In_ BOOL Image,
	_Inout_ PVOID* SectionHandle
)
{
	BOOLEAN RetVal = FALSE;
	NTSTATUS status;

	PVOID DataMemPtrp = NULL;
	HANDLE SectHandle = NULL;
	LARGE_INTEGER FileSize;

	DWORD flProtect;
	DWORD DesiredAccess;
	SIZE_T ViewSize = 0;

	flProtect = ReadOnly ? PAGE_READONLY : PAGE_READWRITE;
	DesiredAccess = ReadOnly ? FILE_MAP_READ : FILE_MAP_READ | FILE_MAP_WRITE;

	if (Image)
	{
		flProtect = SEC_IMAGE | PAGE_READONLY;
	}

	if (!UserKernelUtilsLib::IFileUtilsInt::GetInstance()->GetFileSize(
		FileHandle,
		&FileSize))
	{
		goto Leave;
	}

	status = UserKernelUtilsLib::IMapFileInt::GetInstance()->CreateSection(
		&SectHandle,
		FileHandle,
		NULL,
		FALSE,
		flProtect,
		FileSize.HighPart,
		FileSize.LowPart,
		NULL
	);
	if (!NT_SUCCESS(status))
	{
		goto Leave;
	}

	status = UserKernelUtilsLib::IMapFileInt::GetInstance()->MapFile(
		&DataMemPtrp,
		SectHandle,
		ZwCurrentProcess(),
		DesiredAccess,
		0,
		0,
		&ViewSize
	);
	if (!NT_SUCCESS(status))
	{
		goto Leave;
	}

	*DataMemPtr = DataMemPtrp;
// 	*DataLength = FileSize.LowPart;
	*DataLength = (ULONG)ViewSize;

	RetVal = TRUE;
Leave:
	if (SectHandle)
	{
		if (SectionHandle)
		{
			*SectionHandle = SectHandle;
		}
		else
		{
			ZwClose(SectHandle);
		}
	}

	return RetVal;
}
