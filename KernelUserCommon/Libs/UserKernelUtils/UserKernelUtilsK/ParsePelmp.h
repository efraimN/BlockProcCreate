#pragma once

#include <IParsePe.h>

namespace UserKernelUtilsLib
{
	class ParsePeImp : public IParsePe
	{
	public:
		friend IParsePe;

		virtual
		INT ParsePeMain(
			_In_ HANDLE ProcHandle,
			_In_ PVOID ModuleBase,
			_In_ ParsePeCallBack* CallFunction,
			_In_ BOOLEAN VerBose,
			_In_ BOOLEAN IgnoreMagics
		);

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
		);

		/* returns an array of exported functions (if they are) separated by a ',' */
		virtual
		PCHAR EnumExportedFunctions(
			HANDLE ProcHandle,
			UINT_PTR Base,
			pfnMemAllocator Allocator,
			DWORD Size,
			DWORD Offset
		);

		virtual
		BOOLEAN GetPeDirectorySizeOffset(
			HANDLE ProcHandle,
			PCHAR ImageDataDirectory,
			DWORD Index,
			PDWORD Size,
			PDWORD Ofset
		);

		virtual
		PVOID
		InMemoryGetProcAddress(
			PVOID ModuleBase,
			LPCCH pFunctionName,
			HANDLE ProcHandle
		);

	protected:

	private:
		// delete copy and move constructors and assign operators
		ParsePeImp(ParsePeImp const&);             // Copy construct
		ParsePeImp(ParsePeImp&&);                  // Move construct
		ParsePeImp& operator=(ParsePeImp const&);  // Copy assign
		ParsePeImp& operator=(ParsePeImp&&);       // Move assign


		ParsePeImp() {};
		virtual~ParsePeImp() {};

	};
}
