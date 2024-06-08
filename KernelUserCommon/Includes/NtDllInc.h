#pragma once

#ifndef _NTDDK_
#include <winioctl.h>
#endif

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef _NTDDK_

#define NtCurrentProcess() ( (HANDLE)(LONG_PTR) -1 )  
#define ZwCurrentProcess() NtCurrentProcess()         
	typedef struct _KEY_BASIC_INFORMATION {
		LARGE_INTEGER LastWriteTime;
		ULONG   TitleIndex;
		ULONG   NameLength;
		WCHAR   Name[1];            // Variable length string
	} KEY_BASIC_INFORMATION, * PKEY_BASIC_INFORMATION;

	typedef enum _KEY_INFORMATION_CLASS {
		KeyBasicInformation,
		KeyNodeInformation,
		KeyFullInformation,
		KeyNameInformation,
		KeyCachedInformation,
		KeyFlagsInformation,
		KeyVirtualizationInformation,
		KeyHandleTagsInformation,
		KeyTrustInformation,
		KeyLayerInformation,
		MaxKeyInfoClass  // MaxKeyInfoClass should always be the last enum
	} KEY_INFORMATION_CLASS;

	typedef enum _NTFILE_INFORMATION_CLASS {
		_FileDirectoryInformation = 1,
		FileFullDirectoryInformation,   // 2
		FileBothDirectoryInformation,   // 3
		FileBasicInformation,           // 4
		FileStandardInformation,        // 5
		FileInternalInformation,        // 6
		FileEaInformation,              // 7
		FileAccessInformation,          // 8
		FileNameInformation,            // 9
		FileRenameInformation,          // 10
		FileLinkInformation,            // 11
		FileNamesInformation,           // 12
		FileDispositionInformation,     // 13
		FilePositionInformation,        // 14
		FileFullEaInformation,          // 15
		FileModeInformation,            // 16
		FileAlignmentInformation,       // 17
		FileAllInformation,             // 18
		FileAllocationInformation,      // 19
		FileEndOfFileInformation,       // 20
		FileAlternateNameInformation,   // 21
		FileStreamInformation,          // 22
		FilePipeInformation,            // 23
		FilePipeLocalInformation,       // 24
		FilePipeRemoteInformation,      // 25
		FileMailslotQueryInformation,   // 26
		FileMailslotSetInformation,     // 27
		FileCompressionInformation,     // 28
		FileObjectIdInformation,        // 29
		FileCompletionInformation,      // 30
		FileMoveClusterInformation,     // 31
		FileQuotaInformation,           // 32
		FileReparsePointInformation,    // 33
		FileNetworkOpenInformation,     // 34
		FileAttributeTagInformation,    // 35
		FileTrackingInformation,        // 36
		FileIdBothDirectoryInformation, // 37
		FileIdFullDirectoryInformation, // 38
		FileValidDataLengthInformation, // 39
		FileShortNameInformation,       // 40
		FileIoCompletionNotificationInformation, // 41
		FileIoStatusBlockRangeInformation,       // 42
		FileIoPriorityHintInformation,           // 43
		FileSfioReserveInformation,              // 44
		FileSfioVolumeInformation,               // 45
		FileHardLinkInformation,                 // 46
		FileProcessIdsUsingFileInformation,      // 47
		FileNormalizedNameInformation,           // 48
		FileNetworkPhysicalNameInformation,      // 49
		FileIdGlobalTxDirectoryInformation,      // 50
		FileIsRemoteDeviceInformation,           // 51
		FileUnusedInformation,                   // 52
		FileNumaNodeInformation,                 // 53
		FileStandardLinkInformation,             // 54
		FileRemoteProtocolInformation,           // 55

		//
		//  These are special versions of these operations (defined earlier)
		//  which can be used by kernel mode drivers only to bypass security
		//  access checks for Rename and HardLink operations.  These operations
		//  are only recognized by the IOManager, a file system should never
		//  receive these.
		//
		FileRenameInformationBypassAccessCheck,  // 56
		FileLinkInformationBypassAccessCheck,    // 57
		FileVolumeNameInformation,               // 58
		FileIdInformation,                       // 59
		FileIdExtdDirectoryInformation,          // 60
		FileReplaceCompletionInformation,        // 61
		FileHardLinkFullIdInformation,           // 62
		FileMaximumInformation
	} NTFILE_INFORMATION_CLASS, * PNTFILE_INFORMATION_CLASS;

	typedef enum _SECTION_INHERIT {
		ViewShare = 1,
		ViewUnmap = 2
	} SECTION_INHERIT;

	typedef struct _FILE_STANDARD_INFORMATION {
		LARGE_INTEGER AllocationSize;
		LARGE_INTEGER EndOfFile;
		ULONG NumberOfLinks;
		BOOLEAN DeletePending;
		BOOLEAN Directory;
} FILE_STANDARD_INFORMATION, * PFILE_STANDARD_INFORMATION;

	typedef struct _FILE_END_OF_FILE_INFORMATION {
		LARGE_INTEGER EndOfFile;
	} FILE_END_OF_FILE_INFORMATION, * PFILE_END_OF_FILE_INFORMATION;


#if (_WIN32_WINNT >= 0x0602)

#define FILE_ATTRIBUTE_INTEGRITY_STREAM     0x00008000  

#endif

#define FILE_ATTRIBUTE_VIRTUAL              0x00010000  

#if (_WIN32_WINNT < 0x0602)

#define FILE_ATTRIBUTE_VALID_FLAGS          0x00007fb7
#define FILE_ATTRIBUTE_VALID_SET_FLAGS      0x000031a7

#else

#define FILE_ATTRIBUTE_NO_SCRUB_DATA        0x00020000  

#define FILE_ATTRIBUTE_VALID_FLAGS          0x0002ffb7
#define FILE_ATTRIBUTE_VALID_SET_FLAGS      0x000231a7

#endif

#endif //#ifndef _NTDDK_


#ifdef _NTDDK_
	#define PAGE_NOACCESS          0x01     
	#define PAGE_READONLY          0x02     
	#define PAGE_READWRITE         0x04     
	#define PAGE_WRITECOPY         0x08     
	#pragma region Desktop Family           
	#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP) 
	#define PAGE_EXECUTE           0x10     
	#define PAGE_EXECUTE_READ      0x20     
	#define PAGE_EXECUTE_READWRITE 0x40     
	#define PAGE_EXECUTE_WRITECOPY 0x80     
	#endif /* WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP) */ 
	#pragma endregion                       
	#define PAGE_GUARD            0x100     
	#define PAGE_NOCACHE          0x200     
	#define PAGE_WRITECOMBINE     0x400     
	#define PAGE_REVERT_TO_FILE_MAP     0x80000000     
	#define MEM_COMMIT                  0x1000      
	#define MEM_RESERVE                 0x2000      
	#define MEM_DECOMMIT                0x4000      
	#define MEM_RELEASE                 0x8000      
	#define MEM_FREE                    0x10000     
	#define MEM_PRIVATE                 0x20000     
	#define MEM_MAPPED                  0x40000     
	#define MEM_RESET                   0x80000     
	#define MEM_TOP_DOWN                0x100000    
	#define MEM_WRITE_WATCH             0x200000    
	#define MEM_PHYSICAL                0x400000    
	#define MEM_ROTATE                  0x800000    
	#define MEM_DIFFERENT_IMAGE_BASE_OK 0x800000    
	#define MEM_RESET_UNDO              0x1000000   
	#define MEM_LARGE_PAGES             0x20000000  
	#define MEM_4MB_PAGES               0x80000000  
	#define SEC_FILE           0x800000     
	#define SEC_IMAGE         0x1000000     
	#define SEC_PROTECTED_IMAGE  0x2000000  
	#define SEC_RESERVE       0x4000000     
	#define SEC_COMMIT        0x8000000     
	#define SEC_NOCACHE      0x10000000     
	#define SEC_WRITECOMBINE 0x40000000     
	#define SEC_LARGE_PAGES  0x80000000     
	#define SEC_IMAGE_NO_EXECUTE (SEC_IMAGE | SEC_NOCACHE)     
	#define MEM_IMAGE         SEC_IMAGE     
	#define WRITE_WATCH_FLAG_RESET  0x01     
	#define MEM_UNMAP_WITH_TRANSIENT_BOOST  0x01     

	#define FILE_MAP_WRITE            SECTION_MAP_WRITE
	#define FILE_MAP_READ             SECTION_MAP_READ
	#define FILE_MAP_COPY       0x00000001
	#define FILE_MAP_EXECUTE    SECTION_MAP_EXECUTE_EXPLICIT    // not included in FILE_MAP_ALL_ACCESS

	#define INVALID_HANDLE_VALUE ((HANDLE)(LONG_PTR)-1)

typedef struct _SECURITY_ATTRIBUTES {
	DWORD nLength;
	LPVOID lpSecurityDescriptor;
	BOOL bInheritHandle;
} SECURITY_ATTRIBUTES, * PSECURITY_ATTRIBUTES, * LPSECURITY_ATTRIBUTES;

#define CREATE_NEW          1
#define CREATE_ALWAYS       2
#define OPEN_EXISTING       3
#define OPEN_ALWAYS         4
#define TRUNCATE_EXISTING   5

//
//  These are flags supported through CreateFile (W7) and CreateFile2 (W8 and beyond)
//

#define FILE_FLAG_WRITE_THROUGH         0x80000000
#define FILE_FLAG_OVERLAPPED            0x40000000
#define FILE_FLAG_NO_BUFFERING          0x20000000
#define FILE_FLAG_RANDOM_ACCESS         0x10000000
#define FILE_FLAG_SEQUENTIAL_SCAN       0x08000000
#define FILE_FLAG_DELETE_ON_CLOSE       0x04000000
#define FILE_FLAG_BACKUP_SEMANTICS      0x02000000
#define FILE_FLAG_POSIX_SEMANTICS       0x01000000
#define FILE_FLAG_SESSION_AWARE         0x00800000
#define FILE_FLAG_OPEN_REPARSE_POINT    0x00200000
#define FILE_FLAG_OPEN_NO_RECALL        0x00100000
#define FILE_FLAG_FIRST_PIPE_INSTANCE   0x00080000

#pragma warning( push )
#pragma warning( disable : 4201 )
typedef struct _OVERLAPPED {
	ULONG_PTR Internal;
	ULONG_PTR InternalHigh;
	union {
		struct {
				DWORD Offset;
				DWORD OffsetHigh;
			};
		PVOID Pointer;
		};

	HANDLE  hEvent;
} OVERLAPPED, *LPOVERLAPPED;
#pragma warning( pop )

#endif //#ifdef _NTDDK_

#ifdef NTZW
#undef NTZW
#endif

#define NTZW(a) Nt##a

#include <NtDllFuncInc.h>

#undef NTZW
#define NTZW(a) Zw##a
#include <NtDllFuncInc.h>
#undef NTZW


#ifdef __cplusplus
}
#endif
