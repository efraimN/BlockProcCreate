#pragma once

#include <IUserKernelUtils.h>

class CFileUtils : public UserKernelUtilsLib::IFileUtilsInt
{
public:
	friend IFileUtilsInt;

	virtual
	BOOLEAN OpenFile(
		PUNICODE_STRING FileName,
		PHANDLE hFile,
		ACCESS_MASK DesiredAccess,
		ULONG ShareAccess
	);

	virtual
	BOOLEAN CreateFile(
		PUNICODE_STRING FileName,
		PHANDLE hFile,
		ACCESS_MASK DesiredAccess,
		ULONG ShareAccess,
		LARGE_INTEGER AllocationSize
	);

	virtual
	NTSTATUS CreateFile_Ex(
		PUNICODE_STRING FileName,
		ACCESS_MASK DesiredAccess,
		ULONG ShareMode,
		LPSECURITY_ATTRIBUTES SecurityAttributes,
		DWORD CreationDisposition,
		DWORD  FlagsAndAttributes,
		PHANDLE hFile
	);

	virtual
		NTSTATUS GetOverlappedResult_Ex(
		HANDLE hFile,
		LPOVERLAPPED Overlapped,
		LPDWORD NumberOfBytesTransferred,
		BOOL Wait
	);

	virtual
		NTSTATUS WriteFile_Ex(
		HANDLE hFile,
		LPCVOID lpBuffer,
		DWORD nNumberOfBytesToWrite,
		LPDWORD lpNumberOfBytesWritten,
		LPOVERLAPPED lpOverlapped
	);

	virtual
	BOOLEAN GetFileSize(
		HANDLE hFile,
		PLARGE_INTEGER FileSize
	);

	virtual
	BOOLEAN SetFileSize(
		HANDLE hFile,
		LARGE_INTEGER FileSize
	);

private:
	CFileUtils();
	virtual ~CFileUtils();

	BOOLEAN OpenCreateFileCommon(
		PUNICODE_STRING FileName,
		PHANDLE hFile,
		ACCESS_MASK DesiredAccess,
		ULONG ShareAccess,
		PLARGE_INTEGER AllocationSize
	);

};

