#pragma once
namespace UserKernelUtilsLib
{
	class IUtilsInt
	{
	public:
		static IUtilsInt* GetInstance();

	virtual BOOL IsProcessWow64(
		_In_ HANDLE ProcessHandle,
		_Out_ PBOOL IsWow
	) = 0;

	virtual
	LONG
	SafeSearchString(
		IN PUNICODE_STRING Source,
		IN PUNICODE_STRING Target,
		IN BOOLEAN CaseInSensitive
	) = 0;

	virtual
	LONG
	StrLenSafe(
		_Out_ const char* str,
		_In_ ULONG MaxLen,
		_In_ HANDLE ProcHandle
	) = 0;

	protected:
		IUtilsInt() NOEXCEPT {};
		virtual~IUtilsInt() {};

	private:
		IUtilsInt(const IUtilsInt& other);
		IUtilsInt& operator=(const IUtilsInt& other);

	};
}
