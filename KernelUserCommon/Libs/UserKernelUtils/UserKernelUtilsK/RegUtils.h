#pragma once

#include <IUserKernelUtils.h>

class CRegUtils : public UserKernelUtilsLib::IRegUtilsInt
{
public:
	static CRegUtils *GetInstance();

	virtual
	NTSTATUS
	RegDeleteAllKeys(HANDLE ParentKey);

	virtual
	NTSTATUS
	RegSetValue(
		IN     ULONG           RelativeTo,
		IN     PWSTR           KeyNamePath,
		IN     PWSTR           ParameterName,
		IN OUT PVOID           ParameterValue,
		IN OUT ULONG           ParameterLength,
		IN     ULONG		   ValueType
	);

	virtual
	NTSTATUS
	RegGetValue(
		IN     ULONG           RelativeTo,
		IN     PWSTR           KeyNamePath,
		IN     PWSTR           ParameterName,
		IN OUT PVOID           ParameterValue,
		IN OUT PDWORD          ParameterLength
	);

	virtual
	BOOL
	RegDeletekey(PWCHAR Path);

	virtual
	HANDLE
	RegOpenCreateKey(
		_In_  ULONG RelativeTo,
		_In_  PWSTR Path,
		_In_  PWSTR SubPath,
		BOOLEAN Open_Create,
		ACCESS_MASK DesiredAccess,
		ULONG CreateOptions
	);

private:
	CRegUtils();
	virtual ~CRegUtils();

	NTSTATUS RegDeleteAllSubKeys(HANDLE	ParentKey);

};

