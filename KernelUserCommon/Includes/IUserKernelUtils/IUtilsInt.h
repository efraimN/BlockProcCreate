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

	protected:
		IUtilsInt() NOEXCEPT {};
		virtual~IUtilsInt() {};

	private:
		IUtilsInt(const IUtilsInt& other);
		IUtilsInt& operator=(const IUtilsInt& other);

	};
}
