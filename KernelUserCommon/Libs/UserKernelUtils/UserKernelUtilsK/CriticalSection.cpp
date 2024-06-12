#include "Precompiled.h"

#include <WppIncludes.h>

#include "CriticalSection.h"

using namespace UserKernelUtilsLib;

#ifdef _NTDDK_
struct  CriticalSectionData
{
	ERESOURCE m_Eresource;
	KSPIN_LOCK m_SpinLoc;
};
#endif

CriticalSection::CriticalSection(pCriticalSectionLock Lock, CriticalSectionType LockType)
{
	m_Lock = Lock;
#ifdef _NTDDK_
	m_LockType = LockType;

	switch (m_LockType)
	{
	case CriticalSectionType::CriticalSectionShared:
		if (KeGetCurrentIrql() <= APC_LEVEL)
		{
			KeEnterCriticalRegion();
			ExAcquireResourceSharedLite(&((CriticalSectionData*)m_Lock)->m_Eresource, TRUE);
		}
		else
		{
			KeAcquireInStackQueuedSpinLock(
				&((CriticalSectionData*)m_Lock)->m_SpinLoc,
				&m_LocalData
			);
		}
		break;

	case CriticalSectionType::CriticalSectionExclusive:
		KeEnterCriticalRegion();
		ExAcquireResourceExclusiveLite(&((CriticalSectionData*)m_Lock)->m_Eresource, TRUE);
		KeAcquireInStackQueuedSpinLock(
			&((CriticalSectionData*)m_Lock)->m_SpinLoc,
			&m_LocalData
		);
		break;
	}
#else
	UNREFERENCED_PARAMETER(LockType);
	EnterCriticalSection((LPCRITICAL_SECTION)m_Lock);
#endif
}

CriticalSection::~CriticalSection()
{
#ifdef _NTDDK_
	switch (m_LockType)
	{
	case CriticalSectionType::CriticalSectionShared:
		if (KeGetCurrentIrql() <= APC_LEVEL)
		{
			ExReleaseResourceLite(&((CriticalSectionData*)m_Lock)->m_Eresource);
			KeLeaveCriticalRegion();
		}
		else
		{
			KeReleaseInStackQueuedSpinLock(&m_LocalData);
		}
		break;

	case CriticalSectionType::CriticalSectionExclusive:
		KeReleaseInStackQueuedSpinLock(&m_LocalData);
		ExReleaseResourceLite(&((CriticalSectionData*)m_Lock)->m_Eresource);
		KeLeaveCriticalRegion();
		break;
	}
#else
	LeaveCriticalSection((LPCRITICAL_SECTION)m_Lock);
#endif
}

NTSTATUS CriticalSection::InitLockObject(pCriticalSectionLock* Lock)
{
	NTSTATUS status = STATUS_NO_MEMORY;

#ifdef _NTDDK_
	CriticalSectionData* Tmp = (CriticalSectionData*)new ('tirC') BYTE[sizeof(CriticalSectionData)];
	if (!Tmp)
	{
		goto Leave;
	}

	status = ExInitializeResourceLite(&Tmp->m_Eresource);
	KeInitializeSpinLock(&Tmp->m_SpinLoc);

	status = STATUS_SUCCESS;
Leave:
	if (NT_SUCCESS(status))
	{
		*Lock = (pCriticalSectionLock)Tmp;
	}
	else
	{
		delete[] Tmp;
	}
#else
	PBYTE Tmp = new (std::nothrow) BYTE[sizeof(CRITICAL_SECTION)];
	if (!Tmp)
	{
		status = STATUS_NO_MEMORY;
		goto Leave;
	}

	InitializeCriticalSection((LPCRITICAL_SECTION)Tmp);

	status = STATUS_SUCCESS;
Leave:
	if (NT_SUCCESS(status))
	{
		*Lock = (pCriticalSectionLock)Tmp;
	}
	else
	{
		delete[] Tmp;
	}
#endif

	return status;
}

NTSTATUS CriticalSection::DeleteLockObject(pCriticalSectionLock Lock)
{
#ifdef _NTDDK_
	NTSTATUS status = STATUS_INVALID_PARAMETER;

	if (!Lock)
	{
		return status;
	}

	if (Lock)
	{
		status = ExDeleteResourceLite(&((CriticalSectionData*)Lock)->m_Eresource);

		delete[](PBYTE)Lock;
	}

	return status;
#else
	DeleteCriticalSection((LPCRITICAL_SECTION)Lock);

	delete[](PBYTE)Lock;

	return STATUS_SUCCESS;
#endif
}
