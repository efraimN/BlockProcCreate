#pragma once

#include <IUtilsInt.h>
#include <IMapFileInt.h>
#include <IFileUtilsInt.h>
#include <IRegUtilsInt.h>
#include <ICriticalSectionInt.h>
#include <ILinkListLibInt.h>
#include <IMemPoolMgr.h>
#include <IParsePe.h>

#ifdef _NTDDK_
extern "C"
BOOL
NTAPI
WriteProcessMemory(
	_In_ HANDLE hProcess,
	_In_ LPVOID lpBaseAddress,
	_In_ LPCVOID lpBuffer,
	_In_ SIZE_T nSize,
	_Out_opt_ SIZE_T * lpNumberOfBytesWritten
);

extern "C"
BOOL
NTAPI
ReadProcessMemory(
	_In_ HANDLE hProcess,
	_In_ LPCVOID lpBaseAddress,
	_Out_ LPVOID lpBuffer,
	_In_ SIZE_T nSize,
	_Out_opt_ SIZE_T * lpNumberOfBytesRead
);

#define RAISE *(volatile UCHAR * const)MM_USER_PROBE_ADDRESS = 0
#define READ_RAISE {UCHAR a; a= *(volatile UCHAR * const)MM_USER_PROBE_ADDRESS;}

#define WriteMemory(mem,Data,DataSize,proc) \
	WriteProcessMemory(\
			proc,\
			mem,\
			Data,\
			DataSize,\
			NULL)


#define ReadMemory(mem,Data,DataSize,proc) \
{\
	if(!ReadProcessMemory(\
			proc,\
			mem,\
			Data,\
			DataSize,\
			NULL))\
	{\
		READ_RAISE;\
	}\
}

#else
VOID
NTAPI
ProbeForRead(
	const volatile VOID* Address,
	SIZE_T Length,
	ULONG Alignment
);

#define RAISE *((char*)0) = 0
#define READ_RAISE {char a; a= *((char*)0);}

#define WriteMemory(mem,Data,DataSize,proc) \
	WriteProcessMemory(\
			proc,\
			mem,\
			Data,\
			DataSize,\
			NULL)


#define ReadMemory(mem,Data,DataSize,proc) \
{\
	if(proc == (HANDLE)-1)\
	{\
		RtlCopyMemory(Data,mem,DataSize);\
	}\
	else\
	{\
		if(!ReadProcessMemory(\
				proc,\
				mem,\
				Data,\
				DataSize,\
				NULL))\
		{\
			READ_RAISE;\
		}\
		}\
}
#endif


