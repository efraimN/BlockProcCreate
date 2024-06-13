#pragma once
namespace UserKernelUtilsLib
{
	class IFileUtilsInt
	{
	public:
		static IFileUtilsInt* GetInstance();

		/* In kernel The handle returned is a kernel Handle */
		virtual
		BOOLEAN OpenFile(
			PUNICODE_STRING FileName,
			PHANDLE hFile,
			ACCESS_MASK DesiredAccess,
			ULONG ShareAccess
		) = 0;

		/* In kernel The handle returned is a kernel Handle */
		virtual
		BOOLEAN CreateFile(
			PUNICODE_STRING FileName,
			PHANDLE hFile,
			ACCESS_MASK DesiredAccess,
			ULONG ShareAccess,
			LARGE_INTEGER AllocationSize
		) = 0;

		/* This function is like the usual user-mode function */
		/* In kernel The handle returned is a kernel Handle */
		virtual
		NTSTATUS CreateFile_Ex(
			PUNICODE_STRING FileName,
			ACCESS_MASK DesiredAccess,
			ULONG ShareMode,
			LPSECURITY_ATTRIBUTES SecurityAttributes,
			DWORD CreationDisposition,
			DWORD  FlagsAndAttributes,
			PHANDLE hFile
		) = 0;

		/* This function is like the usual user-mode function */
		virtual
			NTSTATUS GetOverlappedResult_Ex(
			HANDLE hFile,
			LPOVERLAPPED Overlapped,
			LPDWORD NumberOfBytesTransferred,
			BOOL Wait
		) = 0;

		/* This function is like the usual user-mode function */
		virtual
			NTSTATUS WriteFile_Ex(
			HANDLE hFile,
			LPCVOID lpBuffer,
			DWORD nNumberOfBytesToWrite,
			LPDWORD lpNumberOfBytesWritten,
			LPOVERLAPPED lpOverlapped
		) = 0;

		virtual
		BOOLEAN GetFileSize(
			HANDLE hFile,
			PLARGE_INTEGER FileSize
		) = 0;

		virtual
		BOOLEAN SetFileSize(
			HANDLE hFile,
			LARGE_INTEGER FileSize
		) = 0;

	protected:
		IFileUtilsInt() NOEXCEPT {};
		virtual ~IFileUtilsInt() {};

	private:
		IFileUtilsInt(const IFileUtilsInt& other);
		IFileUtilsInt& operator=(const IFileUtilsInt& other);
	};
}
