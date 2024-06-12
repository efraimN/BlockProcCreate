#pragma once

#include <IUserKernelUtils.h>

class CMemPoolMgr : public UserKernelUtilsLib::IMemPoolMgr
{
public:
	friend IMemPoolMgr;

	virtual
	BOOLEAN Start(PVOID MemoryZone, size_t Length, size_t DataBlockSize, size_t NumberOfBlocks);

	virtual
	VOID Stop();

	virtual
	PVOID AllocateFromPool();

	virtual
	BOOL FreeFromPool(PVOID DataBlock);

	virtual  size_t UsedBlocks() { return (m_DataBlockSize - m_FreeBlocks); }

private:
	CMemPoolMgr();
	virtual ~CMemPoolMgr();

	UserKernelUtilsLib::ILinkListLibInt* m_FreeBlocksList;
	size_t m_DataBlockSize;
	size_t m_NumberOfBlocks;
	size_t m_FreeBlocks;
};

