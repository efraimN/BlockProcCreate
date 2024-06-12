#pragma once

#include <IUserKernelUtils.h>

class CLinkListLib : public UserKernelUtilsLib::ILinkListLibInt
{
public:
	friend class UserKernelUtilsLib::ILinkListLibInt;

// 	CLinkListLib(const CLinkListLib& other) = delete;
// 	CLinkListLib& operator=(const CLinkListLib& other) = delete;

	virtual
	BOOLEAN Start();

	virtual
	VOID Stop();


	virtual
	VOID PushItemHead(UserKernelUtilsLib::ILinkListElement* Element);

	virtual
	UserKernelUtilsLib::ILinkListElement* PopItemHead();

	virtual
	UserKernelUtilsLib::ILinkListElement* GetElement(pfnFoundElement FoundElementCallback, PVOID FindContext, BOOLEAN Removal = TRUE);

	virtual
	UserKernelUtilsLib::ILinkListElement* GetElementEx(pfnFoundElementEx FoundElementCallback, PVOID FindContext, BOOLEAN Removal);

	virtual
	VOID PushItemTail(UserKernelUtilsLib::ILinkListElement* Element);

	virtual
	UserKernelUtilsLib::ILinkListElement* PopItemTail();

protected:

private:
	CLinkListLib();
	virtual~CLinkListLib();
	
	CLinkListLib(const CLinkListLib& other);
	CLinkListLib& operator=(const CLinkListLib& other);

	UserKernelUtilsLib::ILinkListElement* GetElementCommon(PVOID Callback, BOOL IsExtendedCall, PVOID FindContext, BOOLEAN Removal);

	BOOLEAN
	RemoveElement(
		UserKernelUtilsLib::ILinkListElement* Element,
		BOOLEAN AcquireLock = FALSE
	);

	VOID
	ListInsertHeadList(
		_Inout_ PLIST_ENTRY ListHead,
		_Out_ PLIST_ENTRY Entry
	);
	
	PLIST_ENTRY
	ListRemoveHeadList(
		_Inout_ PLIST_ENTRY ListHead
	);
	
	VOID
	ListInsertTailList(
		_Inout_ PLIST_ENTRY ListHead,
		_Out_ PLIST_ENTRY Entry
	);
	
	PLIST_ENTRY
	ListRemoveTailList(
		_Inout_ PLIST_ENTRY ListHead
	);
	
	BOOLEAN
	ListIsListEmpty(
		_In_ const LIST_ENTRY * ListHead
	);
	
	VOID
	ListInitializeListHead(
		_Out_ PLIST_ENTRY ListHead
	);

	LIST_ENTRY m_ListHead;
	BOOLEAN m_Inited/* = FALSE*/;

	PVOID m_Lock;

	DWORD m_NumberOfElements;
};
