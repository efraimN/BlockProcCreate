#include "Precompiled.h"

#include <WppIncludes.h>

#include "FileUtils.h"
using namespace UserKernelUtilsLib;

CFileUtils::CFileUtils()
{
}

CFileUtils::~CFileUtils()
{
}

IFileUtilsInt *IFileUtilsInt::GetInstance()
{
	static CFileUtils Singletone;
	return &Singletone;
}

BOOLEAN CFileUtils::OpenCreateFileCommon(
	PUNICODE_STRING FileName,
	PHANDLE hFile,
	ACCESS_MASK DesiredAccess,
	ULONG ShareAccess,
	PLARGE_INTEGER AllocationSize
)
{
// 	auto ProcEntry = [](BOOL start)
// 	{
// 		if (start)
// 			LOG_OUT(DBG_TRACE_FUNCTIONS, ">>> Start file %!FILE! line: %!LINE!");
// 		else
// 			LOG_OUT(DBG_TRACE_FUNCTIONS, "<<< Finish file %!FILE! line: %!LINE!");
// 
// 	};
// 	PROC_ENTRY;

	NTSTATUS status = STATUS_UNSUCCESSFUL;
	OBJECT_ATTRIBUTES ObAttr;
	BOOLEAN RetVal = FALSE;
	DWORD OA_Flags;

	OA_Flags = OBJ_CASE_INSENSITIVE
#ifdef _NTDDK_
		| OBJ_KERNEL_HANDLE
#endif
		;

	InitializeObjectAttributes(
		&ObAttr,
		FileName,
		OA_Flags
		,
		NULL,
		NULL
	);

	IO_STATUS_BLOCK iosb = { 0 };

	if (!AllocationSize)
	{
		status = ZwOpenFile(
			hFile,
			DesiredAccess,
			&ObAttr,
			&iosb,
			ShareAccess,
			0
#ifdef _NTDDK_
			| FILE_SYNCHRONOUS_IO_NONALERT
#endif
		);
	}
	else
	{
		status = ZwCreateFile(
			hFile,
			GENERIC_READ | GENERIC_WRITE,
			&ObAttr,
			&iosb,
			AllocationSize,
			0,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			FILE_SUPERSEDE,
			0
#ifdef _NTDDK_
			| FILE_SYNCHRONOUS_IO_NONALERT
#endif
			,
			NULL,
			0
		);
	}

	if (!NT_SUCCESS(status))
	{
		LOG_OUT(DBG_ERROR, "ZwOpenFile Failed for file %S filename length %d status 0x%08X", FileName->Buffer, (UINT)FileName->Length, status);
		goto Leave;
	}

	RetVal = TRUE;
Leave:
	return RetVal;
}

BOOLEAN CFileUtils::OpenFile(
	PUNICODE_STRING FileName,
	PHANDLE hFile,
	ACCESS_MASK DesiredAccess,
	ULONG ShareAccess
)
{
	return OpenCreateFileCommon(
		FileName,
		hFile,
		DesiredAccess,
		ShareAccess,
		NULL
	);
}

BOOLEAN CFileUtils::CreateFile(
	PUNICODE_STRING FileName,
	PHANDLE hFile,
	ACCESS_MASK DesiredAccess,
	ULONG ShareAccess,
	LARGE_INTEGER AllocationSize
)
{
	return OpenCreateFileCommon(
		FileName,
		hFile,
		DesiredAccess,
		ShareAccess,
		&AllocationSize
	);
}

NTSTATUS CFileUtils::CreateFile_Ex(
	PUNICODE_STRING FileName,
	ACCESS_MASK DesiredAccess,
	ULONG ShareMode,
	LPSECURITY_ATTRIBUTES SecurityAttributes,
	DWORD CreationDisposition,
	DWORD  FlagsAndAttributes,
	PHANDLE hFile
)
{
    OBJECT_ATTRIBUTES ObjectAttributes;
    IO_STATUS_BLOCK IoStatusBlock;
    HANDLE FileHandle = INVALID_HANDLE_VALUE;
    NTSTATUS status = STATUS_UNSUCCESSFUL;
    ULONG FileAttributes, Flags = 0;

    switch (CreationDisposition)
    {
        case CREATE_NEW:
            CreationDisposition = FILE_CREATE;
        break;

        case CREATE_ALWAYS:
            CreationDisposition = FILE_OVERWRITE_IF;
        break;

        case OPEN_EXISTING:
            CreationDisposition = FILE_OPEN;
        break;

        case OPEN_ALWAYS:
            CreationDisposition = FILE_OPEN_IF;
        break;

        case TRUNCATE_EXISTING:
            CreationDisposition = FILE_OVERWRITE;
        break;

        default:
		{
			status = STATUS_INVALID_PARAMETER_5;
			goto Leave;
		}
    }

    if (!(FlagsAndAttributes & FILE_FLAG_OVERLAPPED))
    {
       Flags |= FILE_SYNCHRONOUS_IO_NONALERT;
    }

    if (FlagsAndAttributes & FILE_FLAG_WRITE_THROUGH)
        Flags |= FILE_WRITE_THROUGH;

    if (FlagsAndAttributes & FILE_FLAG_NO_BUFFERING)
        Flags |= FILE_NO_INTERMEDIATE_BUFFERING;

    if (FlagsAndAttributes & FILE_FLAG_RANDOM_ACCESS)
        Flags |= FILE_RANDOM_ACCESS;

    if (FlagsAndAttributes & FILE_FLAG_SEQUENTIAL_SCAN)
        Flags |= FILE_SEQUENTIAL_ONLY;

    if (FlagsAndAttributes & FILE_FLAG_DELETE_ON_CLOSE)
    {
        Flags |= FILE_DELETE_ON_CLOSE;
        DesiredAccess |= DELETE;
    }

    if (FlagsAndAttributes & FILE_FLAG_BACKUP_SEMANTICS)
    {
        if (DesiredAccess & GENERIC_ALL)
        {
            Flags |= FILE_OPEN_FOR_BACKUP_INTENT | FILE_OPEN_REMOTE_INSTANCE;
        }
        else
        {
            if (DesiredAccess & GENERIC_READ)
                Flags |= FILE_OPEN_FOR_BACKUP_INTENT;

            if (DesiredAccess & GENERIC_WRITE)
                Flags |= FILE_OPEN_REMOTE_INSTANCE;
        }
    }
    else
    {
        Flags |= FILE_NON_DIRECTORY_FILE;
    }

    if (FlagsAndAttributes & FILE_FLAG_OPEN_REPARSE_POINT)
        Flags |= FILE_OPEN_REPARSE_POINT;

    if (FlagsAndAttributes & FILE_FLAG_OPEN_NO_RECALL)
        Flags |= FILE_OPEN_NO_RECALL;

    FileAttributes = (FlagsAndAttributes & (FILE_ATTRIBUTE_VALID_FLAGS & ~FILE_ATTRIBUTE_DIRECTORY));

    DesiredAccess |= SYNCHRONIZE | FILE_READ_ATTRIBUTES;

    /* build the object attributes */
    InitializeObjectAttributes(
        &ObjectAttributes,
        FileName,
        0,
        NULL,
        NULL);

    if (SecurityAttributes)
    {
        if (SecurityAttributes->bInheritHandle)
            ObjectAttributes.Attributes |= OBJ_INHERIT;

        ObjectAttributes.SecurityDescriptor = SecurityAttributes->lpSecurityDescriptor;
    }

    if (!(FlagsAndAttributes & FILE_FLAG_POSIX_SEMANTICS))
        ObjectAttributes.Attributes |= OBJ_CASE_INSENSITIVE;

    /* perform the call */
    status = ZwCreateFile(
		&FileHandle,
        DesiredAccess,
        &ObjectAttributes,
        &IoStatusBlock,
        NULL,
        FileAttributes,
        ShareMode,
        CreationDisposition,
        Flags,
        NULL,
        0);

    if (!NT_SUCCESS(status))
    {
        goto Leave;
    }

	*hFile = FileHandle;
Leave:
    return status;
}

NTSTATUS CFileUtils::GetOverlappedResult_Ex(
	HANDLE hFile,
	LPOVERLAPPED Overlapped,
	LPDWORD NumberOfBytesTransferred,
	BOOL Wait
)
{
	NTSTATUS RetVal = STATUS_UNSUCCESSFUL;
	HANDLE hObject;

	/* Check for pending operation */
	if (Overlapped->Internal == STATUS_PENDING)
	{
		/* Check if the caller is okay with waiting */
		if (Wait)
		{
// 			/* Wait for the result */
			hObject = Overlapped->hEvent ? Overlapped->hEvent : hFile;
			RetVal = ZwWaitForSingleObject(hObject, FALSE, NULL);
		}

		if (!NT_SUCCESS(RetVal))
		{
			goto Leave;;
		}
	}

	/* Return bytes transferred */
	*NumberOfBytesTransferred = (DWORD)Overlapped->InternalHigh;

	/* Check for failure during I/O */
	if (!NT_SUCCESS(Overlapped->Internal))
	{
		RetVal = (NTSTATUS)Overlapped->Internal;
		goto Leave;;
	}

	RetVal = STATUS_SUCCESS;
Leave:
	return RetVal;
}

NTSTATUS CFileUtils::WriteFile_Ex(
	HANDLE hFile,
	LPCVOID lpBuffer,
	DWORD nNumberOfBytesToWrite,
	LPDWORD lpNumberOfBytesWritten,
	LPOVERLAPPED lpOverlapped
)
{
    NTSTATUS status = STATUS_UNSUCCESSFUL;

    if (lpNumberOfBytesWritten != NULL) *lpNumberOfBytesWritten = 0;

    if (lpOverlapped != NULL)
    {
        LARGE_INTEGER Offset;
        PVOID ApcContext;

        Offset.u.LowPart = lpOverlapped->Offset;
		Offset.u.HighPart = lpOverlapped->Offset;
        lpOverlapped->Internal = STATUS_PENDING;
        ApcContext = (((ULONG_PTR)lpOverlapped->hEvent & 0x1) ? NULL : lpOverlapped);

        status = ZwWriteFile(hFile,
            lpOverlapped->hEvent,
            NULL,
            ApcContext,
            (PIO_STATUS_BLOCK)lpOverlapped,
            (PVOID)lpBuffer,
            nNumberOfBytesToWrite,
            &Offset,
            NULL);

        if (!NT_SUCCESS(status) || status == STATUS_PENDING)
        {
			goto Leave;
        }

        if (lpNumberOfBytesWritten != NULL)
            *lpNumberOfBytesWritten = (DWORD)lpOverlapped->InternalHigh;
    }
    else
    {
        IO_STATUS_BLOCK Iosb;

        status = ZwWriteFile(hFile,
            NULL,
            NULL,
            NULL,
            &Iosb,
            (PVOID)lpBuffer,
            nNumberOfBytesToWrite,
            NULL,
            NULL);

        /* Wait in case operation is pending */
        if (status == STATUS_PENDING)
        {
            status = ZwWaitForSingleObject(hFile, FALSE, NULL);
            if (NT_SUCCESS(status)) status = Iosb.Status;
        }

        if (NT_SUCCESS(status))
        {
			if (lpNumberOfBytesWritten)
			{
				*lpNumberOfBytesWritten = (DWORD)Iosb.Information;
			}
        }
    }

Leave:
    return status;
}

BOOLEAN
CFileUtils::GetFileSize(
	HANDLE hFile,
	PLARGE_INTEGER FileSize
)
{
	NTSTATUS status;
	FILE_STANDARD_INFORMATION FileStandard;
	IO_STATUS_BLOCK IoStatusBlock = {0};

	if (!FileSize)
	{
		return FALSE;
	}

	status = ZwQueryInformationFile(hFile,
		&IoStatusBlock,
		&FileStandard,
		sizeof(FILE_STANDARD_INFORMATION),
		FileStandardInformation);
	if (!NT_SUCCESS(status))
	{
		return FALSE;
	}

	*FileSize = FileStandard.EndOfFile;

	return TRUE;
}

BOOLEAN
CFileUtils::SetFileSize(
	HANDLE hFile,
	LARGE_INTEGER FileSize
)
{
	NTSTATUS status;
	FILE_END_OF_FILE_INFORMATION FileEndOfFile;
	IO_STATUS_BLOCK IoStatusBlock = {0};


	FileEndOfFile.EndOfFile = FileSize;

	status = ZwSetInformationFile(
		hFile,
		&IoStatusBlock,
		&FileEndOfFile,
		sizeof(FILE_END_OF_FILE_INFORMATION),
		FileEndOfFileInformation);
	if (!NT_SUCCESS(status))
	{
		return FALSE;
	}

	return TRUE;
}
