namespace UserKernelUtilsLib
{
	class IRegUtilsInt
	{
	public:
		static IRegUtilsInt* GetInstance();

		virtual NTSTATUS RegDeleteAllKeys(HANDLE ParentKey) = 0;

		virtual
		NTSTATUS
		RegSetValue(
			IN     ULONG           RelativeTo,
			IN     PWSTR           KeyNamePath,
			IN     PWSTR           ParameterName,
			IN OUT PVOID           ParameterValue,
			IN OUT ULONG           ParameterLength,
			IN     ULONG		   ValueType
		) = 0;

		virtual
		NTSTATUS
		RegGetValue(
			IN     ULONG           RelativeTo,
			IN     PWSTR           KeyNamePath,
			IN     PWSTR           ParameterName,
			IN OUT PVOID           ParameterValue,
			IN OUT PDWORD          ParameterLength
		) = 0;

		virtual
		BOOL
		RegDeletekey(PWCHAR Path) = 0;

		virtual
		HANDLE
		RegOpenCreateKey(
			_In_  ULONG RelativeTo,
			_In_  PWSTR Path,
			_In_  PWSTR SubPath,
			BOOLEAN Open_Create,
			ACCESS_MASK DesiredAccess,
			ULONG CreateOptions
		) = 0;

	protected:
		IRegUtilsInt()NOEXCEPT {};
		virtual ~IRegUtilsInt() {};

	private:
		IRegUtilsInt(const IRegUtilsInt& other);
		IRegUtilsInt& operator=(const IRegUtilsInt& other);
	};
}
