namespace UserKernelUtilsLib
{
	class CUtilsInt
	{
	public:

		static CUtilsInt* GetInstance();
	
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
		CUtilsInt() NOEXCEPT {};
		virtual ~CUtilsInt() {};

	private:
		CUtilsInt(const CUtilsInt& other);
		CUtilsInt& operator=(const CUtilsInt& other);
	};

}
