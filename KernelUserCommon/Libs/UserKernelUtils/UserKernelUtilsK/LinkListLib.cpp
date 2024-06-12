#include "Precompiled.h"

#include <WppIncludes.h>

#include "LinkListLib.h"

using namespace UserKernelUtilsLib;

ILinkListLibInt *ILinkListLibInt::GetNewInstance()
{
#ifdef _NTDDK_
	return new('tsiL') CLinkListLib();
#else
	return new(std::nothrow) CLinkListLib();
#endif
}

VOID ILinkListLibInt::FreeInstance(ILinkListLibInt * Interface)
{
	delete (CLinkListLib*)Interface;
}


ILinkListElement::ILinkListElement()
{
	m_ListElement.Blink = NULL;
	m_ListElement.Flink = NULL;
}

ILinkListElement::~ILinkListElement()
{
}

VOID
CLinkListLib::ListInitializeListHead(
	_Out_ PLIST_ENTRY ListHead
)

{
	ListHead->Flink = ListHead->Blink = ListHead;
	m_NumberOfElements = 0;

	return;
}

BOOLEAN
CLinkListLib::ListIsListEmpty(
	_In_ const LIST_ENTRY * ListHead
)

{
	return (BOOLEAN)(ListHead->Flink == ListHead);
}

PLIST_ENTRY
CLinkListLib::ListRemoveHeadList(
	_Inout_ PLIST_ENTRY ListHead
)

{
	PLIST_ENTRY Entry;
	PLIST_ENTRY NextEntry;

	Entry = ListHead->Flink;

	NextEntry = Entry->Flink;
	if ((Entry->Blink != ListHead) || (NextEntry->Blink != Entry))
	{
		return NULL;
	}

	ListHead->Flink = NextEntry;
	NextEntry->Blink = ListHead;
	m_NumberOfElements--;

	return Entry;
}

VOID
CLinkListLib::ListInsertHeadList(
	_Inout_ PLIST_ENTRY ListHead,
	_Out_ PLIST_ENTRY Entry
)
{
	PLIST_ENTRY NextEntry;

	NextEntry = ListHead->Flink;
	Entry->Flink = NextEntry;
	Entry->Blink = ListHead;
	if (NextEntry->Blink != ListHead)
	{
		return;
	}

	NextEntry->Blink = Entry;
	ListHead->Flink = Entry;
	m_NumberOfElements++;

	return;
}

PLIST_ENTRY
CLinkListLib::ListRemoveTailList(
	_Inout_ PLIST_ENTRY ListHead
)
{
	PLIST_ENTRY Entry;
	PLIST_ENTRY PrevEntry;

	Entry = ListHead->Blink;

	PrevEntry = Entry->Blink;
	if ((Entry->Flink != ListHead) || (PrevEntry->Flink != Entry))
	{
		return NULL;
	}

	ListHead->Blink = PrevEntry;
	PrevEntry->Flink = ListHead;
	m_NumberOfElements--;

	return Entry;
}

VOID
CLinkListLib::ListInsertTailList(
	_Inout_ PLIST_ENTRY ListHead,
	_Out_ PLIST_ENTRY Entry
)
{

	PLIST_ENTRY PrevEntry;

	PrevEntry = ListHead->Blink;
	Entry->Flink = ListHead;
	Entry->Blink = PrevEntry;
	if (PrevEntry->Flink != ListHead)
	{
		return;
	}

	PrevEntry->Flink = Entry;
	ListHead->Blink = Entry;
	m_NumberOfElements++;

	return;
}

CLinkListLib::CLinkListLib()
{
	auto ProcEntry = [](BOOL start)
	{
		if (start)
			LOG_OUT(DBG_TRACE_FUNCTIONS, ">>> Start file %!FILE! line: %!LINE!");
		else
			LOG_OUT(DBG_TRACE_FUNCTIONS, "<<< Finish file %!FILE! line: %!LINE!");

	};
	PROC_ENTRY;
	m_Inited = FALSE;
}

CLinkListLib::~CLinkListLib()
{
	auto ProcEntry = [](BOOL start)
	{
		if (start)
			LOG_OUT(DBG_TRACE_FUNCTIONS, ">>> Start file %!FILE! line: %!LINE!");
		else
			LOG_OUT(DBG_TRACE_FUNCTIONS, "<<< Finish file %!FILE! line: %!LINE!");

	};
	PROC_ENTRY;
}

BOOLEAN CLinkListLib::Start()
{
	auto ProcEntry = [](BOOL start)
	{
		if (start)
			LOG_OUT(DBG_TRACE_FUNCTIONS, ">>> Start file %!FILE! line: %!LINE!");
		else
			LOG_OUT(DBG_TRACE_FUNCTIONS, "<<< Finish file %!FILE! line: %!LINE!");

	};
	PROC_ENTRY;

	m_NumberOfElements = 0;

	ListInitializeListHead(&m_ListHead);
	NT_SUCCESS(CriticalSection::InitLockObject((CriticalSection::pCriticalSectionLock*)&m_Lock)) ? m_Inited = TRUE : m_Inited = FALSE;
	return m_Inited;
}

VOID CLinkListLib::Stop()
{
	auto ProcEntry = [](BOOL start)
	{
		if (start)
			LOG_OUT(DBG_TRACE_FUNCTIONS, ">>> Start file %!FILE! line: %!LINE!");
		else
			LOG_OUT(DBG_TRACE_FUNCTIONS, "<<< Finish file %!FILE! line: %!LINE!");

	};
	PROC_ENTRY;
	if (!m_Inited)
	{
		return;
	}

	CriticalSection::DeleteLockObject((CriticalSection::pCriticalSectionLock)m_Lock);
}

VOID CLinkListLib::PushItemHead(ILinkListElement* This)
{
	CriticalSection a((CriticalSection::pCriticalSectionLock)m_Lock, CriticalSection::CriticalSectionType::CriticalSectionExclusive);
	ListInsertHeadList(&m_ListHead, &This->m_ListElement);
}

ILinkListElement* CLinkListLib::PopItemHead()
{
	ILinkListElement* RetVal = NULL;;

	PLIST_ENTRY Entry;

	CriticalSection a((CriticalSection::pCriticalSectionLock)m_Lock, CriticalSection::CriticalSectionType::CriticalSectionExclusive);

	if (ListIsListEmpty(&m_ListHead))
	{
		goto Leave;
	}
	Entry = ListRemoveHeadList(&m_ListHead);
	if (!Entry)
	{
		goto Leave;
	}
	RetVal = CONTAINING_RECORD(Entry, ILinkListElement, m_ListElement);

Leave:
	return RetVal;
}

VOID CLinkListLib::PushItemTail(UserKernelUtilsLib::ILinkListElement* This)
{
	CriticalSection a((CriticalSection::pCriticalSectionLock)m_Lock, CriticalSection::CriticalSectionType::CriticalSectionExclusive);
	ListInsertTailList(&m_ListHead, &This->m_ListElement);
}

UserKernelUtilsLib::ILinkListElement* CLinkListLib::PopItemTail()
{
	ILinkListElement* RetVal = NULL;;

	PLIST_ENTRY Entry;

	CriticalSection a((CriticalSection::pCriticalSectionLock)m_Lock, CriticalSection::CriticalSectionType::CriticalSectionExclusive);

	if (ListIsListEmpty(&m_ListHead))
	{
		goto Leave;
	}
	Entry = ListRemoveTailList(&m_ListHead);
	if (!Entry)
	{
		goto Leave;
	}
	RetVal = CONTAINING_RECORD(Entry, ILinkListElement, m_ListElement);

Leave:
	return RetVal;
}

BOOLEAN CLinkListLib::RemoveElement(ILinkListElement* Element, BOOLEAN AcquireLock /*= FALSE*/)
{
	PLIST_ENTRY PrevEntry;
	PLIST_ENTRY NextEntry;
	PLIST_ENTRY Entry;
	BOOLEAN RetVal = FALSE;

	Entry = &Element->m_ListElement;
	auto DoRemove = [&]()
	{
		NextEntry = Entry->Flink;
		PrevEntry = Entry->Blink;

		if ((NextEntry->Blink != Entry) || (PrevEntry->Flink != Entry))
		{
			goto Leave;
		}

		PrevEntry->Flink = NextEntry;
		NextEntry->Blink = PrevEntry;

		RetVal = /*(PrevEntry == NextEntry)?FALSE:*/TRUE;
		m_NumberOfElements--;
	Leave:
		return;
	};
	if(AcquireLock)
	{
		CriticalSection a((CriticalSection::pCriticalSectionLock)m_Lock, CriticalSection::CriticalSectionType::CriticalSectionExclusive);
		DoRemove();
	}
	else
	{
		DoRemove();
	}

	return RetVal;
}

ILinkListElement* CLinkListLib::GetElementCommon(PVOID Callback, BOOL IsExtendedCall, PVOID FindContext, BOOLEAN Removal)
{
	ILinkListElement* RetVal = NULL;
	ILinkListElement* TmpVal = NULL;

	CriticalSection a(
		(CriticalSection::pCriticalSectionLock)m_Lock,
		Removal? CriticalSection::CriticalSectionType::CriticalSectionExclusive: CriticalSection::CriticalSectionType::CriticalSectionShared
	);

	PLIST_ENTRY temp = m_ListHead.Flink;
	while (temp != &m_ListHead)
	{
		BOOL FoundElmnt;
		TmpVal = CONTAINING_RECORD(temp, ILinkListElement, m_ListElement);
		if (IsExtendedCall)
		{
			FoundElmnt = ((pfnFoundElementEx)Callback)(
				TmpVal,
				m_NumberOfElements,
				FindContext
			);
		}
		else
		{
			FoundElmnt = ((pfnFoundElement)Callback)(
				TmpVal,
				FindContext
			);
		}
		if (FoundElmnt)
		{
			if (Removal)
			{
				RemoveElement(TmpVal);
			}
			RetVal = TmpVal;
			break;
		}
		temp = temp->Flink;
	}

	return RetVal;
}

ILinkListElement* CLinkListLib::GetElement(pfnFoundElement FoundElementCallback, PVOID FindContext, BOOLEAN Removal)
{
	return GetElementCommon(FoundElementCallback, FALSE, FindContext, Removal);
}

ILinkListElement* CLinkListLib::GetElementEx(pfnFoundElementEx FoundElementCallback, PVOID FindContext, BOOLEAN Removal)
{
	return GetElementCommon(FoundElementCallback, TRUE, FindContext, Removal);
}
