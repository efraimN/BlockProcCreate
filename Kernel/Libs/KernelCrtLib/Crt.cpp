#include "Precompiled.h"

#include <WppIncludes.h>

#include "Crt.h"

#pragma section(".CRT$XCA",long,read)
#pragma section(".CRT$XCZ",long,read)
#define _CRTALLOC(x) __declspec(allocate(x))


typedef void(__cdecl *PVOIDFUNC)();

typedef struct _ATEXIT_LIST_ENTRY {
	LIST_ENTRY link;
	PVOIDFUNC func;
} ATEXIT_LIST_ENTRY, *PATEXIT_LIST_ENTRY;


_CRTALLOC(".CRT$XCA") PVOIDFUNC __xc_a[] = { NULL };
_CRTALLOC(".CRT$XCZ") PVOIDFUNC __xc_z[] = { NULL };

KSPIN_LOCK exitlock;      // spin lock to protect atexit list
LIST_ENTRY exitlist;      // anchor of atexit list


int __cdecl atexit(PVOIDFUNC func)
{
	auto ProcEntry = [](BOOL start)
	{
		if (start)
			LOG_OUT(DBG_TRACE_FUNCTIONS, ">>> Start file %!FILE! line: %!LINE!");
		else
			LOG_OUT(DBG_TRACE_FUNCTIONS, "<<< Finish file %!FILE! line: %!LINE!");

	};

	PROC_ENTRY;

	PATEXIT_LIST_ENTRY p = (PATEXIT_LIST_ENTRY)ExAllocatePoolWithTag(NonPagedPool, sizeof(ATEXIT_LIST_ENTRY), '01YH');
	if (!p)
	{
		return 0;
	}

	p->func = func;
	ExInterlockedInsertHeadList(&exitlist, &p->link, &exitlock);

	return 1;
}

void CrtExit()
{
	auto ProcEntry = [](BOOL start)
	{
		if (start)
			LOG_OUT(DBG_TRACE_FUNCTIONS, ">>> Start file %!FILE! line: %!LINE!");
		else
			LOG_OUT(DBG_TRACE_FUNCTIONS, "<<< Finish file %!FILE! line: %!LINE!");

	};

	PROC_ENTRY;

	PATEXIT_LIST_ENTRY p;

	p = (PATEXIT_LIST_ENTRY)ExInterlockedRemoveHeadList(&exitlist, &exitlock);
	while (p)
	{
		(*p->func)();
		ExFreePool(p);
		p = (PATEXIT_LIST_ENTRY)ExInterlockedRemoveHeadList(&exitlist, &exitlock);
	}
}

void CrtInit()
{

	auto ProcEntry = [](BOOL start)
	{
		if (start)
			LOG_OUT(DBG_TRACE_FUNCTIONS, ">>> Start file %!FILE! line: %!LINE!");
		else
			LOG_OUT(DBG_TRACE_FUNCTIONS, "<<< Finish file %!FILE! line: %!LINE!");

	};

	PROC_ENTRY;

	InitializeListHead(&exitlist);
	KeInitializeSpinLock(&exitlock);

	for (PVOIDFUNC* p = __xc_a; p < __xc_z; ++p) if (*p != NULL)(**p)();
}

// void * __cdecl operator new(size_t nSize)
// {
// 	void* retval;
// 	retval = ExAllocatePoolWithTag(NonPagedPool, nSize, 'ARFE'); // EFRA(per)
// 	if (retval)
// 	{
// 		RtlZeroMemory(retval, nSize);
// 	}
// 	return retval;
// }

void * __cdecl operator new(size_t nSize, ULONG Tag)
{
	void* retval;
	retval = ExAllocatePoolWithTag(NonPagedPool, nSize, Tag);
	if (retval)
	{
		RtlZeroMemory(retval, nSize);
	}
	return retval;
}

void __cdecl operator delete(void * p)
{
	if (p) ExFreePool(p);
}

