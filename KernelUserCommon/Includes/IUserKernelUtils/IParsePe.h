#pragma once

namespace UserKernelUtilsLib
{
	typedef enum {
		DLL_X86 = 1,
		DLL_X64,
		DLL_INVALID
	} DllArchitecture;

	class ParsePeCallBack
	{
	public:
		ParsePeCallBack() {
			m_ProcHandle = NULL;
			m_DoEnumSectionsCallBack = FALSE;
			m_DoEnumImageDirectoryCallBack= FALSE;
		};
		virtual~ParsePeCallBack() {};

		virtual
		INT GetModuleNameAndTypeCallback(
			PCHAR DllName,
			DllArchitecture ImageArchitecture
		) = 0;

		/* This callback is called once for each section */
		virtual
		INT EnumSectionsCallBack(
			PIMAGE_SECTION_HEADER pSectionHeader
		) = 0;

		/* This callback is called once for each directory entry (expordir, importdir, etc) */
		virtual
		INT EnumImageDirectoryCallback(
			DWORD DataDirectoryType,
			UINT_PTR Base,
			PIMAGE_DATA_DIRECTORY ImageDataDirectory
		) = 0;

		HANDLE m_ProcHandle;
		BOOLEAN m_DoEnumSectionsCallBack;
		BOOLEAN m_DoEnumImageDirectoryCallBack;

	protected:
	private:
		// delete copy and move constructors and assign operators
		ParsePeCallBack(ParsePeCallBack const&);             // Copy construct
		ParsePeCallBack(ParsePeCallBack&&);                  // Move construct
		ParsePeCallBack& operator=(ParsePeCallBack const&);  // Copy assign
		ParsePeCallBack& operator=(ParsePeCallBack&&);       // Move assign
	};


	class IParsePe
	{
	public:
		
		typedef PCHAR(*pfnMemAllocator)(size_t size);

		static IParsePe* GetInstance();

		/*
		Returns:
		-1  if something wrong with the parsing
		0   if everything is ok
		>=1 If any of the callbacks has stopped the enumeration

		*/
		virtual
		INT ParsePeMain(
			_In_ HANDLE ProcHandle,
			_In_ PVOID ModuleBase,
			_In_ ParsePeCallBack* CallFunction,
			_In_ BOOLEAN VerBose,
			_In_ BOOLEAN IgnoreMagics
		) = 0;

		/* returns an array of Imported Dlls And Functions (if they are)  */
		/* first is a dll, then ',' separated all the functions then a ';' and a new dll name and functions ',' separated (or nul to finish)*/
		virtual
		PCHAR EnumImportedFunctions(
			HANDLE ProcHandle,
			UINT_PTR Base,
			DllArchitecture ImageArchitecture,
			pfnMemAllocator Allocator,
			DWORD Size,
			DWORD Offset
		) = 0;

		/* returns an array of exported functions (if they are) separated by a ',' */
		virtual
		PCHAR EnumExportedFunctions(
			HANDLE ProcHandle,
			UINT_PTR Base,
			pfnMemAllocator Allocator,
			DWORD Size,
			DWORD Offset
		) = 0;

		virtual
		BOOLEAN GetPeDirectorySizeOffset(
			HANDLE ProcHandle,
			PCHAR ImageDataDirectory,
			DWORD Index,
			PDWORD Size,
			PDWORD Ofset
		) = 0;

		virtual
		PVOID
		InMemoryGetProcAddress(
			PVOID ModuleBase,
			LPCCH pFunctionName,
			HANDLE ProcHandle
		) = 0;

	protected:

		IParsePe() {};
		virtual~IParsePe() {};

	private:
		// delete copy and move constructors and assign operators
		IParsePe(IParsePe const&);             // Copy construct
		IParsePe(IParsePe&&);                  // Move construct
		IParsePe& operator=(IParsePe const&);  // Copy assign
		IParsePe& operator=(IParsePe&&);       // Move assign

	};
}
