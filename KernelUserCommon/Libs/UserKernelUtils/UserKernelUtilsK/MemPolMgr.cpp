#include "Precompiled.h"

#include <WppIncludes.h>

#include "MemPolMgr.h"
using namespace UserKernelUtilsLib;

enum class MemoryUssage : DWORD
{
	UNUSED_MEMORY_BLOCK = 0xBADCACA0,
	USED_MEMORYBLOCK  = 0xADDC0FEE,
};

class FreeListElement : public ILinkListElement
{
public:
	FreeListElement() {};
	~FreeListElement() {};

private:
};

#pragma pack(push)
#pragma pack(1)
struct Elements 
{
	DWORD Magic;
	FreeListElement ListElement;
};
#pragma pack(pop)

CMemPoolMgr::CMemPoolMgr()
{
	m_FreeBlocksList = NULL;
	m_FreeBlocks = 0;
}

CMemPoolMgr::~CMemPoolMgr()
{
	Stop();
}

IMemPoolMgr* IMemPoolMgr::GetNewInstance()
{
#ifdef _NTDDK_
	return new ('lCpM') CMemPoolMgr();
#else
	return new CMemPoolMgr();
#endif
}

VOID IMemPoolMgr::FreeInstance(IMemPoolMgr* Interface)
{
	delete (CMemPoolMgr*)Interface;
}

size_t IMemPoolMgr::GetRequiredMemoryZoneLength(size_t DataBlockSize, size_t NumberOfBlocks)
{
	if (DataBlockSize < sizeof(Elements)) // check that the DataBlockSize is large enough to be managed by our code...
	{
		return 0;
	}

	return NumberOfBlocks*(DataBlockSize+sizeof(DWORD)); // we use a DWORD for management of each block of data
}

BOOLEAN CMemPoolMgr::Start(PVOID MemoryZone, size_t Length, size_t DataBlockSize, size_t NumberOfBlocks)
{
	BOOLEAN RetVal = FALSE;
	Elements* Element;
	if (Length < GetRequiredMemoryZoneLength(DataBlockSize, NumberOfBlocks))
	{
		goto Leave;
	}

	m_FreeBlocksList = ILinkListLibInt::GetNewInstance();
	if (!m_FreeBlocksList)
	{
		goto Leave;
	}

	if (!m_FreeBlocksList->Start())
	{
		goto Leave;
	}

	m_DataBlockSize = DataBlockSize;
	m_NumberOfBlocks = NumberOfBlocks;

	Element = (Elements*)MemoryZone;
	for (size_t i = 0; i < NumberOfBlocks; i++)
	{
		Element->Magic = (DWORD)MemoryUssage::UNUSED_MEMORY_BLOCK;
		m_FreeBlocksList->PushItemHead(&Element->ListElement);
		Element = (Elements*)((BYTE*)Element + m_DataBlockSize + sizeof(DWORD));
	}
	m_FreeBlocks = NumberOfBlocks;

	RetVal = TRUE;
Leave:
	return RetVal;
}

VOID CMemPoolMgr::Stop()
{
	if (m_FreeBlocksList)
	{
		m_FreeBlocksList->Stop();
		ILinkListLibInt::FreeInstance(m_FreeBlocksList);
		m_FreeBlocksList = NULL;
	}
}

PVOID CMemPoolMgr::AllocateFromPool()
{
	PVOID RetVal = NULL;
	Elements* Element;
	FreeListElement* ListEntry;
	if (!m_FreeBlocks)
	{
		goto Leave;
	}

	ListEntry = (FreeListElement*)m_FreeBlocksList->PopItemHead();
	if (!ListEntry)
	{
		goto Leave;
	}
	m_FreeBlocks--;
	Element = CONTAINING_RECORD(ListEntry, Elements, ListElement);
	Element->Magic = (DWORD)MemoryUssage::USED_MEMORYBLOCK;
	RetVal = &Element->ListElement;
	//RtlZeroMemory(RetVal, sizeof(FreeListElement));
	RtlZeroMemory(RetVal, m_DataBlockSize);

Leave:
	return RetVal;
}

BOOL CMemPoolMgr::FreeFromPool(PVOID DataBlock)
{
	BOOL RetVal = FALSE;
	//TODO do sanity checks, if the pointer is in bounds ....
	Elements* Element = CONTAINING_RECORD(DataBlock, Elements, ListElement);
	Element->Magic;
	if (Element->Magic != (DWORD)MemoryUssage::USED_MEMORYBLOCK)
	{
		goto Leave;
	}
	Element->Magic = (DWORD)MemoryUssage::UNUSED_MEMORY_BLOCK;
	m_FreeBlocksList->PushItemHead(&Element->ListElement);
	m_FreeBlocks++;
	RetVal = TRUE;
Leave:
	return RetVal;
}

BOOLEAN  IMemPoolMgr::RetrieveUsedBlocks(PVOID MemoryZone, size_t Length, size_t DataBlockSize, size_t NumberOfBlocks, pfnRetrieveUsedBlocks RetrieveUsedBlocksCallBack, PVOID context)
{
	BOOLEAN RetVal = FALSE;
	Elements* Element;

	if (Length < IMemPoolMgr::GetRequiredMemoryZoneLength(DataBlockSize, NumberOfBlocks))
	{
		goto Leave;
	}

	Element = (Elements*)MemoryZone;
	for (size_t i = 0; i < NumberOfBlocks; i++)
	{
		if (Element->Magic == (DWORD)MemoryUssage::USED_MEMORYBLOCK)
		{
			if (!RetrieveUsedBlocksCallBack(&Element->ListElement, context))
			{
				goto Leave;
			}
		}
		Element = (Elements*)((BYTE*)Element + DataBlockSize + sizeof(DWORD));
	}

	RetVal = TRUE;
Leave:
	return RetVal;
}

