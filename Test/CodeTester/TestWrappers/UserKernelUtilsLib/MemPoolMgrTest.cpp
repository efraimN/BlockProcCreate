#include "Precompiled.h"

#include <WppIncludes.h>

#include <IUserKernelUtils.h>

using namespace UserKernelUtilsLib;

struct PoolMemMgrTestStruct 
{
	BYTE Count;
	DWORD Magic;
	WCHAR Test[10];
};

/*!
MemPoolMgrTest
For Debug purposes ONLY

Usage:
MemPoolMgrTest

Example:

*/
int RUN_FUNCTION MemPoolMgrTest()
{
	int RetVal = -1;
	IMemPoolMgr* pMemPoolMgr;
	size_t RequiredSize;
	PCHAR MemZone = NULL;
	PoolMemMgrTestStruct* TestArray[20];

	pMemPoolMgr = IMemPoolMgr::GetNewInstance();

	if (!pMemPoolMgr)
	{
		goto Leave;
	}

	RequiredSize = IMemPoolMgr::GetRequiredMemoryZoneLength(sizeof(PoolMemMgrTestStruct), 20);
	MemZone = new CHAR[RequiredSize];
	if (!MemZone)
	{
		goto Leave;
	}

	if (!pMemPoolMgr->Start(
		MemZone,
		RequiredSize,
		sizeof(PoolMemMgrTestStruct),
		20
	))
	{
		goto Leave;
	}
	PoolMemMgrTestStruct* Test;
	for (BYTE i=0;i<18;i++)
	{
		Test = (PoolMemMgrTestStruct * )pMemPoolMgr->AllocateFromPool();
		Test->Count = i;
		Test->Test[1] = i;
		TestArray[i] = Test;
	}

	for (BYTE i = 0; i < 6; i++)
	{
		pMemPoolMgr->FreeFromPool(TestArray[i]);
	}

	for (BYTE i = 0; i < 10; i++)
	{
		Test = (PoolMemMgrTestStruct*)pMemPoolMgr->AllocateFromPool();
		Test->Count = i;
		Test->Test[1] = i;
		TestArray[i] = Test;
	}

	RetVal = 0;
Leave:
	if (pMemPoolMgr)
	{
		IMemPoolMgr::FreeInstance(pMemPoolMgr);
	}
	if (MemZone)
	{
		delete[] MemZone;
	}
	return RetVal;
}

