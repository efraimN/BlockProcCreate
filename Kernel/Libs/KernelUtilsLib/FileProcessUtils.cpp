#include "Precompiled.h"

#include <WppIncludes.h>

#include "FileProcessUtils.h"

namespace KernelUtilsLib
{

	CProcessUtils::CProcessUtils()
	{
		m_FullNtPathName = NULL;
		m_ProcessObj = NULL;
		m_FullDosPathName = NULL;
	}

	CProcessUtils::~CProcessUtils()
	{
	}

	IProcessUtils* IProcessUtils::GetNewInstance(PEPROCESS ProcessObj)
	{
		IProcessUtils* RetVal = NULL;
		NTSTATUS status;

		CProcessUtils* pProcessUtils;

		POBJECT_NAME_INFORMATION  ProcesssFullNameInformation = NULL;
		PFILE_OBJECT FileObject = NULL;
		DWORD ProcId = DOWNCASTHANDLE(PsGetProcessId(ProcessObj));

		pProcessUtils = new ('eliF') CProcessUtils();
		if (!pProcessUtils)
		{
			LOG_OUT(DBG_ERROR, "new ('eliF') CProcessUtils() Failed ProcessObj %p", ProcessObj);
			goto Leave;
		}

		pProcessUtils->m_ProcessObj = ProcessObj;
		ObReferenceObject(pProcessUtils->m_ProcessObj);

		pProcessUtils->m_FullDosPathName = NULL;
		pProcessUtils->m_FullNtPathName = NULL;
		status = SeLocateProcessImageName(ProcessObj, &pProcessUtils->m_FullNtPathName);
		if (!NT_SUCCESS(status))
		{
			LOG_OUT(DBG_ERROR, "SeLocateProcessImageName Failed ProcessPID %d status 0x%08X", ProcId, status);
// 			goto Leave;
		}
		status = PsReferenceProcessFilePointer(ProcessObj, &FileObject);
		if (NT_SUCCESS(status))
		{
			if (FileObject)
			{
				status = IoQueryFileDosDeviceName(
					FileObject,
					&ProcesssFullNameInformation);
				ObDereferenceObject(FileObject);
				if (NT_SUCCESS(status))
				{
					pProcessUtils->m_FullDosPathName = &ProcesssFullNameInformation->Name;
				}
				else
				{
					LOG_OUT(DBG_ERROR, "IoQueryFileDosDeviceName Failed ProcessObj %p status 0x%08X", ProcessObj, status);
				}
			}
		}
		else
		{
			LOG_OUT(DBG_ERROR, "%!FUNC! PsReferenceProcessFilePointer Failed ProcessID %d status 0x%08X", ProcId, status);
		}

		RetVal = pProcessUtils;
	Leave:
		if (!RetVal)
		{
			if (pProcessUtils)
			{
				ReleaseInstance(pProcessUtils);
			}
		}
		return RetVal;
	}

	VOID IProcessUtils::ReleaseInstance(IProcessUtils* Instance)
	{
		CProcessUtils* pProcessUtils = (CProcessUtils*)Instance;
		if (!pProcessUtils)
		{
			return;
		}

		if (pProcessUtils->m_FullNtPathName)
		{
			ExFreePool(pProcessUtils->m_FullNtPathName);
		}

		if (pProcessUtils->m_FullDosPathName)
		{
			POBJECT_NAME_INFORMATION  ProcesssFullNameInformation;

			ProcesssFullNameInformation = CONTAINING_RECORD(pProcessUtils->m_FullDosPathName, OBJECT_NAME_INFORMATION, Name);
			ExFreePool(ProcesssFullNameInformation);
		}

		if (pProcessUtils->m_ProcessObj)
		{
			ObDereferenceObject(pProcessUtils->m_ProcessObj);
		}

		delete pProcessUtils;
	}
};
