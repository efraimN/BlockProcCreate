#ifndef _NTDDK_

NTSTATUS
NTAPI
NTZW(Close)(
	_In_ HANDLE Handle
	);

NTSTATUS
NTAPI
NTZW(CreateFile)(
	_Out_ PHANDLE FileHandle,
	_In_ ACCESS_MASK DesiredAccess,
	_In_ POBJECT_ATTRIBUTES ObjectAttributes,
	_Out_ PIO_STATUS_BLOCK IoStatusBlock,
	_In_opt_ PLARGE_INTEGER AllocationSize,
	_In_ ULONG FileAttributes,
	_In_ ULONG ShareAccess,
	_In_ ULONG CreateDisposition,
	_In_ ULONG CreateOptions,
	_In_reads_bytes_opt_(EaLength) PVOID EaBuffer,
	_In_ ULONG EaLength
);

NTSTATUS
NTAPI
NTZW(WriteFile)(
	_In_ HANDLE FileHandle,
	_In_opt_ HANDLE Event,
	_In_opt_ PIO_APC_ROUTINE ApcRoutine,
	_In_opt_ PVOID ApcContext,
	_Out_ PIO_STATUS_BLOCK IoStatusBlock,
	_In_reads_bytes_(Length) PVOID Buffer,
	_In_ ULONG Length,
	_In_opt_ PLARGE_INTEGER ByteOffset,
	_In_opt_ PULONG Key
	);

NTSTATUS
NTAPI
NTZW(WaitForSingleObject)(
	_In_ HANDLE Handle,
	_In_ BOOLEAN Alertable,
	_In_opt_ PLARGE_INTEGER Timeout
);

NTSTATUS
NTAPI
NTZW(QueryInformationFile)(
	_In_ HANDLE FileHandle,
	_Out_ PIO_STATUS_BLOCK IoStatusBlock,
	_Out_writes_bytes_(Length) PVOID FileInformation,
	_In_ ULONG Length,
	_In_ NTFILE_INFORMATION_CLASS FileInformationClass
	);

NTSTATUS
NTAPI
NTZW(SetInformationFile)(
	_In_ HANDLE FileHandle,
	_Out_ PIO_STATUS_BLOCK IoStatusBlock,
	_Out_writes_bytes_(Length) PVOID FileInformation,
	_In_ ULONG Length,
	_In_ NTFILE_INFORMATION_CLASS FileInformationClass
	);

NTSTATUS
NTAPI
NTZW(OpenFile)(
	_Out_ PHANDLE FileHandle,
	_In_ ACCESS_MASK DesiredAccess,
	_In_ POBJECT_ATTRIBUTES ObjectAttributes,
	_Out_ PIO_STATUS_BLOCK IoStatusBlock,
	_In_ ULONG ShareAccess,
	_In_ ULONG OpenOptions
	);

NTSTATUS
NTAPI
NTZW(UnmapViewOfSection)(
	_In_ HANDLE ProcessHandle,
	_In_opt_ PVOID BaseAddress
	);

NTSTATUS
NTAPI
NTZW(CreateSection)(
	_Out_ PHANDLE SectionHandle,
	_In_ ACCESS_MASK DesiredAccess,
	_In_opt_ POBJECT_ATTRIBUTES ObjectAttributes,
	_In_opt_ PLARGE_INTEGER MaximumSize,
	_In_ ULONG SectionPageProtection,
	_In_ ULONG AllocationAttributes,
	_In_opt_ HANDLE FileHandle
	);

NTSTATUS
NTAPI
NTZW(MapViewOfSection)(
	_In_ HANDLE SectionHandle,
	_In_ HANDLE ProcessHandle,
	_Outptr_result_bytebuffer_(*ViewSize) PVOID* BaseAddress,
	_In_ ULONG_PTR ZeroBits,
	_In_ SIZE_T CommitSize,
	_Inout_opt_ PLARGE_INTEGER SectionOffset,
	_Inout_ PSIZE_T ViewSize,
	_In_ SECTION_INHERIT InheritDisposition,
	_In_ ULONG AllocationType,
	_In_ ULONG Win32Protect
	);

NTSTATUS
NTAPI
NTZW(EnumerateKey)(
	_In_ HANDLE KeyHandle,
	_In_ ULONG Index,
	_In_ KEY_INFORMATION_CLASS KeyInformationClass,
	_Out_writes_bytes_opt_(Length) PVOID KeyInformation,
	_In_ ULONG Length,
	_Out_ PULONG ResultLength
	);

NTSTATUS
NTAPI
NTZW(OpenKey)(
	_Out_ PHANDLE KeyHandle,
	_In_ ACCESS_MASK DesiredAccess,
	_In_ POBJECT_ATTRIBUTES ObjectAttributes
	);

NTSTATUS
NTAPI
NTZW(DeleteKey)(
	_In_ HANDLE KeyHandle
	);

NTSTATUS
NTAPI
NTZW(CreateKey)(
	_Out_ PHANDLE KeyHandle,
	_In_ ACCESS_MASK DesiredAccess,
	_In_ POBJECT_ATTRIBUTES ObjectAttributes,
	_Reserved_ ULONG TitleIndex,
	_In_opt_ PUNICODE_STRING Class,
	_In_ ULONG CreateOptions,
	_Out_opt_ PULONG Disposition
	);

#endif

NTSTATUS
NTAPI
NTZW(QueryInformationProcess)(
	__in       HANDLE ProcessHandle,
	__in       PROCESSINFOCLASS ProcessInformationClass,
	__out      PVOID ProcessInformation,
	__in       ULONG ProcessInformationLength,
	__out_opt  PULONG ReturnLength
);

