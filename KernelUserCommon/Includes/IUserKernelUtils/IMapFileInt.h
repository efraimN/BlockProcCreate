#pragma once
namespace UserKernelUtilsLib
{
	class IMapFileInt
	{
	public:
		static IMapFileInt* GetInstance();

		/* In kernel The handle returned is a kernel Handle */
		virtual
		NTSTATUS  CreateSection(
			OUT PHANDLE SectionHandle,
			IN HANDLE FileHandle,
			IN PVOID SecurityDescriptor,
			BOOLEAN Inheritable,
			IN DWORD flProtect,
			IN DWORD MaximumSizeHigh,
			IN DWORD MaximumSizeLow,
			IN PCWSTR SectionName
		) = 0;

		/*
		 if NumberOfBytesToMap is not NULL, will return the amount mapped
		 if NumberOfBytesToMap is NULL, or *NumberOfBytesToMap is zero will map all the section/file
		 if *NumberOfBytesToMap is different of zero will only map *NumberOfBytesToMap bytes
		*/
		virtual
		NTSTATUS MapFile(
			_Out_ LPVOID* ViewBase,
			_In_ HANDLE SectionHandle,
			_In_ HANDLE ProcessHandle,
			_In_ DWORD DesiredAccess,
			_In_ DWORD FileOffsetHigh,
			_In_ DWORD FileOffsetLow,
			_Inout_opt_ SIZE_T* NumberOfBytesToMap
		) = 0;

		virtual
		BOOLEAN UnMapFile(
			_In_ PVOID BaseAddress,
			_In_ HANDLE ProcessHandle
		) = 0;

		virtual
		BOOLEAN
		MapFileIntoMem(
			_In_ PUNICODE_STRING FullFilePath,
			_Out_ PVOID* DataMemPtr,
			_Out_ ULONG* DataLength,
			_In_ BOOL ReadOnly
		) = 0;

		virtual
		BOOLEAN
		MapFileIntoMem(
			_In_ HANDLE FileHandle,
			_Out_ PVOID* DataMemPtr,
			_Out_ ULONG* DataLength,
			_In_ BOOL ReadOnly
		) = 0;

		virtual
		BOOLEAN
		MapFileIntoMemEx(
			_In_ HANDLE FileHandle,
			_Inout_ PVOID* DataMemPtr,
			_Inout_ ULONG* DataLength,
			_In_ BOOL ReadOnly,
			_In_ BOOL Image,
			_Inout_ PVOID* SectionHandle
		) = 0;

		virtual
		BOOLEAN
		MapFileIntoMemEx(
			_In_ PUNICODE_STRING FullFilePath,
			_Inout_ PVOID* DataMemPtr,
			_Inout_ ULONG* DataLength,
			_In_ BOOL ReadOnly,
			_In_ BOOL Image,
			_Inout_ PVOID* SectionHandle
		) = 0;
protected:
		IMapFileInt() NOEXCEPT {};
		virtual~IMapFileInt() {};

	private:
		IMapFileInt(const IMapFileInt& other);
		IMapFileInt& operator=(const IMapFileInt& other);

	};
}


