#ifdef __cplusplus
extern "C"
{
#endif

#ifndef _NTDDK_

	NTSTATUS
	NTAPI
	RtlWriteRegistryValue(
		_In_ ULONG RelativeTo,
		_In_ PCWSTR Path,
		_In_z_ PCWSTR ValueName,
		_In_ ULONG ValueType,
		_In_reads_bytes_opt_(ValueLength) PVOID ValueData,
		_In_ ULONG ValueLength
	);


	typedef
	_Function_class_(RTL_QUERY_REGISTRY_ROUTINE)
	NTSTATUS
	NTAPI
	RTL_QUERY_REGISTRY_ROUTINE(
		_In_z_ PWSTR ValueName,
		_In_ ULONG ValueType,
		_In_reads_bytes_opt_(ValueLength) PVOID ValueData,
		_In_ ULONG ValueLength,
		_In_opt_ PVOID Context,
		_In_opt_ PVOID EntryContext
	);
	typedef RTL_QUERY_REGISTRY_ROUTINE* PRTL_QUERY_REGISTRY_ROUTINE;

	typedef struct _RTL_QUERY_REGISTRY_TABLE {
		PRTL_QUERY_REGISTRY_ROUTINE QueryRoutine;
		ULONG Flags;
		PWSTR Name;
		PVOID EntryContext;
		ULONG DefaultType;
		PVOID DefaultData;
		ULONG DefaultLength;
	} RTL_QUERY_REGISTRY_TABLE, * PRTL_QUERY_REGISTRY_TABLE;

	//
	// The following flags specify how the Name field of a RTL_QUERY_REGISTRY_TABLE
	// entry is interpreted.  A NULL name indicates the end of the table.
	//

#define RTL_QUERY_REGISTRY_SUBKEY   0x00000001  // Name is a subkey and remainder of
// table or until next subkey are value
// names for that subkey to look at.

#define RTL_QUERY_REGISTRY_TOPKEY   0x00000002  // Reset current key to original key for
// this and all following table entries.

#define RTL_QUERY_REGISTRY_REQUIRED 0x00000004  // Fail if no match found for this table
// entry.

#define RTL_QUERY_REGISTRY_NOVALUE  0x00000008  // Used to mark a table entry that has no
// value name, just wants a call out, not
// an enumeration of all values.

#define RTL_QUERY_REGISTRY_NOEXPAND 0x00000010  // Used to suppress the expansion of
// REG_MULTI_SZ into multiple callouts or
// to prevent the expansion of environment
// variable values in REG_EXPAND_SZ

#define RTL_QUERY_REGISTRY_DIRECT   0x00000020  // QueryRoutine field ignored.  EntryContext
// field points to location to store value.
// For null terminated strings, EntryContext
// points to UNICODE_STRING structure that
// that describes maximum size of buffer.
// If .Buffer field is NULL then a buffer is
// allocated.
//

#define RTL_QUERY_REGISTRY_DELETE   0x00000040  // Used to delete value keys after they
// are queried.

#define RTL_QUERY_REGISTRY_NOSTRING 0x00000080  // THIS IS DEPRECATED - use RTL_QUERY_REGISTRY_TYPECHECK
//
// Used with RTL_QUERY_REGISTRY_DIRECT in
// cases where the caller expects a
// non-string value.  Otherwise, the
// assumption that EntryContext points to
// a UNICODE_STRING structure can overrun
// the caller's buffer.
//

#define RTL_QUERY_REGISTRY_TYPECHECK 0x00000100 // Used with RTL_QUERY_REGISTRY_DIRECT to
// validate the registry value type
// expected by caller with actual type thats
// read from the registry.

	NTSTATUS
	NTAPI
	RtlQueryRegistryValues(
		_In_     ULONG RelativeTo,
		_In_     PCWSTR Path,
		_Inout_ _At_(*(*QueryTable).EntryContext, _Pre_unknown_)
		PRTL_QUERY_REGISTRY_TABLE QueryTable,
		_In_opt_ PVOID Context,
		_In_opt_ PVOID Environment
	);

	//
// The following values for the RelativeTo parameter determine what the
// Path parameter to RtlQueryRegistryValues is relative to.
//

#define RTL_REGISTRY_ABSOLUTE     0   // Path is a full path
#define RTL_REGISTRY_SERVICES     1   // \Registry\Machine\System\CurrentControlSet\Services
#define RTL_REGISTRY_CONTROL      2   // \Registry\Machine\System\CurrentControlSet\Control
#define RTL_REGISTRY_WINDOWS_NT   3   // \Registry\Machine\Software\Microsoft\Windows NT\CurrentVersion
#define RTL_REGISTRY_DEVICEMAP    4   // \Registry\Machine\Hardware\DeviceMap
#define RTL_REGISTRY_USER         5   // \Registry\User\CurrentUser
#define RTL_REGISTRY_MAXIMUM      6
#define RTL_REGISTRY_HANDLE       0x40000000    // Low order bits are registry handle
#define RTL_REGISTRY_OPTIONAL     0x80000000    // Indicates the key node is optional

#endif //_NTDDK_


#ifdef __cplusplus
}
#endif

