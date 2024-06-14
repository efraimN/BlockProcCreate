#include "Precompiled.h"

#include <WppIncludes.h>

#include "ParsePelmp.h"
#include <IUserKernelUtils.h>


UserKernelUtilsLib::IParsePe* UserKernelUtilsLib::IParsePe::GetInstance()
{
	static ParsePeImp Singletone;
	return &Singletone;
}


/*
Returns:
-1  if something wrong with the parsing
0   if everything is ok
>=1 If any of the callbacks has stopped the enumeration

*/
INT UserKernelUtilsLib::ParsePeImp::ParsePeMain(
	_In_ HANDLE ProcHandle,
	_In_ PVOID ModuleBase,
	_In_ ParsePeCallBack* CallFunction,
	_In_ BOOLEAN VerBose,
	_In_ BOOLEAN IgnoreMagics
)
{
	INT RetVal = -1;
	UINT_PTR uiBaseAddress = 0;
	PIMAGE_DOS_HEADER DosHeader = NULL;

	DllArchitecture ImageArchitecture  = DLL_X64;

	PVOID NtHeader = NULL;

	PIMAGE_DATA_DIRECTORY ImageDataDirectory = NULL;
	DWORD ImageDataDirectoryCount = 0;
	DWORD DataDirectoryOffset = 0;
	DWORD DataDirectorySize = 0;

	PCHAR DllNameAddressRemote = NULL;
	PIMAGE_EXPORT_DIRECTORY ExportTable = NULL;
	DWORD ExportTableVirtualAddress = 0;
	DWORD ExportTableSize = 0;
	static BOOLEAN DllCallbackDone = FALSE;

	// Locals for PE header members
	WORD e_magic;
	DWORD e_lfanew;
	WORD Magic;
	DWORD NtHeaderSignature;

	UINT EnumSectionsCallBackRetVal = 1;

	if (!CallFunction)
	{
		LOG_OUT(DBG_DEBUG, "Invalid CallFunction is zero");
		return -1;
	}

	CallFunction->m_ProcHandle = ProcHandle;

	__try
	{
		// Base address
		uiBaseAddress = (UINT_PTR)ModuleBase;

		// Check IMAGE_DOS_HEADER memory access
		ProbeForRead(
			(LPVOID)uiBaseAddress,
			sizeof(IMAGE_DOS_HEADER),
			__alignof(BYTE));

		DosHeader = (PIMAGE_DOS_HEADER)uiBaseAddress;

		// Check DOS signature
		ReadMemory((PUCHAR)DosHeader + FIELD_OFFSET(IMAGE_DOS_HEADER, e_magic), &e_magic, sizeof(WORD), ProcHandle);
		if (e_magic != IMAGE_DOS_SIGNATURE)
		{
			if (VerBose)
			{
				LOG_OUT(DBG_DEBUG, "Invalid DOS signature: 0x%02x.", (USHORT)e_magic);
			}
			if (!IgnoreMagics)
			{
				goto Leave;
			}
		}

		// Get offset to NT header
		ReadMemory((PUCHAR)DosHeader + FIELD_OFFSET(IMAGE_DOS_HEADER, e_lfanew), &e_lfanew, sizeof(DWORD), ProcHandle);
		if (e_lfanew >= (256 * 1024 * 1024))
		{
			if (VerBose)
			{
				LOG_OUT(DBG_DEBUG, "NT header offset is larger than 256MB.");
			}
			goto Leave;
		}

		// Get Signature, which is at the same offset and the same size in both IMAGE_NT_HEADERS32 | IMAGE_NT_HEADERS64.
		/*
		typedef struct _IMAGE_NT_HEADERS64 {
			DWORD Signature;
			IMAGE_FILE_HEADER FileHeader;
			IMAGE_OPTIONAL_HEADER64 OptionalHeader;
		} IMAGE_NT_HEADERS64, *PIMAGE_NT_HEADERS64;

		typedef struct _IMAGE_NT_HEADERS {
			DWORD Signature;
			IMAGE_FILE_HEADER FileHeader;
			IMAGE_OPTIONAL_HEADER32 OptionalHeader;
		} IMAGE_NT_HEADERS32, *PIMAGE_NT_HEADERS32;
		*/
		NtHeader = (LPVOID)((PCHAR)DosHeader + e_lfanew);
		ProbeForRead(
			NtHeader,
			sizeof(DWORD),
			__alignof(BYTE)
		);
		ReadMemory(NtHeader, &NtHeaderSignature, sizeof(DWORD), ProcHandle);

		// Verify NT header signature
		if (NtHeaderSignature != IMAGE_NT_SIGNATURE)
		{
			if (VerBose)
			{
				LOG_OUT(DBG_DEBUG, "Invalid NT header signature.");
			}
			if (!IgnoreMagics)
			{
				goto Leave;
			}
		}

		/*
			Get PE's architecture, which is at the same offset and the same size
			in both IMAGE_OPTIONAL_HEADER32 | IMAGE_OPTIONAL_HEADER64.
		*/
		ProbeForRead(
			(PCHAR)NtHeader + FIELD_OFFSET(IMAGE_NT_HEADERS, OptionalHeader) + FIELD_OFFSET(IMAGE_OPTIONAL_HEADER32, Magic),
			sizeof(WORD),
			__alignof(BYTE)
		);
		ReadMemory(
			(PCHAR)NtHeader + FIELD_OFFSET(IMAGE_NT_HEADERS, OptionalHeader) + FIELD_OFFSET(IMAGE_OPTIONAL_HEADER32, Magic),
			&Magic,
			sizeof(WORD),
			ProcHandle);

		if (Magic == 0x010B) // PE32
		{
			ImageArchitecture = DLL_X86;
		}
		else if (Magic == 0x020B) // PE64 
		{
			ImageArchitecture = DLL_X64;
		}
		else
		{
			if (VerBose)
			{
				LOG_OUT(DBG_DEBUG, "The IMAGE_OPTIONAL_HEADER.Magic is not one of the allowed values: 0x%x.", Magic);
			}
			goto Leave;
		}

		// Get export data directory
		if (ImageArchitecture == DLL_X86)
		{
			ProbeForRead(
				(PCHAR)NtHeader + FIELD_OFFSET(IMAGE_NT_HEADERS32, OptionalHeader),
				sizeof(IMAGE_OPTIONAL_HEADER32),
				__alignof(BYTE));
			ImageDataDirectory = (PIMAGE_DATA_DIRECTORY)((PCHAR)NtHeader + FIELD_OFFSET(IMAGE_NT_HEADERS32, OptionalHeader) + FIELD_OFFSET(IMAGE_OPTIONAL_HEADER32, DataDirectory));
			ReadMemory(((PCHAR)NtHeader + FIELD_OFFSET(IMAGE_NT_HEADERS32, OptionalHeader) + FIELD_OFFSET(IMAGE_OPTIONAL_HEADER32, NumberOfRvaAndSizes)), &ImageDataDirectoryCount, sizeof(DWORD), ProcHandle);
		}
		else
		{
			ProbeForRead(
				(PCHAR)NtHeader + FIELD_OFFSET(IMAGE_NT_HEADERS64, OptionalHeader),
				sizeof(IMAGE_OPTIONAL_HEADER64),
				__alignof(BYTE));
			ImageDataDirectory = (PIMAGE_DATA_DIRECTORY)((PCHAR)NtHeader + FIELD_OFFSET(IMAGE_NT_HEADERS64, OptionalHeader) + FIELD_OFFSET(IMAGE_OPTIONAL_HEADER64, DataDirectory));
			ReadMemory(((PCHAR)NtHeader + FIELD_OFFSET(IMAGE_NT_HEADERS64, OptionalHeader) + FIELD_OFFSET(IMAGE_OPTIONAL_HEADER64, NumberOfRvaAndSizes)), &ImageDataDirectoryCount, sizeof(DWORD), ProcHandle);
		}

		if (ImageDataDirectoryCount > IMAGE_NUMBEROF_DIRECTORY_ENTRIES)
		{
			if (VerBose)
			{
				LOG_OUT(DBG_DEBUG, "DataDirectoryCount > IMAGE_NUMBEROF_DIRECTORY_ENTRIES: %d.", ImageDataDirectoryCount);
			}
			goto Leave;
		}

		// Verify Export Data Directory exists
		if (ImageDataDirectoryCount <= IMAGE_DIRECTORY_ENTRY_EXPORT)
		{
			if (VerBose)
			{
			}
			goto Leave;
		}

		if (!GetPeDirectorySizeOffset(
			ProcHandle,
			(PCHAR)ImageDataDirectory,
			IMAGE_DIRECTORY_ENTRY_EXPORT,
			&ExportTableSize,
			&ExportTableVirtualAddress
		))
		{
			goto Leave;
		}

		ExportTable = (PIMAGE_EXPORT_DIRECTORY)((PCHAR)uiBaseAddress + ExportTableVirtualAddress);


		// Check export directory size, which should be at least IMAGE_EXPORT_DIRECTORY bytes
		if (ExportTableSize < sizeof(IMAGE_EXPORT_DIRECTORY))
		{
			if (VerBose)
			{
				LOG_OUT(DBG_ERROR, "Too small size for the export data directory.");
			}
			goto Leave;
		}
		else
		{
			// Check access to the export directory
			ProbeForRead(
				(LPVOID)ExportTable,
				ExportTableSize,
				__alignof(BYTE));

			// Get the remote address of the DLL name
			DWORD Name;
			ReadMemory((PCHAR)ExportTable + FIELD_OFFSET(IMAGE_EXPORT_DIRECTORY, Name), &Name, sizeof(DWORD), ProcHandle);

			DWORD ExportTableMaxVirtualAddress = ExportTableVirtualAddress + ExportTableSize;
			if (Name < ExportTableVirtualAddress ||
				Name >= ExportTableMaxVirtualAddress)
			{
				if (VerBose)
				{
					LOG_OUT(DBG_ERROR, "DllName address outside of Export data directory.");
				}
				goto Leave;
			}

			DllNameAddressRemote = (PCHAR)((PCHAR)uiBaseAddress + Name);
			if (CallFunction)
			{
				RetVal = CallFunction->GetModuleNameAndTypeCallback(DllNameAddressRemote, ImageArchitecture);
				if (RetVal != 0)
				{
					if (VerBose)
					{
						LOG_OUT(DBG_ERROR, "Failed VerifyDllCallback dll.");
					}
					if (RetVal < 0)
					{
						RetVal *= -1;
					}
					goto Leave;
				}
			}

		}

		if (CallFunction->m_DoEnumImageDirectoryCallBack)
		{
			for (DWORD DirectoryIndex = 0; DirectoryIndex < ImageDataDirectoryCount; ++DirectoryIndex)
			{
				ReadMemory(
					(PCHAR)ImageDataDirectory + DirectoryIndex * sizeof(IMAGE_DATA_DIRECTORY) + FIELD_OFFSET(IMAGE_DATA_DIRECTORY, VirtualAddress),
					&DataDirectoryOffset,
					sizeof(DWORD),
					ProcHandle);
				ReadMemory(
					(PCHAR)ImageDataDirectory + DirectoryIndex * sizeof(IMAGE_DATA_DIRECTORY) + FIELD_OFFSET(IMAGE_DATA_DIRECTORY, Size),
					&DataDirectorySize,
					sizeof(DWORD),
					ProcHandle);
				RetVal = CallFunction->EnumImageDirectoryCallback(DirectoryIndex, uiBaseAddress, ImageDataDirectory);
				if (RetVal != 0)
				{
					if (VerBose)
					{
						LOG_OUT(DBG_ERROR, "Failed VerifyDllCallback dll.");
					}
					if (RetVal < 0)
					{
						RetVal *= -1;
					}
					goto Leave;
				}
			}
		}

		if (CallFunction->m_DoEnumSectionsCallBack)
		{
			PIMAGE_SECTION_HEADER pSectionHeader;
			IMAGE_SECTION_HEADER SectionHeader;
			USHORT SectionsCount;
			USHORT SizeOfOptionalHeader;

			/*
				IMAGE_NT_HEADERS32.FileHeader.SizeOfOptionalHeader and IMAGE_NT_HEADERS32.FileHeader.SizeOfOptionalHeader
				are at the same offset in both 32 and 64

				IMAGE_NT_HEADERS32.OptionalHeader and IMAGE_NT_HEADERS64.OptionalHeader
				are at the same offset in both 32 and 64

				The probe for IMAGE_NT_HEADERS32 | IMAGE_NT_HEADERS64 was done before
			*/

			ReadMemory(
				(PCHAR)NtHeader + FIELD_OFFSET(IMAGE_NT_HEADERS32, FileHeader) + FIELD_OFFSET(IMAGE_FILE_HEADER, SizeOfOptionalHeader),
				&SizeOfOptionalHeader,
				sizeof(USHORT),
				ProcHandle);

			ReadMemory(
				(PCHAR)NtHeader + FIELD_OFFSET(IMAGE_NT_HEADERS32, FileHeader) + FIELD_OFFSET(IMAGE_FILE_HEADER, NumberOfSections),
				&SectionsCount,
				sizeof(USHORT),
				ProcHandle);

			pSectionHeader = (PIMAGE_SECTION_HEADER)((ULONG_PTR)((PCHAR)NtHeader + FIELD_OFFSET(IMAGE_NT_HEADERS32, OptionalHeader))
				+ SizeOfOptionalHeader);

// 			LOG_OUT(DBG_INFO, "Section header at: 0x%p - number of sections: %Id.", pSectionHeader, SectionsCount);

			ProbeForRead(
				(LPVOID)pSectionHeader,
				SectionsCount * sizeof(IMAGE_SECTION_HEADER),
				__alignof(BYTE));

			while (SectionsCount--)
			{
				ReadMemory(
					pSectionHeader,
					&SectionHeader,
					sizeof(IMAGE_SECTION_HEADER),
					ProcHandle);

				EnumSectionsCallBackRetVal = CallFunction->EnumSectionsCallBack(&SectionHeader);
				if (EnumSectionsCallBackRetVal > 0)
				{
					RetVal = EnumSectionsCallBackRetVal;
					if (VerBose)
					{
						LOG_OUT(DBG_ERROR, "Failed EnumSectionsCallBack dll.");
					}
					if (RetVal < 0)
					{
						RetVal *= -1;
					}
					goto Leave;
				}

				// get the VA of the next section
				pSectionHeader = (PIMAGE_SECTION_HEADER)((PUCHAR)pSectionHeader + sizeof(IMAGE_SECTION_HEADER));
			}
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
// 		LOG_OUT(DBG_ERROR, "ParsePeMain catch");
		goto Leave;
	}

	RetVal = 0;
Leave:
	return RetVal;
}

BOOLEAN UserKernelUtilsLib::ParsePeImp::GetPeDirectorySizeOffset(
	HANDLE ProcHandle,
	PCHAR ImageDataDirectory,
	DWORD Index,
	PDWORD Size,
	PDWORD Offset
)
{
	return
	[&]()->BOOLEAN
	{
		BOOLEAN RetVal = FALSE;
		__try
		{
			ReadMemory(
				ImageDataDirectory + Index * sizeof(IMAGE_DATA_DIRECTORY) + FIELD_OFFSET(IMAGE_DATA_DIRECTORY, Size),
				Size,
				sizeof(DWORD),
				ProcHandle);

			ReadMemory(
				ImageDataDirectory + Index * sizeof(IMAGE_DATA_DIRECTORY) + FIELD_OFFSET(IMAGE_DATA_DIRECTORY, VirtualAddress),
				Offset,
				sizeof(DWORD),
				ProcHandle);

		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
		}

		RetVal = TRUE;
// 	Leave:
		return RetVal;
	}();
}

PCHAR
UserKernelUtilsLib::ParsePeImp::EnumExportedFunctions(
	HANDLE ProcHandle,
	UINT_PTR Base,
	pfnMemAllocator Allocator,
	DWORD Size,
	DWORD Ofset
)
{
	LONG RequiredStrLength = 0;
	PCHAR StringData = NULL;
	PCHAR RetVal = NULL;

	auto Enum =
	[&]()->VOID
	{
		__try
		{

			DWORD NumberOfFunctions;

			PULONG ArrayOfFunctionNames = 0;

			DWORD AddressOfFunctions;
			DWORD AddressOfNames;
			DWORD AddressOfNameOrdinals;
			PIMAGE_EXPORT_DIRECTORY ExportTable = NULL;

			DWORD TableMaxVirtualAddress;

			LONG StringLength;

			ExportTable = (PIMAGE_EXPORT_DIRECTORY)((PCHAR)Base + Ofset);

			// Check access to the export directory
			ProbeForRead(
				(LPVOID)ExportTable,
				Size,
				__alignof(BYTE));

			// Read the number of functions
			ReadMemory((PCHAR)ExportTable + FIELD_OFFSET(IMAGE_EXPORT_DIRECTORY, NumberOfFunctions), &NumberOfFunctions, sizeof(DWORD), ProcHandle);

			// Read the number of function names
			DWORD NumberOfNames;
			ReadMemory((PCHAR)ExportTable + FIELD_OFFSET(IMAGE_EXPORT_DIRECTORY, NumberOfNames), &NumberOfNames, sizeof(DWORD), ProcHandle);

			// Read the necessary RVAs
			ReadMemory((PCHAR)ExportTable + FIELD_OFFSET(IMAGE_EXPORT_DIRECTORY, AddressOfFunctions), &AddressOfFunctions, sizeof(DWORD), ProcHandle);

			ReadMemory((PCHAR)ExportTable + FIELD_OFFSET(IMAGE_EXPORT_DIRECTORY, AddressOfNames), &AddressOfNames, sizeof(DWORD), ProcHandle);

			ReadMemory((PCHAR)ExportTable + FIELD_OFFSET(IMAGE_EXPORT_DIRECTORY, AddressOfNameOrdinals), &AddressOfNameOrdinals, sizeof(DWORD), ProcHandle);

			TableMaxVirtualAddress = Ofset + Size;

			// Check if addresses point into the export data directory.
			// Also if all address in range we won't need to probe for read again as it was already done on this range.
			if (
				0
				|| (AddressOfFunctions < Ofset)
				|| (AddressOfFunctions >= (TableMaxVirtualAddress))
				|| (AddressOfNames < Ofset)
				|| (AddressOfNames >= (TableMaxVirtualAddress))
				|| (AddressOfNameOrdinals < Ofset)
				|| (AddressOfNameOrdinals >= (TableMaxVirtualAddress))
				|| ((AddressOfFunctions + sizeof(DWORD) * NumberOfFunctions) >= (TableMaxVirtualAddress))
				|| ((AddressOfNames + sizeof(DWORD) * NumberOfNames) >= (TableMaxVirtualAddress))
				|| ((AddressOfNameOrdinals + sizeof(WORD) * NumberOfNames) >= (TableMaxVirtualAddress))
				)
			{
// 				LOG_OUT(DBG_INFO, "The AddressOfNames / AddressOfFunctions / AddressOfNameOrdinals point outside of the export data directory.");
				goto Leave;
			}

			// Get the actual addresses (not the RVAs) of exported functions
			ArrayOfFunctionNames = (PULONG)((PUCHAR)Base + AddressOfNames);

			// loop through named exported function
			for (ULONG i = 0; i < NumberOfNames; i++)
			{
				PCHAR FunctionName = NULL;
				ULONG FunctionNameRVA;

				// get the function name and the function ordinal
				ReadMemory((PCHAR)ArrayOfFunctionNames + (i * sizeof(ULONG)), &FunctionNameRVA, sizeof(ULONG), ProcHandle);
				FunctionName = (PCHAR)((PUCHAR)Base + FunctionNameRVA);

				StringLength = UserKernelUtilsLib::IUtilsInt::GetInstance()->StrLenSafe(FunctionName, MAX_PATH, ProcHandle);
				if (StringData)
				{

					__try
					{
						ReadMemory(
							FunctionName,
							StringData,
							StringLength,
							ProcHandle);
					}
					__except (EXCEPTION_EXECUTE_HANDLER)
					{
						continue;
					}
					StringData = &StringData[StringLength];
					StringData[0] = ',';
					StringData++;
				}
				else
				{
					RequiredStrLength += StringLength + 1;
				}
			}
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
		}

		Leave:
			return;
	};

	Enum();
	if (RequiredStrLength)
	{
		RetVal = Allocator(RequiredStrLength+1);
		if (RetVal)
		{
			RtlZeroMemory(RetVal, RequiredStrLength+1);
			StringData = RetVal;
			Enum();
		}
	}
	return RetVal;
}

PCHAR
UserKernelUtilsLib::ParsePeImp::EnumImportedFunctions(
	HANDLE ProcHandle,
	UINT_PTR Base,
	DllArchitecture ImageArchitecture,
	pfnMemAllocator Allocator,
	DWORD Size,
	DWORD Offset
)
{
	LONG RequiredStrLength = 0;
	PCHAR StringData = NULL;
	PCHAR RetVal = NULL;

	auto Enum =
	[&]()->VOID
	{
		__try
		{
			PIMAGE_IMPORT_DESCRIPTOR pImportTableDescriptor = NULL;
			PCHAR pRemoteLibName = NULL;
			DWORD NameRVA;
			ULONGLONG ILT64 = 0;
			ULONG ILT32 = 0;
			ULONG Length;
			PVOID ILT;

			LONG StringLength;

			pImportTableDescriptor = (PIMAGE_IMPORT_DESCRIPTOR)((PCHAR)Base + Offset);

			if (Size < sizeof(IMAGE_IMPORT_DESCRIPTOR))
			{
				goto Leave;
			}

			// Check access to the import directory
			ProbeForRead(
				(LPVOID)pImportTableDescriptor,
				Size,
				__alignof(BYTE));

			if (ImageArchitecture == DLL_X86)
			{
				Length = sizeof(ULONG);
				ILT = &ILT32;
			}
			else
			{
				Length = sizeof(ULONGLONG);
				ILT = &ILT64;
			}

			for(;; pImportTableDescriptor++ )//advance to next IMAGE_IMPORT_DESCRIPTOR	
			{
				__try
				{
					ProbeForRead(
						(LPVOID)pImportTableDescriptor,
						sizeof(IMAGE_IMPORT_DESCRIPTOR),
						__alignof(BYTE));
				}
				__except (EXCEPTION_EXECUTE_HANDLER)
				{
					break;
				}

				// Get the remote address of the DLL name
				ReadMemory((PCHAR)pImportTableDescriptor + FIELD_OFFSET(IMAGE_IMPORT_DESCRIPTOR, Name), &NameRVA, sizeof(DWORD), ProcHandle);
				// Walk until you reached an empty IMAGE_IMPORT_DESCRIPTOR
				if (!NameRVA)
				{
					break;
				}
				pRemoteLibName = (PCHAR)((PCHAR)Base + NameRVA);

				StringLength = UserKernelUtilsLib::IUtilsInt::GetInstance()->StrLenSafe(pRemoteLibName, MAX_PATH, ProcHandle);
				if (StringData)
				{
					// Read the dll name from remote process
					__try
					{
						ReadMemory(
							pRemoteLibName,
							StringData,
							StringLength,
							ProcHandle);
					}
					__except (EXCEPTION_EXECUTE_HANDLER)
					{
						continue;
					}
					StringData = &StringData[StringLength];
					StringData[0] = ';';
					StringData++;
				}
				else
				{
					RequiredStrLength += StringLength + 1;
				}
			
				ULONG OriginalFirstThunk;
				BOOL ByName;
				ULONG ILTRva;
				// Get the remote address of the original IAT
				ReadMemory((PCHAR)pImportTableDescriptor + FIELD_OFFSET(IMAGE_IMPORT_DESCRIPTOR, OriginalFirstThunk), &OriginalFirstThunk, sizeof(DWORD), ProcHandle);
				if (!OriginalFirstThunk)
				{
					continue;
				}

				// Now get all the imported functions RVA's
				PIMAGE_IMPORT_BY_NAME pImportByName;
				PVOID pRemoteILT;
				UINT h;
				h = 0;
				pRemoteILT = (PVOID)((PCHAR)Base + OriginalFirstThunk);
				// Walk until you reached an empty entry
				for (;; pRemoteILT = ((PCHAR)pRemoteILT + Length), h++)
				{
					__try
					{
						ReadMemory(pRemoteILT, ILT, Length, ProcHandle);
					}
					__except (EXCEPTION_EXECUTE_HANDLER)
					{
						break;
					}

					if (ImageArchitecture == DLL_X86)
					{
						if (!ILT32)
						{
							break;
						}
						ByName = !IMAGE_SNAP_BY_ORDINAL32(ILT32);
						ILTRva = ILT32;
					}
					else
					{
						if (!ILT64)
						{
							break;
						}
						ByName = !IMAGE_SNAP_BY_ORDINAL64(ILT64);
						ILTRva = (ULONG)ILT64;
					}

					if (ByName)
					{
						// it means is imported by name
						pImportByName = (PIMAGE_IMPORT_BY_NAME)((PCHAR)Base + ILTRva);
						StringLength = UserKernelUtilsLib::IUtilsInt::GetInstance()->StrLenSafe(pImportByName->Name, MAX_PATH, ProcHandle);
						if (StringData)
						{
								__try
								{
									ReadMemory(
										pImportByName->Name,
										StringData,
										StringLength,
										ProcHandle);
								}
								__except (EXCEPTION_EXECUTE_HANDLER)
								{
									continue;
								}
								StringData = &StringData[StringLength];
								StringData[0] = ',';
								StringData++;
						}
						else
						{
							RequiredStrLength += StringLength + 1;
						}
					}
// 					else
// 					{
// 						if (StringData) {
// 							RequiredStrLength = RequiredStrLength;
// 						}
// 					}
				}

			}

		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
		}

	Leave:
		return;
	};

	Enum();
	if (RequiredStrLength)
	{
		RetVal = Allocator(RequiredStrLength+1);
		if (RetVal)
		{
			RtlZeroMemory(RetVal, RequiredStrLength+1);
			StringData = RetVal;
			Enum();
		}
	}

return RetVal;
}

PVOID
UserKernelUtilsLib::ParsePeImp::InMemoryGetProcAddress(
	PVOID ModuleBase,
	LPCCH pFunctionName,
	HANDLE ProcHandle
)
{
	PVOID pFunctionAddress = NULL;
	WORD e_magic;
	DWORD e_lfanew;
	DWORD NtHeaderSignature;
	WORD Magic;
	BOOLEAN IsPE32;
	PIMAGE_DATA_DIRECTORY DataDirectory = NULL;
	DWORD DataDirectoryCount = 0;
	DWORD ExportTableVirtualAddress = 0;
	DWORD ExportTableSize = 0;
	PIMAGE_EXPORT_DIRECTORY ExportTable = NULL;

	PULONG ArrayOfFunctionAddresses = 0;
	PULONG ArrayOfFunctionNames = 0;
	PUSHORT ArrayOfFunctionNameOrdinals = 0;

	ULONG FunctionNameIndex;
	ULONG FunctionAddressRVA;
	PCHAR FunctionName = NULL;
	USHORT FunctionOrdinal = 0;
	PULONG_PTR FunctionAddress = 0;

	__try
	{
		ProbeForRead(
			ModuleBase,
			sizeof(IMAGE_DOS_HEADER),
			__alignof(BYTE));

		// get and check e_magic
		PIMAGE_DOS_HEADER DosHeader = (PIMAGE_DOS_HEADER)ModuleBase;
		ProbeForRead(
			(PUCHAR)DosHeader + FIELD_OFFSET(IMAGE_DOS_HEADER, e_magic),
			sizeof(WORD),
			__alignof(BYTE));
		ReadMemory((PUCHAR)DosHeader + FIELD_OFFSET(IMAGE_DOS_HEADER, e_magic), &e_magic, sizeof(WORD), ProcHandle);
		if (e_magic != IMAGE_DOS_SIGNATURE)
		{
			goto Leave;
		}

		// get and check e_lfanew
		ProbeForRead(
			(PUCHAR)DosHeader + FIELD_OFFSET(IMAGE_DOS_HEADER, e_lfanew),
			sizeof(DWORD),
			__alignof(BYTE));
		ReadMemory((PUCHAR)DosHeader + FIELD_OFFSET(IMAGE_DOS_HEADER, e_lfanew), &e_lfanew, sizeof(DWORD), ProcHandle);
		if (e_lfanew >= (256 * 1024 * 1024))
		{
			goto Leave;
		}

		// get and check NtHeader
		PVOID NtHeader = (LPVOID)((PCHAR)DosHeader + e_lfanew);
		ProbeForRead(
			NtHeader,
			sizeof(DWORD),
			__alignof(BYTE)
		);
		ReadMemory(NtHeader, &NtHeaderSignature, sizeof(DWORD), ProcHandle);
		if (NtHeaderSignature != IMAGE_NT_SIGNATURE)
		{
			goto Leave;
		}

		/*
			Get PE's architecture, which is at the same offset and the same size
			in both IMAGE_OPTIONAL_HEADER32 | IMAGE_OPTIONAL_HEADER64.
		*/
		ProbeForRead(
			(PCHAR)NtHeader + FIELD_OFFSET(IMAGE_NT_HEADERS, OptionalHeader) + FIELD_OFFSET(IMAGE_OPTIONAL_HEADER32, Magic),
			sizeof(WORD),
			__alignof(BYTE)
		);
		ReadMemory(
			(PCHAR)NtHeader + FIELD_OFFSET(IMAGE_NT_HEADERS, OptionalHeader) + FIELD_OFFSET(IMAGE_OPTIONAL_HEADER32, Magic),
			&Magic,
			sizeof(WORD),
			ProcHandle);

		if (Magic == 0x010B) // PE32
		{
			IsPE32 = TRUE;// 32 bits
		}
		else if (Magic == 0x020B) // PE64 
		{
			IsPE32 = FALSE; //64 bits
		}
		else
		{
			goto Leave;
		}

		// Get data directory
		if (IsPE32 == TRUE)
		{
			ProbeForRead(
				(PCHAR)NtHeader + FIELD_OFFSET(IMAGE_NT_HEADERS32, OptionalHeader),
				sizeof(IMAGE_OPTIONAL_HEADER32),
				__alignof(BYTE));
			DataDirectory = (PIMAGE_DATA_DIRECTORY)((PCHAR)NtHeader + FIELD_OFFSET(IMAGE_NT_HEADERS32, OptionalHeader) + FIELD_OFFSET(IMAGE_OPTIONAL_HEADER32, DataDirectory));
			ReadMemory(((PCHAR)NtHeader + FIELD_OFFSET(IMAGE_NT_HEADERS32, OptionalHeader) + FIELD_OFFSET(IMAGE_OPTIONAL_HEADER32, NumberOfRvaAndSizes)), &DataDirectoryCount, sizeof(DWORD), ProcHandle);
		}
		else
		{
			ProbeForRead(
				(PCHAR)NtHeader + FIELD_OFFSET(IMAGE_NT_HEADERS64, OptionalHeader),
				sizeof(IMAGE_OPTIONAL_HEADER64),
				__alignof(BYTE));
			DataDirectory = (PIMAGE_DATA_DIRECTORY)((PCHAR)NtHeader + FIELD_OFFSET(IMAGE_NT_HEADERS64, OptionalHeader) + FIELD_OFFSET(IMAGE_OPTIONAL_HEADER64, DataDirectory));
			ReadMemory(
				((PCHAR)NtHeader + FIELD_OFFSET(IMAGE_NT_HEADERS64, OptionalHeader) + FIELD_OFFSET(IMAGE_OPTIONAL_HEADER64, NumberOfRvaAndSizes)),
				&DataDirectoryCount,
				sizeof(DWORD),
				ProcHandle);
		}

		if (DataDirectoryCount <= IMAGE_DIRECTORY_ENTRY_EXPORT)
		{
			goto Leave;
		}
		ReadMemory(
			(PCHAR)DataDirectory + IMAGE_DIRECTORY_ENTRY_EXPORT * sizeof(IMAGE_DATA_DIRECTORY) + FIELD_OFFSET(IMAGE_DATA_DIRECTORY, VirtualAddress),
			&ExportTableVirtualAddress,
			sizeof(DWORD),
			ProcHandle);
		ReadMemory(
			(PCHAR)DataDirectory + IMAGE_DIRECTORY_ENTRY_EXPORT * sizeof(IMAGE_DATA_DIRECTORY) + FIELD_OFFSET(IMAGE_DATA_DIRECTORY, Size),
			&ExportTableSize,
			sizeof(DWORD),
			ProcHandle);
		ExportTable = (PIMAGE_EXPORT_DIRECTORY)((PCHAR)ModuleBase + ExportTableVirtualAddress);

		DWORD ExportTableMaxVirtualAddress = ExportTableVirtualAddress + ExportTableSize;

		if (ExportTableSize < sizeof(IMAGE_EXPORT_DIRECTORY))
		{
			goto Leave;
		}

		ProbeForRead(
			(LPVOID)ExportTable,
			ExportTableSize,
			__alignof(BYTE));


		// Read the number of functions
		DWORD NumberOfFunctions;
		ReadMemory((PCHAR)ExportTable + FIELD_OFFSET(IMAGE_EXPORT_DIRECTORY, NumberOfFunctions), &NumberOfFunctions, sizeof(DWORD), ProcHandle);

		// Read the number of function names
		DWORD NumberOfNames;
		ReadMemory((PCHAR)ExportTable + FIELD_OFFSET(IMAGE_EXPORT_DIRECTORY, NumberOfNames), &NumberOfNames, sizeof(DWORD), ProcHandle);

		// Read the necessary RVAs
		DWORD AddressOfFunctions;
		DWORD AddressOfNames;
		DWORD AddressOfNameOrdinals;
		ReadMemory((PCHAR)ExportTable + FIELD_OFFSET(IMAGE_EXPORT_DIRECTORY, AddressOfFunctions), &AddressOfFunctions, sizeof(DWORD), ProcHandle);

		ReadMemory((PCHAR)ExportTable + FIELD_OFFSET(IMAGE_EXPORT_DIRECTORY, AddressOfNames), &AddressOfNames, sizeof(DWORD), ProcHandle);

		ReadMemory((PCHAR)ExportTable + FIELD_OFFSET(IMAGE_EXPORT_DIRECTORY, AddressOfNameOrdinals), &AddressOfNameOrdinals, sizeof(DWORD), ProcHandle);

		// Check if addresses point into the export data directory.
		// Also if all address in range we won't need to probe for read again as it was already done on this range.
		if (
			(AddressOfFunctions < ExportTableVirtualAddress) ||
			(AddressOfFunctions >= (ExportTableMaxVirtualAddress)) ||
			(AddressOfNames < ExportTableVirtualAddress) ||
			(AddressOfNames >= (ExportTableMaxVirtualAddress)) ||
			(AddressOfNameOrdinals < ExportTableVirtualAddress) ||
			(AddressOfNameOrdinals >= (ExportTableMaxVirtualAddress)) ||
			((AddressOfFunctions + sizeof(DWORD) * NumberOfFunctions) >= (ExportTableMaxVirtualAddress)) ||
			((AddressOfNames + sizeof(DWORD) * NumberOfNames) >= (ExportTableMaxVirtualAddress)) ||
			((AddressOfNameOrdinals + sizeof(WORD) * NumberOfNames) >= (ExportTableMaxVirtualAddress)))
		{
			goto Leave;
		}

		// Get the actual addresses (not the RVAs) of exported functions
		ArrayOfFunctionAddresses = (PULONG)((PUCHAR)ModuleBase + AddressOfFunctions);
		ArrayOfFunctionNames = (PULONG)((PUCHAR)ModuleBase + AddressOfNames);
		ArrayOfFunctionNameOrdinals = (PUSHORT)((PUCHAR)ModuleBase + AddressOfNameOrdinals);

		// loop through named exported function
		for (ULONG i = 0; i < NumberOfNames; i++)
		{
			// get the function name and the function ordinal
			ReadMemory((PCHAR)ArrayOfFunctionNames + (i * sizeof(ULONG)), &FunctionNameIndex, sizeof(ULONG), ProcHandle);
			FunctionName = (PCHAR)((PUCHAR)ModuleBase + FunctionNameIndex);

			// When searching for an exported function, we have to go through the AddressOfNames and matching the function
			// that we need; then take the number at the same index from the AddressOfNameOrdinals array. Therefore the
			// 'Base' (pExportTable->Base) is only used when searching for a function via an ordinal (not the name).
			ReadMemory((PCHAR)ArrayOfFunctionNameOrdinals + (i * sizeof(USHORT)), &FunctionOrdinal, sizeof(USHORT), ProcHandle);

			// Get the function address, where the function pointer is not located at the corresponding index [i], but
			// rather at the index [FunctionOrdinal], which is part of the specification of the PE file format.
			if (FunctionOrdinal > NumberOfFunctions)
			{
				// FunctionOrdinal points to an out of bound location in the FunctionTable.
				continue;
			}
			ReadMemory((PCHAR)ArrayOfFunctionAddresses + (FunctionOrdinal * sizeof(ULONG)), &FunctionAddressRVA, sizeof(ULONG), ProcHandle);

			if (FunctionAddressRVA >= ExportTableVirtualAddress &&
				FunctionAddressRVA < ExportTableMaxVirtualAddress)
			{
				// Exported function is a forward reference to another function in a different DLL, no need to process it
				continue;
			}

			LONG FunctionNameLength;
			FunctionNameLength = UserKernelUtilsLib::IUtilsInt::GetInstance()->StrLenSafe(FunctionName, MAX_PATH, ProcHandle);
			if (FunctionNameLength <= 0)
			{
				continue;
			}
			FunctionNameLength++;

			// Read the function name from remote process, including the terminating NULL.
			CHAR Function[MAX_PATH];
			ReadMemory(FunctionName, Function, FunctionNameLength, ProcHandle);

			FunctionAddress = (PULONG_PTR)((PUCHAR)ModuleBase + FunctionAddressRVA);
			if (strcmp(Function, pFunctionName) == 0)
			{
				pFunctionAddress = FunctionAddress;
				break;
			}
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		pFunctionAddress = NULL;
	}

Leave:
	return pFunctionAddress;
}
