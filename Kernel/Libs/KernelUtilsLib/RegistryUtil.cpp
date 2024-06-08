#include "Precompiled.h"

#include <WppIncludes.h>

#include <KernelUtilsLib.h>

#include "IUserKernelUtils.h"

UNICODE_STRING KernelUtilsLib::CRegistryUtil::m_RegistryPath = { 0, 0, NULL };
UNICODE_STRING KernelUtilsLib::CRegistryUtil::m_CurrentControlSetPath = { 0, 0, NULL };
UNICODE_STRING KernelUtilsLib::CRegistryUtil::m_DriverImageFileName = { 0, 0, NULL };
UNICODE_STRING KernelUtilsLib::CRegistryUtil::m_Attitude = { 0, 0, NULL };



NTSTATUS
KernelUtilsLib::CRegistryUtil::InitDriverRegPath(IN PUNICODE_STRING RegistryPath)
{
	NTSTATUS RetVal = STATUS_INSUFFICIENT_RESOURCES;
	DWORD TmpDword;

	if (m_RegistryPath.Buffer)
	{
		// fail, coz already inited
		RetVal = STATUS_UNSUCCESSFUL;
		goto Leave;
	}

	m_RegistryPath.Length = RegistryPath->Length;
	if (0xFFFF < (RegistryPath->MaximumLength + 2))
	{
		goto Leave;
	}
	m_RegistryPath.MaximumLength = RegistryPath->MaximumLength + 2;
	m_RegistryPath.Buffer = NULL;

	// allocate memory to hold this registry path,
	m_RegistryPath.Buffer = (PWCH)new('02YH') BYTE[m_RegistryPath.MaximumLength];
	if (!m_RegistryPath.Buffer)
	{
		goto Leave;
	}

	if (!NT_SUCCESS(RtlUpcaseUnicodeString(&m_RegistryPath, RegistryPath, FALSE)))
	{
		goto Leave;
	}
	m_RegistryPath.Buffer[m_RegistryPath.Length / 2] = 0;

	ULONG64 Length = (ULONG64)wcsstr(m_RegistryPath.Buffer, L"SERVICES");
	if (!Length)
	{
		goto Leave;
	}
	Length -= (ULONG64)m_RegistryPath.Buffer;
	m_CurrentControlSetPath.Length = (USHORT)Length;
	m_CurrentControlSetPath.MaximumLength = m_CurrentControlSetPath.Length + 2;
	m_CurrentControlSetPath.Buffer = (PWCH)new('03YH') BYTE[m_CurrentControlSetPath.MaximumLength];
	if (!m_CurrentControlSetPath.Buffer)
	{
		goto Leave;
	}
	memcpy(m_CurrentControlSetPath.Buffer, m_RegistryPath.Buffer, m_CurrentControlSetPath.Length);
	m_CurrentControlSetPath.Buffer[m_CurrentControlSetPath.Length / 2] = 0;

	m_DriverImageFileName.MaximumLength = 0;

	RetVal = UserKernelUtilsLib::IRegUtilsInt::GetInstance()->RegGetValue(
		RTL_REGISTRY_ABSOLUTE,
		m_RegistryPath.Buffer,
		L"ImagePath",
		NULL,
		&TmpDword
	);
	if (!NT_SUCCESS(RetVal))
	{
		RetVal = STATUS_UNSUCCESSFUL;
		goto Leave;
	}
	RetVal = STATUS_UNSUCCESSFUL;

	if (!TmpDword)
	{
		goto Leave;
	}
	TmpDword += 2;
	if (TmpDword > MAXUSHORT)
	{
		goto Leave;
	}

	m_DriverImageFileName.Buffer = (PWCH)new('04YH') BYTE[TmpDword];
	if (!m_DriverImageFileName.Buffer)
	{
		goto Leave;
	}

	RetVal = UserKernelUtilsLib::IRegUtilsInt::GetInstance()->RegGetValue(
		RTL_REGISTRY_ABSOLUTE,
		m_RegistryPath.Buffer,
		L"ImagePath",
		m_DriverImageFileName.Buffer,
		&TmpDword
	);
	if (!NT_SUCCESS(RetVal))
	{
		RetVal = STATUS_UNSUCCESSFUL;
		goto Leave;
	}
	m_DriverImageFileName.MaximumLength = (USHORT)TmpDword;
	m_DriverImageFileName.Length = m_DriverImageFileName.MaximumLength - 2;

	RtlInitUnicodeString(&m_Attitude, L"260550.1");// should retrieve it from the registry

	RetVal = STATUS_SUCCESS;

Leave:
	if (!NT_SUCCESS(RetVal))
	{
		CRegistryUtil::ReleaseDriverRegPath();
	}
	
	return RetVal;
}

void KernelUtilsLib::CRegistryUtil::ReleaseDriverRegPath()
{
	if (m_CurrentControlSetPath.Buffer)
	{
		delete[] (PBYTE)m_CurrentControlSetPath.Buffer;
	}
	m_CurrentControlSetPath.Buffer = NULL;
	m_CurrentControlSetPath.Length = 0;
	m_CurrentControlSetPath.MaximumLength = 0;

	if (m_RegistryPath.Buffer)
	{
		delete[](PBYTE)m_RegistryPath.Buffer;
	}
	m_RegistryPath.Buffer = NULL;
	m_RegistryPath.Length = 0;
	m_RegistryPath.MaximumLength = 0;

	if (m_DriverImageFileName.Buffer)
	{
		delete[](PBYTE)m_DriverImageFileName.Buffer;
	}
	m_DriverImageFileName.Buffer = NULL;
	m_DriverImageFileName.Length = 0;
	m_DriverImageFileName.MaximumLength = 0;
}

